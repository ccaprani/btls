"""
The simulation window and the event partition, on both engines.

The run is the set of vehicles arriving in [0, end_time]. Each is counted
once (vehicle file, flow statistics) and, if it loads the bridge, crossed to
completion: the bridge is run on until it empties, whether the traffic is
generated, a recorded stream cut by ``no_day``, or a recorded stream at the
end of its file, and an event that starts after end_time is credited to the
window's last block whatever the state of the write buffer. The first arrival
beyond end_time is neither simulated nor counted. An event ends only when the
set of vehicles ON the bridge changes: a vehicle at or below ``min_gvw`` never
joins the bridge, so its arrival does not cut the running event.

The cross-engine cases drive ``pybtls.gpu.runner.run(..., device="cpu")``
(the torch CPU backend) against ``engine="cpu"``, so they need PyTorch but no
CUDA.
"""

import os
import shutil
from pathlib import Path

import pytest

import pybtls as pb
from pybtls.gpu import runner as gpu_runner
from utils import read_records, run_loader_cpu, run_loader_torch

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
        "cross-engine tests would be skipped."
    )

requires_torch = pytest.mark.skipif(
    not HAS_TORCH, reason="requires PyTorch (uses the torch-CPU backend)"
)

ROOT = Path(__file__).parent / "temp_event_boundaries"


def _clean():
    shutil.rmtree(ROOT, ignore_errors=True)


def _config(**buffer):
    cfg = pb.OutputConfig()
    cfg.set_stats_output(
        write_overall=True, write_intervals=True, write_flow_stats=True, **buffer
    )
    cfg.set_POT_output(write_summary=True, write_counter=True, **buffer)
    cfg.set_BM_output(write_summary=True, **buffer)
    return cfg


# --- events starting after the end of the window --------------------------


def test_events_past_the_end_do_not_depend_on_the_write_buffer(tmp_path):
    # 100 kN and 200 kN trucks arrive 5 s and 1 s before the end of the day;
    # the 200 kN truck's own 1-truck event starts after 86400 s. With
    # buffer_size=1 the day's block had already been flushed when that event
    # arrived, which used to drop it.
    rows = [(0.0, 1.0), (86395.0, 100.0), (86399.0, 200.0), (86430.0, 1.0)]
    out = {
        size: run_loader_cpu(
            tmp_path, f"buffer-{size}", rows, _config(buffer_size=size)
        )
        for size in (1, 10000)
    }
    for key in (
        "BM_summary",
        "POT_counter",
        "E_cumulative_statistics",
        "E_interval_statistics",
    ):
        assert read_records(out[1], key) == read_records(out[10000], key), key

    bm = out[1].read_data("BM_summary")["BM_S_20_Eff_1"]
    assert bm["Block Index"].tolist() == [1]
    assert bm["1-Truck Event"].tolist() == [200.0]  # the beyond-end event
    assert bm["2-Truck Event"].tolist() == [300.0]
    counter = out[1].read_data("POT_counter")["PT_C_20"]
    assert counter["Block"].tolist() == [1]
    assert counter["Effect 1"].tolist() == [3]
    intervals = out[1].read_data("E_interval_statistics")["SS_S_20_Eff_1"]
    assert intervals["Index"].tolist() == list(range(1, 25))
    assert int(intervals["No. Events"].sum()) == 3


# --- the first arrival beyond the end, and the end of a recorded file --------


@requires_torch
def test_recorded_traffic_cut_by_no_day_keeps_the_last_truck(tmp_path):
    # the 100 kN truck arrives 10 s before the end of the simulated day and
    # crosses to completion; the car at 86420 s arrives beyond the end and is
    # not part of the run (neither simulated nor counted in FlowData)
    rows = [(0.0, 1.0), (86390.0, 100.0), (86420.0, 1.0)]
    cpu = run_loader_cpu(tmp_path, "cpu", rows, _config())
    gpu = run_loader_torch(tmp_path, "gpu", rows, _config())
    for key in (
        "BM_summary",
        "POT_counter",
        "E_cumulative_statistics",
        "traffic_statistics",
    ):
        assert read_records(cpu, key) == read_records(gpu, key), key
    assert cpu.read_data("BM_summary")["BM_S_20_Eff_1"]["1-Truck Event"].tolist() == [
        100.0
    ]
    assert len(gpu.read_data("POT_summary")["PT_S_20_Eff_1"]) == 1
    assert len(cpu.read_data("traffic_statistics")["FlowData_1_1"]) == 24


@requires_torch
def test_recorded_traffic_that_runs_out_is_run_to_completion(tmp_path):
    # end of file before the end of the day: the bridge is run on until it
    # empties (as the C++ program always did), so the last truck's crossing is
    # recorded on both engines. The first two trucks form three events
    # ([10, 20), [20, 31) and [31, 41) s), the third truck a fourth.
    rows = [(10.0, 100.0), (20.0, 100.0), (100.0, 100.0)]
    cpu = run_loader_cpu(tmp_path, "cpu", rows, _config(), min_gvw=0)
    gpu = run_loader_torch(tmp_path, "gpu", rows, _config(), min_gvw=0)
    n_cpu = int(cpu.read_data("E_cumulative_statistics")["SS_C_20"]["No. Events"][0])
    n_gpu = int(gpu.read_data("E_cumulative_statistics")["SS_C_20"]["No. Events"][0])
    assert n_cpu == 4 and n_gpu == 4
    assert len(gpu.read_data("POT_summary")["PT_S_20_Eff_1"]) == 4


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
    """No. Events (per effect) from a 3-truck loader run whose file ends with
    the last truck (t=100): both engines run the bridge on until it empties, so
    that crossing is recorded too."""
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


@requires_torch
def test_recorded_traffic_end_of_file_matches_cpu():
    cpu_n = _loader_events("cpu", "cpu")
    gpu_n = _loader_events("cuda", "gpu")
    # three non-overlapping crossings (20 m/s), the last one at the end of file
    assert cpu_n == 3, f"CPU baseline unexpected: {cpu_n}"
    assert gpu_n == cpu_n, f"loader end-of-run mismatch: cpu={cpu_n} gpu={gpu_n}"


# Deterministic constant-headway scenario engineered so the last composition
# event STARTS after end_time (86400 s): the bridge is run on until it empties,
# so the CPU records that event (credited to the last block) and the GPU must
# too:
#   lane 1: arrivals k*GAP1 -> last in-run arrival 86398.2 s, off at 86400.6 s
#   lane 2: arrivals k*GAP2 -> last in-run arrival 86399.0 s, off at 86401.4 s
# Events near the end: [86398.2, 86399.0), [86399.0, 86400.6) and
# [86400.6, 86401.4) — the last one starts AFTER end_time. The first arrival
# past end_time (73*GAP2 = 87598.99 s, lane 2) is not part of the run: neither
# simulated nor counted. Constant speed 10 m/s and identical 4 m trucks keep
# every composition window >= 0.2 s (>= 2 sample steps), so no sub-time-step
# grid flicker: counts must match the CPU exactly.
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


@requires_torch
def test_generated_traffic_end_of_run_matches_cpu():
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
        # starting after end_time was recorded. It does not get a block of
        # its own: CPOTManager::Update credits an event starting past the end
        # of the simulated window to the final block, so this one-day run
        # emits exactly one counter row rather than a spurious second one.
        n_cpu = sum(1 for _ in open(ROOT / "cpu" / "PT_S_20_Eff_1.txt"))
        n_gpu = sum(1 for _ in open(ROOT / "gpu" / "PT_S_20_Eff_1.txt"))
        assert n_cpu == n_gpu, f"PT_S rows: cpu={n_cpu} gpu={n_gpu}"
        ptc_cpu = _parse_table(ROOT / "cpu" / "PT_C_20.txt")
        ptc_gpu = _parse_table(ROOT / "gpu" / "PT_C_20.txt")
        assert ptc_cpu == ptc_gpu, f"PT_C: cpu={ptc_cpu} gpu={ptc_gpu}"
        assert [row[0] for row in ptc_cpu] == ["1"], f"unexpected blocks: {ptc_cpu}"
        assert (
            sum(int(row[1]) for row in ptc_cpu) == n_cpu
        ), f"scenario lost its beyond-end event: PT_C={ptc_cpu}, PT_S rows={n_cpu}"

        # FlowData: full tables equal; the arrival past end_time is not
        # counted, so one day is exactly 24 rows
        for lane in (1, 2):
            fc = _parse_table(ROOT / "cpu" / f"FlowData_1_{lane}.txt")
            fg = _parse_table(ROOT / "gpu" / f"FlowData_1_{lane}.txt")
            assert fc == fg, f"FlowData lane {lane} differs"
        assert len(fc) == 24, f"beyond-end arrival counted: {len(fc)} rows"
    finally:
        _clean()


# --- what ends an event ------------------------------------------------------


@requires_torch
def test_light_arrivals_do_not_end_events(tmp_path):
    # one 100 kN truck; three 1 kN cars arrive while it is on the bridge and
    # are kept out of the load calculation, so the set of vehicles ON the
    # bridge never changes during the crossing: one event, one POT peak, on
    # both engines (the CPU loop used to cut the event at every arrival)
    rows = [(0.0, 100.0), (5.0, 1.0), (10.0, 1.0), (30.0, 1.0)]
    cpu = run_loader_cpu(tmp_path, "cpu", rows, _config())
    gpu = run_loader_torch(tmp_path, "gpu", rows, _config())
    for key in ("BM_summary", "POT_counter", "E_cumulative_statistics"):
        assert read_records(cpu, key) == read_records(gpu, key), key
    stats = cpu.read_data("E_cumulative_statistics")["SS_C_20"]
    assert int(stats["No. Events"][0]) == 1
    assert len(gpu.read_data("POT_summary")["PT_S_20_Eff_1"]) == 1
    # the cars are still traffic: counted in the flow statistics
    flow = cpu.read_data("traffic_statistics")["FlowData_1_1"]
    assert int(flow["No. Cars"].iloc[0]) == 3
