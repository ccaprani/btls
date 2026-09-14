"""
End-to-end tests for the auto-chunked parallel simulation API:
``Simulation.add_sim(..., no_chunk=N)`` expands one logical simulation
into N seeded day-chunks, runs them across cores, and ``get_output()``
returns a single merged view (_ChunkedOutputManager).
"""

import pandas as pd
import pybtls as pb
import pytest
from pathlib import Path
from utils import remove_folder, run_loader_cpu

from pybtls.output.chunked_manager import _ChunkedOutputManager

DAY = 86400.0
NO_DAY = 4
NO_CHUNK = 2
SEED = 777


def _make_bridge() -> "pb.Bridge":
    inf_line = pb.InfluenceLine(IL_type="built-in")
    inf_line.set_IL(id=1, length=20.0)
    bridge = pb.Bridge(length=20.0, no_lane=2)
    bridge.add_load_effect(inf_line_surf=inf_line, threshold=5.0)
    return bridge


def _make_traffic_generator() -> "pb.TrafficGenerator":
    garage = pb.garage.read_garage_file(
        garage_path=Path(__file__).parent / "test_data/garage.txt", garage_format=4
    )
    kernel = [[1.0, 0.08], [1.0, 0.05], [1.0, 0.02]]

    traffic_gen = pb.TrafficGenerator(no_lane=2)
    for lane_index, lane_dir in ((1, 1), (2, 2)):
        lfc = pb.LaneFlowComposition(lane_index=lane_index, lane_dir=lane_dir)
        lfc.assign_lane_data(
            hourly_truck_flow=[40] * 24,
            hourly_car_flow=[10] * 24,
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


def _make_output_config() -> "pb.OutputConfig":
    output_config = pb.OutputConfig()
    output_config.set_event_output(write_time_history=False, write_each_event=True)
    output_config.set_BM_output(
        write_vehicle=True, write_summary=True, write_mixed=True
    )
    output_config.set_POT_output(
        write_vehicle=True, write_summary=True, write_counter=True
    )
    output_config.set_fatigue_output(
        write_fatigue_event=True, write_rainflow_output=True
    )
    output_config.set_stats_output(
        write_flow_stats=True, write_overall=True, write_intervals=True
    )
    output_config.set_vehicle_file_output(
        write_vehicle_file=True, vehicle_file_format=4, vehicle_file_name="traffic.txt"
    )
    return output_config


def _run_chunked(out_dir: Path, seed: int = SEED) -> _ChunkedOutputManager:
    sim = pb.Simulation(output_dir=out_dir)
    sim.add_sim(
        bridge=_make_bridge(),
        traffic=_make_traffic_generator(),
        no_day=NO_DAY,
        output_config=_make_output_config(),
        time_step=0.1,
        min_gvw=35,
        tag="chunked",
        seed=seed,
        no_chunk=NO_CHUNK,
    )
    sim.run(no_core=NO_CHUNK)
    return sim.get_output()["chunked"]


@pytest.fixture(scope="module")
def chunked_output():
    root = Path(__file__).parent / "temp_auto_chunk"
    remove_folder(root)
    output = _run_chunked(root / "run1")
    yield output
    remove_folder(root)


def test_returns_single_merged_manager(chunked_output):
    assert isinstance(chunked_output, _ChunkedOutputManager)
    assert chunked_output.tag == "chunked"
    assert chunked_output.no_chunk == NO_CHUNK
    assert chunked_output.master_seed == SEED


def test_all_outputs_readable_and_merged(chunked_output):
    # Every configured output must be present and merge without error.
    keys = chunked_output.get_summary()
    expected = {
        "all_events",
        "BM_by_no_trucks",
        "BM_by_mixed",
        "BM_summary",
        "POT_vehicle",
        "POT_summary",
        "POT_counter",
        "traffic",
        "traffic_statistics",
        "E_cumulative_statistics",
        "E_interval_statistics",
        "fatigue_events",
        "fatigue_rainflow",
    }
    assert expected <= set(keys)

    for key in keys:
        data = chunked_output.read_data(key)
        assert data, f"no files merged for {key}"
        for stem, df in data.items():
            assert isinstance(df, pd.DataFrame)


def test_merged_timeline_is_continuous(chunked_output):
    # Block/interval counters must continue across the chunk boundary and
    # times must extend beyond the first chunk.
    bm = chunked_output.read_data("BM_summary")
    for df in bm.values():
        assert df["Block Index"].tolist() == list(range(1, NO_DAY + 1))

    ae = chunked_output.read_data("all_events")
    for df in ae.values():
        times = df["Start Time"]
        assert times.is_monotonic_increasing
        assert times.iloc[-1] > (NO_DAY - 1) * DAY  # events on the last day

    ss = chunked_output.read_data("E_interval_statistics")
    for df in ss.values():
        assert df["Index"].tolist() == list(range(1, NO_DAY * 24 + 1))

    pc = chunked_output.read_data("POT_counter")
    for df in pc.values():
        assert df["Block"].tolist() == list(range(1, NO_DAY + 1))


def test_chunked_run_is_reproducible(chunked_output):
    # Same master seed -> identical merged outputs.
    root = Path(__file__).parent / "temp_auto_chunk"
    rerun = _run_chunked(root / "run2")

    first = chunked_output.read_data("all_events")
    second = rerun.read_data("all_events")
    assert set(first) == set(second)
    for stem in first:
        pd.testing.assert_frame_equal(first[stem], second[stem])


def test_chunking_validation_errors():
    bridge = _make_bridge()
    traffic = _make_traffic_generator()
    output_config = _make_output_config()
    sim = pb.Simulation(output_dir=Path(__file__).parent / "temp_auto_chunk_err")

    with pytest.raises(ValueError, match="divisible"):
        sim.add_sim(
            bridge=bridge,
            traffic=traffic,
            no_day=5,
            output_config=output_config,
            no_chunk=2,
        )
    with pytest.raises(ValueError, match="no_day"):
        sim.add_sim(
            bridge=bridge,
            traffic=traffic,
            output_config=output_config,
            no_chunk=2,
        )
    with pytest.raises(ValueError, match=">= 2"):
        sim.add_sim(
            bridge=bridge,
            traffic=traffic,
            no_day=4,
            output_config=output_config,
            no_chunk=0,
        )

    loader = pb.TrafficLoader(no_lane=2)
    with pytest.raises(ValueError, match="TrafficGenerator"):
        sim.add_sim(
            bridge=bridge,
            traffic=loader,
            no_day=4,
            output_config=output_config,
            no_chunk=2,
        )

    # Chunk length misaligned with the BM block size.
    bad_config = _make_output_config()
    bad_config.set_BM_output(
        write_vehicle=True, write_summary=True, write_mixed=True, block_size_days=4
    )
    with pytest.raises(ValueError, match="block size"):
        sim.add_sim(
            bridge=bridge,
            traffic=traffic,
            no_day=8,
            output_config=bad_config,
            no_chunk=4,
        )

    remove_folder(Path(__file__).parent / "temp_auto_chunk_err")


# --- hand-built two-chunk outputs from real C++ runs ------------------------


def _chunk(root, tag, heavy):
    # one 100 kN truck (or a 1 kN car) at 0 s, and a 1 kN car at 30 s; the BM
    # and POT vehicle files hold the truck as a Vehicle object
    cfg = pb.OutputConfig()
    cfg.set_BM_output(write_vehicle=True, write_mixed=True)
    cfg.set_POT_output(write_vehicle=True)
    return run_loader_cpu(root, tag, [(0.0, 100.0 if heavy else 1.0), (30.0, 1.0)], cfg)


def test_merged_event_vehicles_are_shifted_with_the_event_time(tmp_path):
    first, second = _chunk(tmp_path, "a", True), _chunk(tmp_path, "b", True)
    merged = _ChunkedOutputManager([first, second], [1, 1], "merged")
    for key in ("BM_by_no_trucks", "BM_by_mixed", "POT_vehicle"):
        (df,) = merged.read_data(key).values()
        assert len(df) == 2, key
        for _, row in df.iterrows():
            # the truck arrives at 0 s in its chunk and governs at 19.9 s: that
            # gap must survive the merge for every embedded vehicle
            for vehicle in row["Trucks"]:
                assert row["Time"] - vehicle.get_time() == pytest.approx(19.9), key
        assert [v.get_time() for v in df["Trucks"].iloc[-1]] == [86400.0], key
        # the chunk's own objects are left as read
        (own,) = second.read_data(key).values()
        assert [v.get_time() for v in own["Trucks"].iloc[0]] == [0.0], key


def test_chunk_without_a_bm_vehicle_file_does_not_hide_the_others(tmp_path):
    empty, full = _chunk(tmp_path, "empty", False), _chunk(tmp_path, "full", True)
    assert "BM_by_no_trucks" not in empty.get_summary()  # no truck, no BM_V_20_1
    merged = _ChunkedOutputManager([empty, full], [1, 1], "merged")
    assert "BM_by_no_trucks" in merged.get_summary()
    assert set(merged.get_summary()) == set(merged.get_summary(with_path=True))

    (df,) = merged.read_data("BM_by_no_trucks").values()
    assert df["Index"].tolist() == [2]  # the truck's day is the second block
    assert [v.get_time() for v in df["Trucks"].iloc[0]] == [86400.0]
    assert merged.read_chunk_data("BM_by_no_trucks")[0] == {}
    with pytest.raises(ValueError, match="invalid"):
        merged.read_data("time_history")  # no chunk has it
