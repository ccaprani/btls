"""Device-independent unit tests for the GPU engine's pure-numpy logic.

Unlike tests/test_gpu_engine.py (which needs PyTorch + CUDA and is skipped in
CI), these cover the host-side building blocks — the event partition, the flow
statistics accumulator, and the influence-surface / influence-line resampling
decisions — with plain numpy, so they run on every platform in CI with no torch
and no GPU. They lock down the logic that the device kernels build on.
"""

import numpy as np
import pytest

from pybtls.gpu import pot, influence
from pybtls.gpu.stats import StatsAccumulator, _finalize
from pybtls.output.read.E_cumulative_statistics import read_E_CS
from pybtls.output.read.E_interval_statistics import read_E_IS


# --- event partition (pot.py) ------------------------------------------------

def test_build_partition_overlapping():
    # two vehicles, overlapping on the bridge: v0 on [0,10), v1 on [5,15)
    B, win_count, k_start, k_end = pot.build_partition(
        np.array([0.0, 5.0]), np.array([10.0, 15.0]))
    assert np.array_equal(B, [0.0, 5.0, 10.0, 15.0])
    # windows [0,5){v0}, [5,10){v0,v1}, [10,15){v1}
    assert np.array_equal(win_count, [1, 2, 1])
    assert np.array_equal(k_start, [0, 1])
    assert np.array_equal(k_end, [2, 3])


def test_build_partition_disjoint_has_empty_gap_window():
    # v0 on [0,10), v1 on [20,30): the [10,20) gap window has zero occupancy
    B, win_count, k_start, k_end = pot.build_partition(
        np.array([0.0, 20.0]), np.array([10.0, 30.0]))
    assert np.array_equal(B, [0.0, 10.0, 20.0, 30.0])
    assert np.array_equal(win_count, [1, 0, 1])   # middle window is not an event


def test_build_partition_empty():
    B, win_count, k_start, k_end = pot.build_partition(np.array([]), np.array([]))
    assert win_count.size == 0 and k_start.size == 0


def test_truck_occupancy_excludes_cars():
    # same partition as the overlapping case; v0 is a truck, v1 is a car
    _, win_count, k_start, k_end = pot.build_partition(
        np.array([0.0, 5.0]), np.array([10.0, 15.0]))
    is_truck = np.array([True, False])
    wt = pot.truck_occupancy(k_start, k_end, len(win_count), is_truck)
    # only v0 (truck) covers windows 0 and 1; v1 (car) covers windows 1 and 2
    assert np.array_equal(wt, [1, 1, 0])
    # and it never exceeds the all-vehicle occupancy
    assert np.all(wt <= win_count)


def test_window_members_csr():
    _, win_count, k_start, k_end = pot.build_partition(
        np.array([0.0, 5.0]), np.array([10.0, 15.0]))
    indptr, members = pot.window_members_csr(k_start, k_end, len(win_count))
    per_window = [sorted(members[indptr[w]:indptr[w + 1]].tolist())
                  for w in range(len(win_count))]
    assert per_window == [[0], [0, 1], [1]]


# --- flow statistics accumulator (stats.py) ----------------------------------

def _single_effect_events(values):
    """Build (peak_value, win_count, win_truck_count, B) for n events, one effect,
    one vehicle/truck each, at unit-spaced window starts."""
    n = len(values)
    peak = np.array(values, dtype=float)[None, :]          # [1, n]
    win_count = np.ones(n, dtype=np.int64)
    win_truck = np.ones(n, dtype=np.int64)
    B = np.arange(n + 1, dtype=float)
    return peak, win_count, win_truck, B


def test_stats_moments_match_closed_form():
    # values 10..50: mean 30, var 250, skew 0 (symmetric), kurt N*M4/M2^2-3 = -1.3
    acc = StatsAccumulator(n_eff=1)
    peak, wc, wt, B = _single_effect_events([10, 20, 30, 40, 50])
    acc.update(peak, wc, wt, B, time_offset=0.0)
    lo, hi, mean, std, var, skew, kurt = _finalize(
        np.array([acc.n_events]), acc.S1, acc.S2, acc.S3, acc.S4, acc.vmin, acc.vmax)
    assert acc.n_events == 5 and acc.n_veh == 5 and acc.n_trk == 5
    assert mean[0] == pytest.approx(30.0)
    assert var[0] == pytest.approx(250.0)
    assert std[0] == pytest.approx(np.sqrt(250.0))
    assert skew[0] == pytest.approx(0.0, abs=1e-12)
    assert kurt[0] == pytest.approx(-1.3)
    assert lo[0] == 10.0 and hi[0] == 50.0


def test_stats_streaming_is_additive():
    # two windows fed separately must equal one window of the concatenation
    one = StatsAccumulator(n_eff=1)
    p, wc, wt, B = _single_effect_events([10, 20, 30, 40, 50])
    one.update(p, wc, wt, B, 0.0)

    split = StatsAccumulator(n_eff=1)
    p1, wc1, wt1, B1 = _single_effect_events([10, 20])
    p2, wc2, wt2, B2 = _single_effect_events([30, 40, 50])
    split.update(p1, wc1, wt1, B1, 0.0)
    split.update(p2, wc2, wt2, B2, 0.0)

    for a in ("n_events", "n_veh", "n_trk"):
        assert getattr(one, a) == getattr(split, a)
    for arr in ("S1", "S2", "S3", "S4"):
        assert np.allclose(getattr(one, arr), getattr(split, arr))


def test_stats_skips_empty_windows():
    acc = StatsAccumulator(n_eff=1)
    peak = np.array([[10.0, 999.0, 20.0]])       # middle window has no vehicle
    win_count = np.array([1, 0, 1], dtype=np.int64)
    win_truck = np.array([1, 0, 1], dtype=np.int64)
    B = np.array([0.0, 1.0, 2.0, 3.0])
    acc.update(peak, win_count, win_truck, B, 0.0)
    assert acc.n_events == 2                       # the 999 window is not counted
    assert acc.vmax[0] == 20.0 and acc.vmin[0] == 10.0


def test_stats_cumulative_file_roundtrips(tmp_path):
    acc = StatsAccumulator(n_eff=2)
    peak = np.array([[10.0, 20.0, 30.0], [1.0, 2.0, 3.0]])
    wc = np.array([2, 1, 3], dtype=np.int64)
    wt = np.array([1, 1, 2], dtype=np.int64)
    B = np.array([0.0, 1.0, 2.0, 3.0])
    acc.update(peak, wc, wt, B, 0.0)
    acc.write_cumulative(tmp_path / "SS_C_20.txt")
    df = read_E_CS(tmp_path / "SS_C_20.txt")
    assert list(df["Effect"]) == [1, 2]
    assert (df["No. Events"] == 3).all()
    assert (df["No. Vehicles"] == 6).all()        # 2+1+3
    assert (df["No. Trucks"] == 4).all()          # 1+1+2
    assert df.loc[0, "Max"] == pytest.approx(30.0)
    assert df.loc[1, "Min"] == pytest.approx(1.0)


def test_stats_interval_file_silent_intervals(tmp_path):
    # interval_size 1s, two events at t=0 and t=2 -> interval 2 (t in [1,2)) is silent
    acc = StatsAccumulator(n_eff=1, want_intervals=True, interval_size=1.0,
                           total_intervals=3)
    peak = np.array([[10.0, 50.0]])
    wc = np.array([1, 1], dtype=np.int64)
    wt = np.array([1, 1], dtype=np.int64)
    B = np.array([0.0, 2.0, 5.0])                  # event starts at t=0 and t=2
    acc.update(peak, wc, wt, B, 0.0)
    acc.write_intervals(tmp_path, "20")
    df = read_E_IS(tmp_path / "SS_S_20_Eff_1.txt")
    assert len(df) == 3
    assert list(df["Time"]) == [1, 2, 3]
    assert list(df["No. Events"]) == [1, 0, 1]     # middle interval silent
    assert df.loc[1, "Mean"] == 0.0 and df.loc[1, "Max"] == 0.0


# --- influence-surface / influence-line resampling (influence.py) ------------

def test_is_uniform():
    assert influence._is_uniform(np.array([0.0, 1.0, 2.0, 3.0]))
    assert influence._is_uniform(np.array([0.0, 2.5, 5.0]))
    assert not influence._is_uniform(np.array([0.0, 1.0, 3.0]))    # unequal gaps
    assert not influence._is_uniform(np.array([0.0]))              # too short
    assert not influence._is_uniform(np.array([0.0, -1.0]))        # not increasing


def test_uniform_surface_grid_passthrough_and_none():
    X = np.array([0.0, 10.0, 20.0]); Y = np.array([0.0, 8.0, 16.0])
    Z = np.arange(9.0).reshape(3, 3)
    out = influence.uniform_surface_grid(X, Y, Z)
    assert out is not None
    Zu, x0, dx, y0, dy = out
    assert (x0, dx, y0, dy) == (0.0, 10.0, 0.0, 8.0)
    assert np.array_equal(Zu, Z)
    # non-uniform X -> fall back (None), engine then uses the torch searchsorted path
    assert influence.uniform_surface_grid(np.array([0.0, 3.0, 20.0]), Y, Z) is None


def test_resample_il_discrete_matches_interp():
    spec = {"kind": "discrete", "pos": np.array([0.0, 10.0, 20.0]),
            "ord": np.array([0.0, 10.0, 0.0])}
    grid = np.linspace(0.0, 20.0, 21)
    g = influence.resample_il(spec, grid)
    assert np.allclose(g, np.interp(grid, spec["pos"], spec["ord"]))
    assert g[0] == 0.0 and g[-1] == 0.0


# --- unsupported-output warning (runner.py) ----------------------------------

def test_warn_unsupported_outputs(capsys):
    from pybtls.gpu import runner
    from pybtls.output import OutputConfig
    cfg = OutputConfig()
    cfg.set_event_output(write_each_event=True)          # unsupported on GPU
    runner._warn_unsupported_outputs(cfg._Output)
    err = capsys.readouterr().err
    assert "every-event" in err and "engine='cpu'" in err

    cfg2 = OutputConfig()
    cfg2.set_BM_output(write_summary=True)                # all supported
    runner._warn_unsupported_outputs(cfg2._Output)
    assert capsys.readouterr().err == ""
