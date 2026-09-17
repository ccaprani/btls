"""
Argument validation and error paths of the Python API: every entry point
rejects what it cannot use, fails before mutating state, and names what is
wrong.
"""

from pathlib import Path

import pybtls as pb
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


# --- 1. vehicle_generator.py raise sites (kernel checks are in
# test_traffic_generator.py) -------------------------------------------------


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

    v2 = pb.Vehicle(no_axles=3)
    v2.set_axle_weights([50.0, 60.0, 55.0])
    v2.set_axle_spacings([3.0, 3.5, 0.0])
    v2.set_axle_widths([2.0, 2.0, 2.0])
    v2.set_velocity(25.0)
    v2.set_direction(2)
    v2.set_local_lane(2)
    v2.set_time(200.0)
    v2.set_trans(-0.3)

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


def test_bridge_add_load_effect_validates_before_mutating_state():
    bridge = pb.Bridge(length=10.0, no_lane=2)

    inf_line = pb.InfluenceLine(IL_type="built-in")
    inf_line.set_IL(id=1, length=10.0)

    with pytest.raises(ValueError):
        bridge.add_load_effect(inf_line, inf_weight=[1.0])  # wrong length

    # State must be unchanged after the failed call.
    assert bridge._no_load_effect == 0
    assert len(bridge._threshold_list) == 0

    # Correct retry must succeed and be usable.
    bridge.add_load_effect(inf_line, inf_weight=[1.0, 1.0])
    assert bridge._no_load_effect == 1

    output_config = pb.OutputConfig()
    c_bridge = bridge._get_bridge(output_config)  # must not raise KeyError
    assert c_bridge is not None


def test_vehicle_list_to_df_day_year_columns():
    vehicle = pb.Vehicle(no_axles=1)
    vehicle.set_axle_weights([100.0])
    vehicle.set_axle_spacings([0.0])
    vehicle.set_axle_widths([2.0])

    props = list(vehicle._get_all_properties())
    props[1] = 15  # Day
    props[2] = 6  # Month
    props[3] = 2020  # Year
    vehicle._set_all_properties(tuple(props))

    df = pb.utils.vehicle_list_to_df([vehicle])
    assert df["Day"].iloc[0] == 15
    assert df["Month"].iloc[0] == 6
    assert df["Year"].iloc[0] == 2020

    # Round trip must preserve the (now correctly labelled) values.
    round_tripped = pb.utils.df_to_vehicle_list(df)[0]
    rt_props = round_tripped._get_all_properties()
    assert rt_props[1] == 15
    assert rt_props[2] == 6
    assert rt_props[3] == 2020


# --- every **kwargs entry point rejects unknown names ------------------------

TRAFFIC_FILE = Path(__file__).parent / "test_data/test_traffic_file.txt"
GARAGE_FILE = Path(__file__).parent / "test_data/garage.txt"


def _bogus_calls(tmp_path):
    bogus = {"bogus_option": 1}
    truck = _nominal_vehicle()
    return {
        "add_traffic": lambda: pb.TrafficLoader(no_lane=4).add_traffic(
            traffic=TRAFFIC_FILE, traffic_format=4, **bogus
        ),
        "VehicleGenNominal": lambda: pb.VehicleGenNominal(
            nominal_vehicle=truck, COV_list=[0.05, 0.05], **bogus
        ),
        "VehicleGenGrave": lambda: pb.VehicleGenGrave(traffic_site="Auxerre", **bogus),
        "VehicleGenGarage": lambda: pb.VehicleGenGarage(
            garage=[truck], kernel=[[1.0, 0.05]] * 3, **bogus
        ),
        "HeadwayGenHeDS": lambda: pb.HeadwayGenHeDS(**bogus),
        "HeadwayGenConstant": lambda: pb.HeadwayGenConstant(
            constant_speed=36.0, constant_gap=5.0, **bogus
        ),
        "HeadwayGenCongested": lambda: pb.HeadwayGenCongested(
            congested_spacing=20.0, congested_speed=36.0, **bogus
        ),
        "HeadwayGenFreeflow": lambda: pb.HeadwayGenFreeflow(**bogus),
        "assign_lane_data": lambda: pb.LaneFlowComposition(
            lane_index=1, lane_dir=1
        ).assign_lane_data(
            hourly_truck_flow=[100] * 24, hourly_car_flow=[0] * 24, **bogus
        ),
        "set_IL": lambda: pb.InfluenceLine("built-in").set_IL(
            id=1, length=20.0, **bogus
        ),
        "read_garage_file": lambda: pb.garage.read_garage_file(GARAGE_FILE, 4, **bogus),
        "write_garage_file": lambda: pb.garage.write_garage_file(
            [truck], tmp_path / "garage.txt", 4, **bogus
        ),
    }


@pytest.mark.parametrize("entry_point", sorted(_bogus_calls(Path(".")).keys()))
def test_unknown_kwargs_are_rejected(entry_point, tmp_path):
    with pytest.raises(TypeError, match="bogus_option"):
        _bogus_calls(tmp_path)[entry_point]()
