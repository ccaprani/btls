"""
Regression tests for the core-module bugs found in
dev_log/audit_20260704/bugs_core.md.
"""

import pybtls as pb
import pytest


def _lane_fixture(lane_index, lane_dir=1):
    """Build a minimal (lfc, vehicle_gen, headway_gen) triple for a lane."""

    vehicle = pb.Vehicle(no_axles=1)
    vehicle.set_axle_weights([100.0])
    vehicle.set_axle_spacings([0.0])
    vehicle.set_axle_widths([2.0])

    lfc = pb.LaneFlowComposition(lane_index=lane_index, lane_dir=lane_dir)
    lfc.assign_lane_data(
        hourly_truck_flow=[80] * 24,
        hourly_car_flow=[20] * 24,
        hourly_speed_mean=[100.0] * 24,
        hourly_speed_std=[5.0] * 24,
        hourly_truck_composition=[[25.0, 25.0, 25.0, 25.0] for _ in range(24)],
    )

    vehicle_gen = pb.VehicleGenNominal(nominal_vehicle=vehicle, COV_list=[0.1, 0.1])
    headway_gen = pb.HeadwayGenConstant(constant_speed=36.0, constant_gap=5.0)

    return lfc, vehicle_gen, headway_gen


# --- FIX 1: TrafficGenerator lane order -------------------------------------


def test_traffic_generator_lane_order_independent_of_add_order():
    tg = pb.TrafficGenerator(no_lane=2)

    lfc2, vg2, hg2 = _lane_fixture(lane_index=2)
    lfc1, vg1, hg1 = _lane_fixture(lane_index=1)

    # lane 2 added first, lane 1 added second
    tg.add_lane(vehicle_gen=vg2, headway_gen=hg2, lfc=lfc2)
    tg.add_lane(vehicle_gen=vg1, headway_gen=hg1, lfc=lfc1)

    assert tg._lanes[0].index == 1
    assert tg._lanes[1].index == 2

    # set_start_time (list form) must map by global lane_index, not call order
    tg.set_start_time([10.0, 20.0])
    assert tg._lanes[0].start_time == 10.0
    assert tg._lanes[1].start_time == 20.0


def test_traffic_generator_duplicate_lane_index_raises():
    tg = pb.TrafficGenerator(no_lane=2)

    lfc1a, vg1a, hg1a = _lane_fixture(lane_index=1)
    lfc1b, vg1b, hg1b = _lane_fixture(lane_index=1)

    tg.add_lane(vehicle_gen=vg1a, headway_gen=hg1a, lfc=lfc1a)
    with pytest.raises(ValueError):
        tg.add_lane(vehicle_gen=vg1b, headway_gen=hg1b, lfc=lfc1b)


def test_traffic_generator_out_of_range_lane_index_raises():
    tg = pb.TrafficGenerator(no_lane=2)

    lfc3, vg3, hg3 = _lane_fixture(lane_index=3)
    with pytest.raises(ValueError):
        tg.add_lane(vehicle_gen=vg3, headway_gen=hg3, lfc=lfc3)


# --- FIX 2: Bridge.add_load_effect state corruption -------------------------


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


# --- FIX 3: vehicle_list_to_df Year/Day swap --------------------------------


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


# --- FIX 4: VehicleGenGarage kernel validation and->or ----------------------


def test_garage_kernel_validation_rejects_wrong_number_of_sublists():
    with pytest.raises(ValueError):
        pb.VehicleGenGarage(garage=[], kernel=[[100.0, 10.0], [50.0, 5.0]])


def test_garage_kernel_validation_rejects_malformed_sublist():
    with pytest.raises(ValueError):
        pb.VehicleGenGarage(garage=[], kernel=[[1.0, 0.1], [1.0, 0.1], [1.0, 0.1, 0.1]])


def test_garage_kernel_validation_accepts_valid_kernel():
    vehicle_gen = pb.VehicleGenGarage(
        garage=[], kernel=[[1.0, 0.1], [1.0, 0.1], [1.0, 0.1]]
    )
    assert vehicle_gen is not None


# --- FIX 5: write_garage_file with a str path -------------------------------


def test_write_garage_file_accepts_str_path(tmp_path):
    vehicle = pb.Vehicle(no_axles=2)
    vehicle.set_axle_weights([80.0, 80.0])
    vehicle.set_axle_spacings([4.0, 0.0])
    vehicle.set_axle_widths([2.0, 2.0])

    out_path = str(tmp_path / "garage_str_path.txt")
    pb.garage.write_garage_file([vehicle], out_path, 4)

    assert (tmp_path / "garage_str_path.txt").exists()


# --- FIX 6: compress_discrete_IL ZeroDivisionError --------------------------


def test_compress_discrete_IL_handles_duplicate_x_discontinuity():
    xs, ys = pb.utils.compress_discrete_IL(
        [0.0, 0.0, 5.0, 10.0], [1.0, 0.0, -0.5, 0.0], 0.05
    )

    # The discontinuity at x=0.0 must be preserved (both points kept).
    assert xs.count(0.0) == 2


# --- FIX 7: raise Warning -> warnings.warn ----------------------------------


def test_influence_line_compress_tolerance_warns_and_continues():
    inf_line = pb.InfluenceLine(IL_type="discrete")
    with pytest.warns(UserWarning):
        inf_line.set_IL(
            position=[0.0, 10.0, 20.0],
            ordinate=[0.0, 5.0, 0.0],
            compress_tolerance=0.2,
        )
    assert inf_line._data_assigned is True


def test_traffic_loader_empty_lane_warns_and_continues():
    vehicle = pb.Vehicle(no_axles=1)
    vehicle.set_axle_weights([100.0])
    vehicle.set_axle_spacings([0.0])
    vehicle.set_axle_widths([2.0])
    vehicle.set_direction(1)
    vehicle.set_local_lane(1)
    vehicle.set_time(0.0)

    # Directly drive _get_traffic_loader() with lane 1 populated and lane 2
    # empty, bypassing add_traffic()'s file/list-derived lane-count logic
    # (which cannot itself represent a declared-but-empty lane).
    traffic_loader = pb.TrafficLoader(no_lane=2)
    traffic_loader._no_lane_dir_1 = 2
    traffic_loader._no_lane_dir_2 = 0
    traffic_loader._lanes_vehicles = [[vehicle], []]

    with pytest.warns(UserWarning):
        loader_list = traffic_loader._get_traffic_loader()

    assert len(loader_list) == 2


# --- Review follow-up: empty loader lane must not silently end the sim ------


def test_traffic_loader_empty_lane_sim_still_processes_vehicles(tmp_path):
    """An initially-empty lane (next arrival stuck at 0.0) must be excluded
    from the merge loop instead of ending the whole simulation on the first
    iteration with zero vehicles processed."""
    from pathlib import Path

    src = Path(__file__).parent / "test_data/test_traffic_file.txt"
    vehicles = pb.garage.read_garage_file(garage_path=src, garage_format=4)
    # keep only dir-1 local-lane-2 vehicles: the file's max lane number stays
    # 2 (so add_traffic accepts no_lane=2) while global lane 1 has no vehicles
    lane2 = [v for v in vehicles if v.get_direction() == 1 and v.get_local_lane() == 2][
        :20
    ]
    traffic_file = tmp_path / "lane2_only.txt"
    pb.garage.write_garage_file(lane2, traffic_file, 4)

    loader = pb.TrafficLoader(no_lane=2)
    loader.add_traffic(
        traffic=traffic_file,
        traffic_format=4,
        use_average_speed=False,
        use_const_speed=True,
        const_speed_value=40.0,
    )

    il = pb.InfluenceLine(IL_type="built-in")
    il.set_IL(id=1, length=20.0)
    bridge = pb.Bridge(length=20.0, no_lane=2)
    bridge.add_load_effect(inf_line_surf=il, inf_weight=[1.0, 1.0], threshold=0.0)

    output_config = pb.OutputConfig()
    output_config.set_vehicle_file_output(
        write_vehicle_file=True,
        vehicle_file_format=4,
        vehicle_file_name="proof_traffic_file.txt",
    )

    sim = pb.Simulation(output_dir=tmp_path / "out")
    sim.add_sim(
        bridge=bridge,
        traffic=loader,
        output_config=output_config,
        time_step=0.5,
        min_gvw=35,
        tag="EmptyLane",
        track_progress=False,
    )
    sim.run(no_core=1)

    traffic_data = sim.get_output()["EmptyLane"].read_data("traffic")
    assert traffic_data, "no traffic output produced"
    df = next(iter(traffic_data.values()))
    # the CPU read loop loses only the final read's own crossing bookkeeping,
    # never the whole stream: all (or all but the last) vehicles must appear
    assert len(df) >= len(lane2) - 1, f"only {len(df)} of {len(lane2)} processed"


# --- Review follow-up: unfilled lane slots must fail loudly -----------------


def test_traffic_generator_missing_lane_raises():
    tg = pb.TrafficGenerator(no_lane=2)
    lfc1, vg1, hg1 = _lane_fixture(lane_index=1)
    tg.add_lane(vehicle_gen=vg1, headway_gen=hg1, lfc=lfc1)

    with pytest.raises(ValueError, match="lane index"):
        tg._get_traffic_generator(20.0)
