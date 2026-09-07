"""
Tests closing the ranked coverage gaps in
dev_log/audit_20260704/tests_audit.md "Top gaps" for validation/error paths
that were not already covered by test_audit_fixes_core.py.
"""

from pathlib import Path

import pybtls as pb
import pybtls._resource as _resource
import pytest


def _nominal_vehicle():
    vehicle = pb.Vehicle(no_axles=1)
    vehicle.set_axle_weights([100.0])
    vehicle.set_axle_spacings([0.0])
    vehicle.set_axle_widths([2.0])
    return vehicle


def _lane_fixture(lane_index, lane_dir=1, classifier_type=None):
    """Build a minimal (lfc, vehicle_gen, headway_gen) triple for a lane."""

    lfc = pb.LaneFlowComposition(lane_index=lane_index, lane_dir=lane_dir)
    lfc.assign_lane_data(
        hourly_truck_flow=[80] * 24,
        hourly_car_flow=[20] * 24,
        hourly_speed_mean=[100.0] * 24,
        hourly_speed_std=[5.0] * 24,
        hourly_truck_composition=[[25.0, 25.0, 25.0, 25.0] for _ in range(24)],
    )

    kwargs = {"classifier_type": classifier_type} if classifier_type else {}
    vehicle_gen = pb.VehicleGenNominal(
        nominal_vehicle=_nominal_vehicle(), COV_list=[0.1, 0.1], **kwargs
    )
    headway_gen = pb.HeadwayGenConstant(constant_speed=36.0, constant_gap=5.0)

    return lfc, vehicle_gen, headway_gen


# --- 1. vehicle_generator.py raise sites (kernel checks already covered in
# test_audit_fixes_core.py, skipped here) ------------------------------------


def test_vehicle_gen_nominal_rejects_invalid_cov_list():
    with pytest.raises(ValueError, match="Invalid COV list"):
        pb.VehicleGenNominal(nominal_vehicle=_nominal_vehicle(), COV_list=[0.1])


def test_vehicle_gen_garage_requires_garage_format_for_path():
    with pytest.raises(ValueError, match="Garage format is not specified"):
        pb.VehicleGenGarage(
            garage=Path(__file__).parent / "test_data/garage.txt",
            kernel=[[1.0, 0.1], [1.0, 0.1], [1.0, 0.1]],
        )


def test_vehicle_gen_garage_rejects_non_vehicle_objects():
    with pytest.raises(ValueError, match="non-Vehicle"):
        pb.VehicleGenGarage(
            garage=[1, 2, 3], kernel=[[1.0, 0.1], [1.0, 0.1], [1.0, 0.1]]
        )


# --- 2. lfc.py: mismatched hourly list lengths / wrong types (7 sites) ------


def test_lfc_rejects_invalid_lane_dir():
    with pytest.raises(ValueError, match="lane_dir"):
        pb.LaneFlowComposition(lane_index=1, lane_dir=3)


def test_lfc_rejects_mismatched_hourly_flow1_length():
    lfc = pb.LaneFlowComposition(lane_index=1, lane_dir=1)
    with pytest.raises(ValueError, match="does not match"):
        lfc.assign_lane_data(hourly_truck_flow=[1] * 23, hourly_car_flow=[1] * 24)


def test_lfc_rejects_mismatched_hourly_flow2_length():
    lfc = pb.LaneFlowComposition(lane_index=1, lane_dir=1)
    with pytest.raises(ValueError, match="does not match"):
        lfc.assign_lane_data(
            hourly_truck_flow=[1] * 23, hourly_car_percentage=[1.0] * 24
        )


def test_lfc_rejects_mismatched_hourly_speed_length():
    lfc = pb.LaneFlowComposition(lane_index=1, lane_dir=1)
    with pytest.raises(ValueError, match="does not match"):
        lfc.assign_lane_data(hourly_speed_mean=[1] * 23, hourly_speed_std=[1] * 24)


def test_lfc_rejects_non_list_truck_composition():
    lfc = pb.LaneFlowComposition(lane_index=1, lane_dir=1)
    with pytest.raises(TypeError, match="list of lists"):
        lfc.assign_lane_data(hourly_truck_composition=[1, 2, 3])


def test_lfc_rejects_mismatched_truck_composition_length():
    lfc = pb.LaneFlowComposition(lane_index=1, lane_dir=1)
    with pytest.raises(ValueError, match="does not match"):
        lfc.assign_lane_data(hourly_truck_composition=[[25.0] * 4])


def test_lfc_rejects_truck_composition_sublist_not_four():
    lfc = pb.LaneFlowComposition(lane_index=1, lane_dir=1)
    with pytest.raises(ValueError, match="four truck percentages"):
        lfc.assign_lane_data(hourly_truck_composition=[[25.0, 25.0, 25.0]] * 24)


# --- 3. traffic_loader.py error paths ---------------------------------------


def test_traffic_loader_rejects_empty_file(tmp_path):
    empty = tmp_path / "empty.txt"
    empty.write_text("")

    # An empty file also has 0 lanes, so no_lane must be 0 too in order to
    # reach the "no traffic" check instead of the lane-count RuntimeError.
    loader = pb.TrafficLoader(no_lane=0)
    with pytest.raises(ValueError, match="No traffic"):
        loader.add_traffic(
            traffic=empty,
            traffic_format=4,
            use_const_speed=True,
            const_speed_value=40.0,
        )


def test_traffic_loader_rejects_mismatched_lane_count():
    # test_traffic_file.txt declares 4 lanes (see test_sim_run.py); declaring
    # a different no_lane must raise.
    loader = pb.TrafficLoader(no_lane=1)
    with pytest.raises(RuntimeError, match="Number of lanes"):
        loader.add_traffic(
            traffic=Path(__file__).parent / "test_data/test_traffic_file.txt",
            traffic_format=4,
            use_const_speed=True,
            const_speed_value=40.0,
        )


# --- 4. headway_generator.py raise sites -------------------------------------


def test_headway_gen_heds_requires_speed_assigned():
    lfc = pb.LaneFlowComposition(lane_index=1, lane_dir=1)
    headway_gen = pb.HeadwayGenHeDS()
    with pytest.raises(ValueError, match="Speed data"):
        headway_gen._check_lfc(lfc)


def test_headway_gen_freeflow_requires_speed_assigned():
    lfc = pb.LaneFlowComposition(lane_index=1, lane_dir=1)
    headway_gen = pb.HeadwayGenFreeflow()
    with pytest.raises(ValueError, match="Speed data"):
        headway_gen._check_lfc(lfc)


# --- 5. traffic_generator.py remaining raise sites ---------------------------


def test_traffic_generator_set_start_time_rejects_wrong_type():
    tg = pb.TrafficGenerator(no_lane=2)
    with pytest.raises(ValueError, match="start_time"):
        tg.set_start_time("bad")


def test_traffic_generator_set_start_time_rejects_wrong_length():
    tg = pb.TrafficGenerator(no_lane=2)
    with pytest.raises(ValueError, match="start_time"):
        tg.set_start_time([1.0, 2.0, 3.0])


def test_traffic_generator_rejects_mixed_vehicle_classifiers():
    tg = pb.TrafficGenerator(no_lane=2)
    lfc1, vg1, hg1 = _lane_fixture(lane_index=1, classifier_type="axle")
    lfc2, vg2, hg2 = _lane_fixture(lane_index=2, classifier_type=None)

    tg.add_lane(vehicle_gen=vg1, headway_gen=hg1, lfc=lfc1)
    tg.add_lane(vehicle_gen=vg2, headway_gen=hg2, lfc=lfc2)

    with pytest.raises(RuntimeError, match="classifier"):
        tg._get_traffic_generator(20.0)


# --- 6. garage format-5 (SIWIM) write is not supported by the C++ layer -----
#
# Sanity check first (see tests/test_SIWIM_format.py for the read-side
# fixture): a genuine write-then-read round trip is impossible because the
# C++ traffic-file writer explicitly rejects the SIWIM format. This is
# stronger than the audit's original sketch (which assumed the round trip
# would succeed) -- confirmed empirically against the current source.


def test_garage_write_siwim_format_is_rejected(tmp_path):
    vehicles = pb.garage.read_garage_file(
        Path(__file__).parent / "test_data/siwim.csv", garage_format=5
    )
    out = tmp_path / "rewritten_siwim.csv"
    with pytest.raises(ValueError, match="does not support writing"):
        pb.garage.write_garage_file(vehicles, out, 5)


# --- 7. utils/vehicle_DF.df_to_vehicle_list round trip -----------------------


def test_vehicle_df_round_trip_preserves_properties():
    v1 = pb.Vehicle(no_axles=2)
    v1.set_axle_weights([80.0, 70.0])
    v1.set_axle_spacings([4.0, 0.0])
    v1.set_axle_widths([2.0, 2.0])
    v1.set_velocity(20.0)
    v1.set_direction(1)
    v1.set_local_lane(1)
    v1.set_time(100.0)
    v1.set_trans(0.5)
    v1.set_acceleration(-0.2)

    v2 = pb.Vehicle(no_axles=3)
    v2.set_axle_weights([50.0, 60.0, 55.0])
    v2.set_axle_spacings([3.0, 3.5, 0.0])
    v2.set_axle_widths([2.0, 2.0, 2.0])
    v2.set_velocity(25.0)
    v2.set_direction(2)
    v2.set_local_lane(2)
    v2.set_time(200.0)
    v2.set_trans(-0.3)
    v2.set_acceleration(0.1)

    df = pb.utils.vehicle_list_to_df([v1, v2])
    round_tripped = pb.utils.df_to_vehicle_list(df)

    assert len(round_tripped) == 2
    for original, back in zip([v1, v2], round_tripped):
        assert back.get_no_axles() == original.get_no_axles()
        assert back.get_axle_weights() == pytest.approx(original.get_axle_weights())
        assert back.get_axle_spacings() == pytest.approx(original.get_axle_spacings())
        assert back.get_velocity() == pytest.approx(original.get_velocity())
        assert back.get_direction() == original.get_direction()
        assert back.get_local_lane() == original.get_local_lane()
        assert back.get_time() == pytest.approx(original.get_time())
        assert back.get_trans() == pytest.approx(original.get_trans())
        assert back.get_acceleration() == pytest.approx(original.get_acceleration())


# --- 8. _resource.warn_if_file_too_large ------------------------------------
#
# Note: the current implementation prints a warning to stderr (not
# warnings.warn), so this is verified via capsys rather than pytest.warns
# -- confirmed empirically against the current source.


def test_warn_if_file_too_large_prints_warning_when_memory_scarce(
    tmp_path, monkeypatch, capsys
):
    big_file = tmp_path / "big.txt"
    big_file.write_text("x" * 1000)

    monkeypatch.setattr(_resource, "available_host_memory", lambda: 100)
    _resource.warn_if_file_too_large(big_file, "traffic file")

    captured = capsys.readouterr()
    assert "WARNING" in captured.err


def test_warn_if_file_too_large_silent_when_memory_plentiful(
    tmp_path, monkeypatch, capsys
):
    small_file = tmp_path / "small.txt"
    small_file.write_text("x" * 1000)

    monkeypatch.setattr(_resource, "available_host_memory", lambda: 10**15)
    _resource.warn_if_file_too_large(small_file, "traffic file")

    captured = capsys.readouterr()
    assert captured.err == ""


def _one_vehicle():
    v = pb.Vehicle(no_axles=3)
    v.set_axle_weights([80.0, 90.0, 100.0])
    v.set_axle_spacings([4.0, 3.0, 0.0])
    v.set_axle_widths([2.0, 2.0, 2.0])
    v.set_velocity(20.0)
    v.set_direction(1)
    v.set_local_lane(1)
    v.set_time(7 * 86400.0)
    return v


def test_df_to_vehicle_list_matches_columns_by_name_not_position():
    """_set_all_properties is positional, so a frame whose columns are in a
    different order must still be read into the right properties."""
    df = pb.utils.vehicle_list_to_df([_one_vehicle()])
    shuffled = df[sorted(df.columns)]
    assert list(shuffled.columns) != list(df.columns)

    back = pb.utils.df_to_vehicle_list(shuffled)[0]
    assert back.get_no_axles() == 3
    assert back.get_gvw() == pytest.approx(270.0)
    assert back.get_velocity() == pytest.approx(20.0)


def test_df_to_vehicle_list_ignores_extra_columns():
    df = pb.utils.vehicle_list_to_df([_one_vehicle()])
    df["SomeUserAnnotation"] = "keep me out of the tuple"

    back = pb.utils.df_to_vehicle_list(df)[0]
    assert back.get_no_axles() == 3
    assert back.get_gvw() == pytest.approx(270.0)


def test_df_to_vehicle_list_does_not_mutate_the_callers_frame():
    """The GVW / Length refresh must not write back into the input frame."""
    df = pb.utils.vehicle_list_to_df([_one_vehicle()])
    df["GVW"] = -1.0
    df["Length"] = -1.0

    pb.utils.df_to_vehicle_list(df)

    assert df["GVW"].iloc[0] == -1.0
    assert df["Length"].iloc[0] == -1.0


def test_df_to_vehicle_list_names_the_missing_columns():
    df = pb.utils.vehicle_list_to_df([_one_vehicle()]).drop(columns=["Dir", "Trns"])
    with pytest.raises(ValueError, match="Dir, Trns"):
        pb.utils.df_to_vehicle_list(df)
