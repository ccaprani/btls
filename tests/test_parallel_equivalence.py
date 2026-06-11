"""
End-to-end exactness test for the chunk-merge layer.

Strategy ("split-replay"): record a 2-day traffic stream with a guard gap
around midnight (no vehicle on the bridge at the split instant), then

* replay the full stream in ONE simulation        -> reference outputs
* replay day 1 and day 2 (rebased to t=0) apart   -> chunk outputs
* merge the chunk outputs with the merge primitives

Because all three replays feed identical vehicles through the deterministic
load calculation, the merged outputs must equal the reference outputs up to
text-formatting precision. This validates the merge layer against the real
C++ accumulator semantics, sidestepping the fact that seeded chunks are
different random realisations.

Known exceptions (exact handling lands in Phase 2):
* fatigue_rainflow — residual reversals are closed at each chunk end, so the
  merged histogram may differ slightly near the boundary (lenient check).
* E_cumulative_statistics — whole-run moments need Welford/Chan combination.
"""

import pandas as pd
import pybtls as pb
import pytest
from pathlib import Path
from utils import remove_folder

from pybtls.output._merge import (
    MERGE_REGISTRY,
    merge_bin_sum,
    merge_concat,
    merge_cumulative_stats,
)

DAY = 86400.0
GUARD = 15.0  # s, > max bridge crossing time (20 m at >5 m/s)
SEED = 20260612
TIME_STEP = 0.1
MIN_GVW = 35

# Output keys checked exactly in this test (concat category, Phase 1 scope).
EXACT_KEYS = [
    "time_history",
    "all_events",
    "BM_by_no_trucks",
    "BM_by_mixed",
    "BM_summary",
    "POT_vehicle",
    "POT_summary",
    "POT_counter",
    "traffic_statistics",
    "E_interval_statistics",
    "fatigue_events",
]


def _make_bridge() -> "pb.Bridge":
    inf_line = pb.InfluenceLine(IL_type="built-in")
    inf_line.set_IL(id=1, length=20.0)
    inf_line_2 = pb.InfluenceLine(IL_type="discrete")
    inf_line_2.set_IL(position=[0.0, 10.0, 20.0], ordinate=[0.0, 5.0, 0.0])

    bridge = pb.Bridge(length=20.0, no_lane=2)
    bridge.add_load_effect(inf_line_surf=inf_line, threshold=5.0)
    bridge.add_load_effect(inf_line_surf=inf_line_2, threshold=5.0)
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
            hourly_truck_flow=[80] * 24,
            hourly_car_flow=[20] * 24,
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


def _record_traffic(out_root: Path) -> Path:
    """Generate 2 days of traffic, writing only the vehicle file."""
    output_config = pb.OutputConfig()
    output_config.set_vehicle_file_output(
        write_vehicle_file=True,
        vehicle_file_format=4,
        vehicle_file_name="recorded_traffic.txt",
    )

    sim = pb.Simulation(output_dir=out_root)
    sim.add_sim(
        bridge=_make_bridge(),
        traffic=_make_traffic_generator(),
        no_day=2,
        output_config=output_config,
        time_step=TIME_STEP,
        min_gvw=MIN_GVW,
        tag="record",
        seed=SEED,
    )
    sim.run(no_core=1)
    return out_root / "record" / "recorded_traffic.txt"


def _split_traffic(recorded: Path, out_dir: Path) -> tuple[Path, Path, Path]:
    """
    Doctor the recorded stream: drop vehicles arriving inside the guard
    window around midnight (ensures an empty bridge at the split), then
    write the full stream plus the two day-halves (second half rebased).
    """
    vehicles = pb.garage.read_garage_file(garage_path=recorded, garage_format=4)

    kept = [v for v in vehicles if abs(v.get_time() - DAY) > GUARD]
    half_a = [v for v in kept if v.get_time() < DAY]
    half_b = [v for v in kept if v.get_time() > DAY]

    assert half_a and half_b, "fixture must contain traffic on both days"

    paths = (out_dir / "full.txt", out_dir / "half_a.txt", out_dir / "half_b.txt")
    # half_b aliases objects in kept — write full/half_a BEFORE rebasing times.
    pb.garage.write_garage_file(kept, paths[0], 4)
    pb.garage.write_garage_file(half_a, paths[1], 4)
    for v in half_b:
        v.set_time(v.get_time() - DAY)
    pb.garage.write_garage_file(half_b, paths[2], 4)
    return paths


def _replay(traffic_files: dict[str, tuple[Path, int]], out_root: Path) -> dict:
    """Replay each recorded stream through an identical bridge."""
    output_config = pb.OutputConfig()
    output_config.set_event_output(write_time_history=True, write_each_event=True)
    output_config.set_BM_output(write_vehicle=True, write_summary=True, write_mixed=True)
    output_config.set_POT_output(write_vehicle=True, write_summary=True, write_counter=True)
    output_config.set_fatigue_output(write_fatigue_event=True, write_rainflow_output=True)
    output_config.set_stats_output(write_flow_stats=True, write_overall=True, write_intervals=True)

    sim = pb.Simulation(output_dir=out_root)
    for tag, (traffic_file, no_day) in traffic_files.items():
        loader = pb.TrafficLoader(no_lane=2)
        loader.add_traffic(
            traffic=traffic_file,
            traffic_format=4,
            use_average_speed=False,
            use_const_speed=False,
        )
        sim.add_sim(
            bridge=_make_bridge(),
            traffic=loader,
            no_day=no_day,
            output_config=output_config,
            time_step=TIME_STEP,
            min_gvw=MIN_GVW,
            tag=tag,
        )
    sim.run(no_core=3)
    return sim.get_output()


@pytest.fixture(scope="module")
def replayed_outputs():
    root = Path(__file__).parent / "temp_equivalence"
    remove_folder(root)

    recorded = _record_traffic(root / "fixture")
    full, half_a, half_b = _split_traffic(recorded, root / "fixture")
    outputs = _replay(
        {"full": (full, 2), "chunk_a": (half_a, 1), "chunk_b": (half_b, 1)},
        root / "replay",
    )

    yield outputs
    remove_folder(root)


def _frames(output, key) -> dict[str, pd.DataFrame]:
    data = output.read_data(key)
    # The Trucks column holds Vehicle objects — excluded from comparison.
    return {
        stem: df.drop(columns=["Trucks"], errors="ignore") for stem, df in data.items()
    }


@pytest.mark.parametrize("key", EXACT_KEYS)
def test_merged_chunks_equal_full_run(replayed_outputs, key):
    full = _frames(replayed_outputs["full"], key)
    chunk_a = _frames(replayed_outputs["chunk_a"], key)
    chunk_b = _frames(replayed_outputs["chunk_b"], key)

    assert set(full) == set(chunk_a) == set(chunk_b), f"file sets differ for {key}"

    spec = MERGE_REGISTRY[key]
    for stem in full:
        merged = merge_concat([chunk_a[stem], chunk_b[stem]], spec, [0.0, DAY])
        # Tolerances cover text-formatting quanta only: fixed-point files
        # round to 0.01 (atol), and AllEvents/POT write times with C++
        # default 6-significant-digit formatting, so the quantum scales
        # with magnitude (rtol). The underlying doubles agree to ~1e-12.
        pd.testing.assert_frame_equal(
            merged,
            full[stem],
            check_exact=False,
            rtol=1e-5,
            atol=0.011,
            obj=f"{key}/{stem}",
        )


def test_merged_cumulative_stats_equal_full_run(replayed_outputs):
    # SS_C is merged by inverting each chunk's reported statistics back to
    # raw moment sums and combining them with the Chan parallel formulas.
    # The combination is mathematically exact; tolerances cover the 0.01
    # quantisation of the input files (amplified slightly by the M3/M4
    # inversion for skewness/kurtosis).
    key = "E_cumulative_statistics"
    full = _frames(replayed_outputs["full"], key)
    chunk_a = _frames(replayed_outputs["chunk_a"], key)
    chunk_b = _frames(replayed_outputs["chunk_b"], key)

    assert set(full) == set(chunk_a) == set(chunk_b)

    for stem in full:
        merged = merge_cumulative_stats([chunk_a[stem], chunk_b[stem]])
        pd.testing.assert_frame_equal(
            merged,
            full[stem],
            check_exact=False,
            rtol=1e-3,
            atol=0.03,
            obj=f"{key}/{stem}",
        )


def test_merged_rainflow_close_to_full_run(replayed_outputs):
    # Exact residual-reversal splicing lands in Phase 2; until then the
    # boundary closure may shift a handful of cycles between bins.
    key = "fatigue_rainflow"
    full = _frames(replayed_outputs["full"], key)
    chunk_a = _frames(replayed_outputs["chunk_a"], key)
    chunk_b = _frames(replayed_outputs["chunk_b"], key)

    spec = MERGE_REGISTRY[key]
    for stem in full:
        merged = merge_bin_sum([chunk_a[stem], chunk_b[stem]], spec)
        total_full = full[stem]["No. Cycles"].sum()
        total_merged = merged["No. Cycles"].sum()
        assert total_full > 0
        assert abs(total_merged - total_full) <= max(4, 0.01 * total_full)
