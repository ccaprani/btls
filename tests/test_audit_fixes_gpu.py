"""Regression tests for the GPU-engine bugs fixed in dev_log/audit_20260704.

These run on the torch **CPU** backend (device="cpu"), so they exercise the full
GPU runner/engine pipeline on any machine with PyTorch installed — no CUDA
needed. The full ``pybtls.Simulation`` public API forces device==engine, so a
GPU run needs CUDA; here we drive ``pybtls.gpu.runner.run(..., device="cpu")``
directly and compare against the C++ ``engine="cpu"`` path.

Covered:
  * Fix 1 — POT sample->window clamp no longer leaks out-of-partition samples
    into the first/last event window as phantom peak_value=0 events.
  * Fix 2 — loader end-of-run drops the tail after the last arrival, matching
    the CPU read-and-sim loop; generated end-of-run owns events through A2 (the
    first arrival strictly after end_time) and counts A2 in the flow statistics,
    matching the CPU `while current_time <= end_time` loop's final iteration.
  * Fix 3 — a streamed run keeps every window's sample grid on the global k*ts
    lattice, so results are independent of the (memory-driven) window split even
    when time_step does not divide a day (e.g. ts=0.07).
"""

import os
import shutil
from pathlib import Path

import numpy as np
import pytest

import pybtls as pb
from pybtls.gpu import engine as eng
from pybtls.gpu import pot as potmod
from pybtls.gpu import runner as gpu_runner

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
        "GPU-engine regression tests would be skipped."
    )

pytestmark = pytest.mark.skipif(
    not HAS_TORCH, reason="requires PyTorch (uses the torch-CPU backend)"
)

GARAGE = Path(__file__).parent / "test_data/garage.txt"
ROOT = Path(__file__).parent / "temp_audit_gpu"


def _clean():
    shutil.rmtree(ROOT, ignore_errors=True)


# --- Fix 1: POT clamp phantom events -----------------------------------------


def test_fix1_pot_clamp_no_phantom_events():
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


def test_fix1_symmetric_trailing_leak():
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


# --- Fix 3: split-invariant global sample lattice ----------------------------


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


def test_fix3_streamed_equals_single_window_ts_007():
    # ts=0.07 does NOT divide a day: without the global-lattice fix the second
    # window's grid is phase-shifted, so the forced 2-window split gives
    # different block maxima than a single window. With the fix they are equal.
    one = _bm_values(0.07, 10**12)  # single window (whole run)
    many = _bm_values(0.07, 200)  # ~1-day windows -> 2 windows over 2 days
    assert one.shape == many.shape and one.shape[0] == 2, f"{one.shape} {many.shape}"
    assert np.array_equal(
        one, many
    ), f"ts=0.07 split-dependent: max|d|={np.abs(one - many).max()}"


def test_fix3_streamed_equals_single_window_ts_01():
    # ts=0.1 divides a day (grid_phase == 0): the existing invariant must still
    # hold bit-for-bit (guards against the fix perturbing the default path).
    one = _bm_values(0.1, 10**12)
    many = _bm_values(0.1, 200)
    assert np.array_equal(
        one, many
    ), f"ts=0.1 regressed: max|d|={np.abs(one - many).max()}"


# --- Fix 2: loader end-of-run matches the CPU read-and-sim cut ----------------


def _truck(lane, t):
    v = pb.Vehicle(2)
    v.set_time(t)
    v.set_velocity(20.0)
    v.set_direction(1)
    v.set_axle_weights([100.0, 100.0])
    v.set_axle_spacings([4.0])
    v.set_axle_widths([2.0, 2.0])
    v.set_trans(1.8)
    v.set_local_lane(lane)
    return v


def _loader_events(engine, tag):
    """No. Events (per effect) from a 3-truck loader run. The last truck (t=100)
    is on the bridge only AFTER its arrival, so the CPU read-and-sim loop never
    records its crossing — the GPU loader path must drop it too."""
    _clean()
    ld = pb.TrafficLoader(no_lane=1)
    ld.add_traffic(traffic=[_truck(1, 10.0), _truck(1, 20.0), _truck(1, 100.0)])
    il = pb.InfluenceLine(IL_type="discrete")
    il.set_IL(position=[0.0, 10.0, 20.0], ordinate=[0.0, 10.0, 0.0])
    b = pb.Bridge(length=20.0, no_lane=1)
    b.add_load_effect(inf_line_surf=il, threshold=0.0)
    cfg = pb.OutputConfig()
    cfg.set_stats_output(write_overall=True)
    from pybtls.output.read.E_cumulative_statistics import read_E_CS

    if engine == "cpu":
        sim = pb.Simulation(output_dir=ROOT)
        sim.add_sim(
            bridge=b,
            traffic=ld,
            output_config=cfg,
            time_step=0.1,
            min_gvw=0,
            tag=tag,
            engine="cpu",
        )
        sim.run(no_core=1)
    else:
        gpu_runner.run(
            b,
            ld,
            1,
            0.1,
            0,
            None,
            tag,
            0.0,
            ROOT,
            None,
            device="cpu",
            output_config=cfg,
        )
    df = read_E_CS(ROOT / tag / "SS_C_20.txt")
    n_events = int(df["No. Events"].iloc[0])
    _clean()
    return n_events


def test_fix2_loader_tail_matches_cpu():
    cpu_n = _loader_events("cpu", "cpu")
    gpu_n = _loader_events("cuda", "gpu")
    # the last truck's crossing is lost by the CPU cut; the GPU must agree
    assert cpu_n == 2, f"CPU baseline unexpected: {cpu_n}"
    assert gpu_n == cpu_n, f"loader end-of-run mismatch: cpu={cpu_n} gpu={gpu_n}"


# Deterministic constant-headway scenario engineered so the last composition
# event STARTS after end_time (86400 s) but before A2 (the first arrival past
# end_time) — exactly the events the CPU records on its final
# `while current_time <= end_time` iteration and the old GPU cut dropped:
#   lane 1: arrivals k*GAP1 -> last in-run arrival 86398.2 s, off at 86400.6 s
#   lane 2: arrivals k*GAP2 -> last in-run arrival 86399.0 s, off at 86401.4 s
#   A2 = 73*GAP2 = 87598.99 s (lane 2's next arrival)
# Events near the end: [86398.2, 86399.0), [86399.0, 86400.6) and
# [86400.6, 86401.4) — the last one starts AFTER end_time (its counter block is
# 2), and A2 lands in FlowData hour row 25. Constant speed 10 m/s and identical
# 4 m trucks keep every composition window >= 0.2 s (>= 2 sample steps), so no
# sub-time-step grid flicker: counts must match the CPU exactly.
GAP1 = 86398.2 / 71
GAP2 = 86399.0 / 72


def _const_truck():
    v = pb.Vehicle(2)
    v.set_time(0.0)
    v.set_velocity(10.0)
    v.set_direction(1)
    v.set_axle_weights([100.0, 100.0])
    v.set_axle_spacings([4.0])
    v.set_axle_widths([2.0, 2.0])
    v.set_trans(0.0)
    v.set_local_lane(1)
    return v


def _const_traffic():
    g = pb.TrafficGenerator(no_lane=2)
    for i, gap in ((1, GAP1), (2, GAP2)):
        lfc = pb.LaneFlowComposition(lane_index=i, lane_dir=1)
        lfc.assign_lane_data(
            hourly_truck_flow=[100] * 24,
            hourly_car_flow=[0] * 24,
            hourly_speed_mean=[100] * 24,
            hourly_speed_std=[0] * 24,
            hourly_truck_composition=[[25.0, 25.0, 25.0, 25.0] for _ in range(24)],
        )
        g.add_lane(
            vehicle_gen=pb.VehicleGenGarage(
                garage=[_const_truck()], kernel=[[1.0, 0.0], [1.0, 0.0], [1.0, 0.0]]
            ),
            headway_gen=pb.HeadwayGenConstant(constant_speed=36.0, constant_gap=gap),
            lfc=lfc,
        )
    g.set_start_time(0.0)
    return g


def _const_bridge():
    il = pb.InfluenceLine(IL_type="discrete")
    il.set_IL(position=[0.0, 10.0, 20.0], ordinate=[0.0, 10.0, 0.0])
    b = pb.Bridge(length=20.0, no_lane=2)
    b.add_load_effect(inf_line_surf=il, threshold=0.0)
    return b


def _end_cfg():
    cfg = pb.OutputConfig()
    cfg.set_stats_output(write_flow_stats=True, write_overall=True)
    cfg.set_POT_output(write_summary=True, write_counter=True)
    return cfg


def _parse_table(path):
    """Whitespace-split data rows (header skipped) of a fixed-width output file."""
    return [ln.split() for ln in Path(path).read_text().splitlines()[1:]]


def test_fix2_generated_end_matches_cpu():
    from pybtls.output.read.E_cumulative_statistics import read_E_CS

    _clean()
    try:
        sim = pb.Simulation(output_dir=ROOT)
        sim.add_sim(
            bridge=_const_bridge(),
            traffic=_const_traffic(),
            no_day=1,
            output_config=_end_cfg(),
            time_step=0.1,
            min_gvw=0,
            tag="cpu",
            engine="cpu",
            seed=1,
        )
        sim.run(no_core=1)
        gpu_runner.run(
            _const_bridge(),
            _const_traffic(),
            1,
            0.1,
            0,
            None,
            "gpu",
            0.0,
            ROOT,
            1,
            device="cpu",
            output_config=_end_cfg(),
        )

        # SS_C: event / vehicle counts (the beyond-end event included)
        cpu = read_E_CS(ROOT / "cpu" / "SS_C_20.txt")
        gpu = read_E_CS(ROOT / "gpu" / "SS_C_20.txt")
        for col in ("No. Events", "No. Vehicles", "No. Trucks"):
            c, g = int(cpu[col].iloc[0]), int(gpu[col].iloc[0])
            assert c == g, f"SS_C {col}: cpu={c} gpu={g}"

        # PT counts: PT_S holds one row per peak, so the counter table must
        # account for every one of them -- that is the witness that the event
        # starting after end_time was recorded. It no longer gets a block of
        # its own: CPOTManager::FinishAt folds a block opened past the end of
        # the simulated window back into the final block, so this one-day run
        # emits exactly one counter row rather than a spurious second one.
        n_cpu = sum(1 for _ in open(ROOT / "cpu" / "PT_S_20_Eff_1.txt"))
        n_gpu = sum(1 for _ in open(ROOT / "gpu" / "PT_S_20_Eff_1.txt"))
        assert n_cpu == n_gpu, f"PT_S rows: cpu={n_cpu} gpu={n_gpu}"
        ptc_cpu = _parse_table(ROOT / "cpu" / "PT_C_20.txt")
        ptc_gpu = _parse_table(ROOT / "gpu" / "PT_C_20.txt")
        assert ptc_cpu == ptc_gpu, f"PT_C: cpu={ptc_cpu} gpu={ptc_gpu}"
        assert [row[0] for row in ptc_cpu] == ["1"], f"unexpected blocks: {ptc_cpu}"
        assert sum(int(row[1]) for row in ptc_cpu) == n_cpu, (
            f"scenario lost its beyond-end event: PT_C={ptc_cpu}, PT_S rows={n_cpu}"
        )

        # FlowData: full tables equal, incl. the hour-25 row counting A2
        for lane in (1, 2):
            fc = _parse_table(ROOT / "cpu" / f"FlowData_1_{lane}.txt")
            fg = _parse_table(ROOT / "gpu" / f"FlowData_1_{lane}.txt")
            assert fc == fg, f"FlowData lane {lane} differs"
        assert len(fc) == 25 and fc[-1][1] == "1", f"A2 flow row missing: {fc[-1]}"
    finally:
        _clean()


def test_fix2_vehicle_path_threads_end_arrival():
    # The per-vehicle streaming path (used when PT_V needs Vehicle objects) must
    # thread the same A2 boundary: the run's last window gets A2 as its
    # next_arrival, and end_tail carries A2's arrival + single-vehicle
    # extraction row for the flow count.
    tail = {}
    wins = list(
        gpu_runner._traffic_windows(
            _const_traffic(),
            _const_bridge(),
            1,
            None,
            1,
            10**9,
            True,  # want_vehicles -> per-vehicle path
            None,
            tail,
        )
    )
    assert [w[6] for w in wins] == [False] * (len(wins) - 1) + [True]
    next_arrival = wins[-1][5]
    a2 = 73 * GAP2  # lane 2's first arrival past end_time
    assert next_arrival == pytest.approx(a2) and next_arrival > 86400.0
    assert tail["arrival"] == next_arrival
    assert len(tail["extracted"][0]) == 1  # exactly one flow row (A2 itself)
    assert float(tail["extracted"][0][0]) == next_arrival
