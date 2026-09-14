"""
Recorded traffic: TrafficLoader lane handling and its speed overrides.
"""

from pathlib import Path

import numpy as np
import pytest

import pybtls as pb

TRAFFIC_FILE = Path(__file__).parent / "test_data/test_traffic_file.txt"  # MON, 4 lanes


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
    # every vehicle read within the simulated window is written out
    assert len(df) == len(lane2), f"only {len(df)} of {len(lane2)} processed"


def _loaded_speeds(**speed_kwargs):
    loader = pb.TrafficLoader(no_lane=4)
    loader.add_traffic(traffic=TRAFFIC_FILE, traffic_format=4, **speed_kwargs)
    return np.array([v.get_velocity() for lane in loader._lanes_vehicles for v in lane])


def test_loader_use_average_speed_applies_file_mean_to_all_vehicles():
    recorded = _loaded_speeds()
    assert len(np.unique(recorded)) > 1  # the file really has varying speeds

    averaged = _loaded_speeds(use_average_speed=True)

    assert len(np.unique(averaged)) == 1
    assert averaged[0] == pytest.approx(recorded.mean())


def test_loader_use_const_speed_applies_given_value():
    speeds = _loaded_speeds(use_const_speed=True, const_speed_value=54.0)

    assert np.all(speeds == pytest.approx(54.0 / 3.6))
