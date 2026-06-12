"""
Output-integrity tests targeting failure modes found during the parallel
work: silent-block index misalignment (DR-1/DR-10), the lost final
statistics interval (DR-9), statistics column mislabelling (DR-11), and
vehicle-file round-trip fidelity.
"""

import numpy as np
import pandas as pd
import pybtls as pb
import pytest
from pathlib import Path
from utils import remove_folder

NO_DAY = 2


def _make_bridge() -> "pb.Bridge":
    inf_line = pb.InfluenceLine(IL_type="built-in")
    inf_line.set_IL(id=1, length=20.0)
    bridge = pb.Bridge(length=20.0, no_lane=2)
    bridge.add_load_effect(inf_line_surf=inf_line, threshold=5.0)
    return bridge


def _make_traffic(trucks_per_hour: list[int]) -> "pb.TrafficGenerator":
    garage = pb.garage.read_garage_file(
        garage_path=Path(__file__).parent / "test_data/garage.txt", garage_format=4
    )
    kernel = [[1.0, 0.08], [1.0, 0.05], [1.0, 0.02]]
    traffic_gen = pb.TrafficGenerator(no_lane=2)
    for lane_index, lane_dir in ((1, 1), (2, 2)):
        lfc = pb.LaneFlowComposition(lane_index=lane_index, lane_dir=lane_dir)
        lfc.assign_lane_data(
            hourly_truck_flow=trucks_per_hour,
            hourly_car_flow=[2] * 24,
            hourly_speed_mean=[40 / 3.6 * 10] * 24,
            hourly_speed_std=[5.0] * 24,
            hourly_truck_composition=[[25.0, 25.0, 25.0, 25.0] for _ in range(24)],
        )
        traffic_gen.add_lane(
            vehicle_gen=pb.VehicleGenGarage(garage=garage, kernel=kernel),
            headway_gen=pb.HeadwayGenFreeflow(),
            lfc=lfc,
        )
    traffic_gen.set_start_time(0.0)
    return traffic_gen


def _run(traffic, out_dir: Path, tag: str, interval_size: int = 3600,
         pot_secs: int = 0):
    output_config = pb.OutputConfig()
    output_config.set_event_output(write_each_event=True)
    output_config.set_BM_output(write_vehicle=False, write_summary=True, write_mixed=False)
    output_config.set_POT_output(
        write_vehicle=False, write_summary=False, write_counter=True,
        POT_size_days=0 if pot_secs else 1, POT_size_secs=pot_secs,
    )
    output_config.set_stats_output(
        write_flow_stats=False, write_overall=True, write_intervals=True,
        interval_size=interval_size,
    )

    sim = pb.Simulation(output_dir=out_dir)
    sim.add_sim(
        bridge=_make_bridge(), traffic=traffic, no_day=NO_DAY,
        output_config=output_config, time_step=0.1, min_gvw=35, tag=tag, seed=99,
    )
    sim.run(no_core=1)
    return sim.get_output()[tag]


@pytest.fixture(scope="module")
def outputs():
    root = Path(__file__).parent / "temp_integrity"
    remove_folder(root)
    # "sparse": ~1 truck/h with 10-min intervals/blocks, so many intervals
    # carry no events - exercises the silent-block paths (DR-9/DR-10).
    # NB zero hourly flow is not usable here: it permanently silences the
    # lane (DR-14 in dev_log/design_review.md).
    results = {
        "busy": _run(_make_traffic([60] * 24), root, "busy"),
        "sparse": _run(
            _make_traffic([1] * 24), root, "sparse", interval_size=600, pot_secs=600
        ),
    }
    yield results
    remove_folder(root)


def test_interval_indices_complete_and_aligned(outputs):
    # DR-9 (lost final interval) and DR-10 (silent-interval lag): every
    # interval of the simulation must be present exactly once.
    data = outputs["busy"].read_data("E_interval_statistics")
    for df in data.values():
        assert df["Index"].tolist() == list(range(1, NO_DAY * 24 + 1))
        assert df["Time"].tolist() == [3600 * i for i in range(1, NO_DAY * 24 + 1)]

    data = outputs["sparse"].read_data("E_interval_statistics")
    for df in data.values():
        assert df["Index"].tolist() == list(range(1, NO_DAY * 144 + 1))


def test_block_indices_complete_and_aligned(outputs):
    data = outputs["busy"].read_data("POT_counter")
    for df in data.values():
        assert df["Block"].tolist() == list(range(1, NO_DAY + 1))

    data = outputs["sparse"].read_data("POT_counter")
    for df in data.values():
        assert df["Block"].tolist() == list(range(1, NO_DAY * 144 + 1))

    for which in ("busy", "sparse"):
        data = outputs[which].read_data("BM_summary")
        for df in data.values():
            assert df["Block Index"].tolist() == list(range(1, NO_DAY + 1))


def test_silent_intervals_have_zero_events(outputs):
    # With ~1 truck/h and 10-minute intervals, most intervals carry no
    # events - they must exist with No. Events == 0 rather than shifting
    # later events into earlier rows.
    data = outputs["sparse"].read_data("E_interval_statistics")
    for df in data.values():
        empty = df[df["No. Events"] == 0]
        assert len(empty) > len(df) / 2
        assert (empty["Mean"] == 0.0).all()
        assert (empty["Std Dev"] == 0.0).all()


def test_cumulative_stats_match_event_data(outputs):
    # DR-11 regression: the SS_C columns must numerically match statistics
    # computed from the raw event list (they were mislabelled before).
    out = outputs["busy"]
    events = next(iter(out.read_data("all_events").values()))
    stats = next(iter(out.read_data("E_cumulative_statistics").values()))

    values = events["Effect 1"]
    row = stats[stats["Effect"] == 1].iloc[0]

    assert row["No. Events"] == len(values)
    assert row["Min"] == pytest.approx(values.min(), abs=0.011)
    assert row["Max"] == pytest.approx(values.max(), abs=0.011)
    assert row["Mean"] == pytest.approx(values.mean(), rel=1e-3)
    assert row["Std Dev"] == pytest.approx(values.std(ddof=1), rel=1e-3)
    assert row["Variance"] == pytest.approx(values.var(ddof=1), rel=1e-3)
    g1 = float(((values - values.mean()) ** 3).mean() / values.std(ddof=0) ** 3)
    assert row["Skewness"] == pytest.approx(g1, abs=0.02)


def test_interval_stats_sum_to_cumulative(outputs):
    out = outputs["busy"]
    intervals = next(iter(out.read_data("E_interval_statistics").values()))
    stats = next(iter(out.read_data("E_cumulative_statistics").values()))
    assert intervals["No. Events"].sum() == stats["No. Events"].sum()


def test_zero_flow_hours_recover():
    # DR-14: an hour with zero total flow must produce no arrivals in that
    # hour AND must not silence the lane for the rest of the simulation.
    root = Path(__file__).parent / "temp_zero_flow"
    remove_folder(root)
    flow = [10] * 6 + [0] * 6 + [10] * 6 + [0] * 6
    out = _run(_make_traffic(flow), root, "zeroflow")

    events = next(iter(out.read_data("all_events").values()))
    second_of_day = events["Start Time"] % 86400.0

    # silent windows stay silent (small margin for events that started just
    # before the boundary)
    in_silent = (second_of_day > 6.5 * 3600) & (second_of_day < 11.5 * 3600)
    assert in_silent.sum() == 0

    # flow resumes after each silent window, on both days
    in_resumed = (second_of_day >= 12 * 3600) & (second_of_day < 18 * 3600)
    assert (in_resumed & (events["Start Time"] < 86400.0)).sum() > 0
    assert (in_resumed & (events["Start Time"] >= 86400.0)).sum() > 0

    remove_folder(root)


def test_garage_round_trip(tmp_path):
    # Vehicle files must round-trip: read -> write -> read preserves every
    # vehicle's fields (the values are already quantised by the format).
    src = Path(__file__).parent / "test_data/garage.txt"
    vehicles = pb.garage.read_garage_file(garage_path=src, garage_format=4)

    rewritten = tmp_path / "rewritten.txt"
    pb.garage.write_garage_file(vehicles, rewritten, 4)
    again = pb.garage.read_garage_file(garage_path=rewritten, garage_format=4)

    assert len(vehicles) == len(again)
    for v1, v2 in zip(vehicles, again):
        assert v1.get_time() == v2.get_time()
        assert v1.get_gvw() == pytest.approx(v2.get_gvw(), rel=1e-9)
        assert v1.get_no_axles() == v2.get_no_axles()
        assert v1.get_velocity() == pytest.approx(v2.get_velocity(), rel=1e-9)
