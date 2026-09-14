"""GPU-engine tests that need PyTorch but no CUDA: they drive the full
runner/engine pipeline on the torch CPU backend (``device="cpu"``), so they run
on any machine with PyTorch installed (the public ``Simulation`` API forces
device == engine, so a real GPU run needs CUDA; see tests/test_gpu_engine.py).

  * the POT sample->window clamp never leaks out-of-partition samples into a
    sub-time-step window as a phantom peak_value=0 event;
  * a streamed run keeps every window's sample grid on the global k*ts lattice,
    so results are independent of the (memory-driven) window split even when
    time_step does not divide a day;
  * the flow statistics bin arrivals into the C++ hour grid.
"""

import os
import shutil
from pathlib import Path

import numpy as np
import pandas as pd
import pytest

import pybtls as pb
from pybtls.gpu import engine as eng
from pybtls.gpu import pot as potmod
from pybtls.gpu import runner as gpu_runner
from utils import run_loader_cpu, run_loader_torch

try:
    import torch  # noqa: F401

    HAS_TORCH = True
except ImportError:  # pragma: no cover
    HAS_TORCH = False

# These cases need no CUDA, so a CI leg with a CPU PyTorch wheel sets
# PYBTLS_REQUIRE_TORCH=1 and a missing torch fails there instead of skipping.
if os.environ.get("PYBTLS_REQUIRE_TORCH") == "1" and not HAS_TORCH:
    raise RuntimeError(
        "PYBTLS_REQUIRE_TORCH=1 but PyTorch is not installed: the torch-CPU "
        "GPU-engine tests would be skipped."
    )

pytestmark = pytest.mark.skipif(
    not HAS_TORCH, reason="requires PyTorch (uses the torch-CPU backend)"
)

GARAGE = Path(__file__).parent / "test_data/garage.txt"
ROOT = Path(__file__).parent / "temp_gpu_torch_cpu"


def _clean():
    shutil.rmtree(ROOT, ignore_errors=True)


# --- POT sample->window clamp -------------------------------------------------


def test_pot_clamp_no_phantom_events():
    # Two single-axle vehicles arriving 5.03 s / 5.09 s (crossing 3 s, L=20,
    # ts=0.1). The event partition B = [5.03, 5.09, 8.03, 8.09] has two
    # sub-time-step windows (0 and 2) that contain NO grid sample k*0.1. Their
    # peak_index must stay -1 (function contract), so the runner's ev_mask gate
    # rejects them instead of counting phantom peak_value=0 events.
    L, ts = 20.0, 0.1
    v = L / 3.0  # zero-length vehicle: t_off - t_on = L/v = 3 s
    axles = {
        "datum": np.array([5.03, 5.09]),
        "sign": np.array([1.0, 1.0]),
        "speed": np.array([v, v]),
        "weight": np.array([980.0, 980.0]),
    }
    t_on = np.array([5.03, 5.09])
    t_off = t_on + 3.0
    B, win_count, _, _ = potmod.build_partition(t_on, t_off)
    n_total = int(np.ceil((5.09 + L / v) / ts)) + 1

    il = [{"kind": "builtin", "id": 1, "length": L}]
    out = eng.compute_from_axles(
        axles, n_total, il, [1.0], L, ts, device="cpu", pot_boundaries=B
    )
    pi = out["pot_peak_index"][0]
    pv = out["pot_peak_value"][0]

    # sample-less boundary windows keep peak_index == -1 (no phantom event)
    assert pi[0] == -1 and pi[2] == -1, f"phantom peak_index leaked: {pi}"
    assert pi[1] >= 0, "the real (middle) window lost its sample"
    assert pv[0] == 0.0 and pv[2] == 0.0  # value untouched, but gated out below

    # the runner's event gate now rejects windows 0 and 2
    starts = B[:-1]
    ev_mask = (starts >= 0.0) & (win_count >= 1) & (pi >= 0)
    assert list(ev_mask) == [False, True, False], f"ev_mask={ev_mask}"


def test_pot_clamp_no_trailing_phantom():
    # A single vehicle whose partition's last window is sub-ts, plus a grid
    # sample past B[-1] (the n_total +1 margin): the trailing clamp used to leak
    # a phantom into the last window. It must now stay -1.
    L, ts = 20.0, 0.1
    v = L / 3.0
    axles = {
        "datum": np.array([2.03]),
        "sign": np.array([1.0]),
        "speed": np.array([v]),
        "weight": np.array([980.0]),
    }
    t_on = np.array([2.03])
    t_off = t_on + 3.0  # 5.03 -> last boundary 5.03, sub-ts final region
    B, win_count, _, _ = potmod.build_partition(t_on, t_off)
    n_total = int(np.ceil((2.03 + L / v) / ts)) + 1
    il = [{"kind": "builtin", "id": 1, "length": L}]
    out = eng.compute_from_axles(
        axles, n_total, il, [1.0], L, ts, device="cpu", pot_boundaries=B
    )
    pi = out["pot_peak_index"][0]
    # every window with a real grid sample is fine; none may be a phantom from a
    # sample outside [B[0], B[-1])
    t = np.arange(n_total) * ts
    for w in range(len(win_count)):
        own = ((t >= B[w]) & (t < B[w + 1])).sum()
        if own == 0:
            assert pi[w] == -1, f"phantom in window {w}: peak_index={pi[w]}"


# --- split-invariant global sample lattice -----------------------------------


def _gen_bridge():
    il = pb.InfluenceLine(IL_type="discrete")
    il.set_IL(position=[0.0, 10.0, 20.0], ordinate=[0.0, 10.0, 0.0])
    b = pb.Bridge(length=20.0, no_lane=2)
    b.add_load_effect(inf_line_surf=il, threshold=0.0)
    return b


def _gen_traffic():
    garage = pb.garage.read_garage_file(garage_path=GARAGE, garage_format=4)
    g = pb.TrafficGenerator(no_lane=2)
    for i in (1, 2):
        lfc = pb.LaneFlowComposition(lane_index=i, lane_dir=1)
        lfc.assign_lane_data(
            hourly_truck_flow=[20] * 24,
            hourly_car_flow=[5] * 24,
            hourly_speed_mean=[40 / 3.6 * 10] * 24,
            hourly_speed_std=[5.0] * 24,
            hourly_truck_composition=[[25.0, 25.0, 25.0, 25.0] for _ in range(24)],
        )
        g.add_lane(
            vehicle_gen=pb.VehicleGenGarage(
                garage=garage, kernel=[[1.0, 0.08], [1.0, 0.05], [1.0, 0.02]]
            ),
            headway_gen=pb.HeadwayGenFreeflow(),
            lfc=lfc,
        )
    g.set_start_time(0.0)
    return g


def _bm_values(time_step, target, no_day=2, seed=7):
    """Block-maxima matrix from a generated GPU run on the torch-CPU backend,
    forcing the per-window vehicle budget to ``target`` to control the split."""
    _clean()
    saved = gpu_runner._window_target_vehicles
    gpu_runner._window_target_vehicles = lambda *a, **k: target
    try:
        om = gpu_runner.run(
            _gen_bridge(),
            _gen_traffic(),
            no_day,
            time_step,
            0,
            None,
            "g",
            0.0,
            ROOT,
            seed,
            device="cpu",
        )
        df = next(iter(om.read_data("BM_summary").values()))
        return df[[c for c in df.columns if c != "Block Index"]].values
    finally:
        gpu_runner._window_target_vehicles = saved
        _clean()


def test_streamed_equals_single_window_ts_007():
    # ts=0.07 does NOT divide a day: without the global-lattice fix the second
    # window's grid is phase-shifted, so the forced 2-window split gives
    # different block maxima than a single window. With the fix they are equal.
    one = _bm_values(0.07, 10**12)  # single window (whole run)
    many = _bm_values(0.07, 200)  # ~1-day windows -> 2 windows over 2 days
    assert one.shape == many.shape and one.shape[0] == 2, f"{one.shape} {many.shape}"
    assert np.array_equal(
        one, many
    ), f"ts=0.07 split-dependent: max|d|={np.abs(one - many).max()}"


def test_streamed_equals_single_window_ts_01():
    # ts=0.1 divides a day (grid_phase == 0): the existing invariant must still
    # hold bit-for-bit (guards against the fix perturbing the default path).
    one = _bm_values(0.1, 10**12)
    many = _bm_values(0.1, 200)
    assert np.array_equal(
        one, many
    ), f"ts=0.1 regressed: max|d|={np.abs(one - many).max()}"


# --- flow statistics ---------------------------------------------------------


def test_flow_statistics_hour_bins_match_cpu(tmp_path):
    # arrivals exactly on the hour: the C++ buffer keeps an arrival on the hour
    # in the hour that ends there (hours are ((h-1)*3600, h*3600]), and t = 0
    # opens hour 1
    cfg = pb.OutputConfig()
    cfg.set_stats_output(write_flow_stats=True)
    rows = [(0.0, 100.0, 20.0), (3600.0, 100.0, 20.0), (7200.0, 100.0, 20.0)]
    cpu = run_loader_cpu(tmp_path, "cpu", rows, cfg, min_gvw=0)
    gpu = run_loader_torch(tmp_path, "gpu", rows, cfg, min_gvw=0)
    c = cpu.read_data("traffic_statistics")["FlowData_1_1"]
    g = gpu.read_data("traffic_statistics")["FlowData_1_1"]
    pd.testing.assert_frame_equal(c, g)
    assert c["No. Vehicles"].tolist()[:3] == [2, 1, 0]
