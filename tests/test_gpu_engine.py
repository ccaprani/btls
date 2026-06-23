"""
Validate the experimental GPU engine (engine="cuda") against the CPU engine.

The GPU engine computes per-effect global/block maxima via per-vehicle
superposition. It must agree with the C++ "cpu" engine's load-effect maxima on
the same recorded traffic (statistical tolerance — the GPU path samples on its
own uniform grid and reorders the summation, so it is not bit-exact by design).

Covers discrete ILs, all built-in IL ids, and influence surfaces.
Skipped when torch/CUDA is unavailable.
"""

import numpy as np
import pybtls as pb
import pytest
from pathlib import Path
from utils import remove_folder
from pybtls.gpu import is_available

pytestmark = pytest.mark.skipif(not is_available(), reason="requires PyTorch + CUDA")

TRAFFIC = Path(__file__).parent / "test_data/test_traffic_file.txt"
GARAGE = Path(__file__).parent / "test_data/garage.txt"
TIME_STEP = 0.1
ROOT = Path(__file__).parent / "temp_gpu_engine"


def _loader():
    loader = pb.TrafficLoader(no_lane=4)
    loader.add_traffic(traffic=TRAFFIC, traffic_format=4,
                       use_average_speed=False, use_const_speed=False)
    return loader


def _compare(bridge_factory, n_eff, time_step=TIME_STEP):
    """Return (cpu_global_max[n_eff], gpu_global_max[n_eff]) on the same traffic."""
    remove_folder(ROOT)
    cfg = pb.OutputConfig()
    cfg.set_event_output(write_time_history=True)
    sim = pb.Simulation(output_dir=ROOT)
    sim.add_sim(bridge=bridge_factory(), traffic=_loader(), output_config=cfg,
                time_step=time_step, min_gvw=0, tag="cpu", engine="cpu")
    sim.run(no_core=1)
    th = next(iter(sim.get_output()["cpu"].read_data("time_history").values()))
    cpu_max = np.array([th[f"Effect {i + 1}"].abs().max() for i in range(n_eff)])

    sim2 = pb.Simulation(output_dir=ROOT)
    sim2.add_sim(bridge=bridge_factory(), traffic=_loader(),
                 time_step=time_step, min_gvw=0, tag="gpu", engine="cuda")
    sim2.run(no_core=1)
    gpu_bm = sim2.get_output()["gpu"].read_data("BM_summary")
    gpu_max = np.zeros(n_eff)
    for stem, df in gpu_bm.items():
        e = int(stem.split("Eff_")[1]) - 1
        valcols = [c for c in df.columns if c != "Block Index"]
        # magnitude of the governing extreme (signed values for hogging ILs)
        gpu_max[e] = df[valcols].abs().max().max()
    remove_folder(ROOT)
    return cpu_max, gpu_max


def test_gpu_engine_matches_cpu_discrete():
    def factory():
        il1 = pb.InfluenceLine(IL_type="discrete")
        il1.set_IL(position=[0.0, 5.0, 10.0, 15.0, 20.0], ordinate=[0.0, 5.0, 10.0, 5.0, 0.0])
        il2 = pb.InfluenceLine(IL_type="discrete")
        il2.set_IL(position=[0.0, 10.0, 20.0], ordinate=[0.0, 8.0, 0.0])
        b = pb.Bridge(length=20.0, no_lane=4)
        b.add_load_effect(inf_line_surf=il1, threshold=0.0)
        b.add_load_effect(inf_line_surf=il2, threshold=0.0)
        return b

    cpu_max, gpu_max = _compare(factory, 2)
    rel = np.abs(gpu_max - cpu_max) / np.maximum(np.abs(cpu_max), 1e-9)
    assert (rel < 0.01).all(), f"cpu={cpu_max}, gpu={gpu_max}, rel={rel}"


@pytest.mark.parametrize("il_id", [1, 2, 3, 4, 5, 6, 7, 8, 9])
def test_gpu_engine_matches_cpu_builtin(il_id):
    def factory():
        il = pb.InfluenceLine(IL_type="built-in")
        il.set_IL(id=il_id, length=20.0)
        b = pb.Bridge(length=20.0, no_lane=4)
        b.add_load_effect(inf_line_surf=il, threshold=0.0)
        return b

    # Shear ILs (ids 3-6) are discontinuous at supports, so the two engines'
    # independent sampling grids capture the peak a few % apart — grid-phase
    # noise that does not vanish with finer ts (statistical-tolerance regime).
    cpu_max, gpu_max = _compare(factory, 1)
    rel = np.abs(gpu_max - cpu_max) / np.maximum(np.abs(cpu_max), 1e-9)
    assert (rel < 0.05).all(), f"id={il_id} cpu={cpu_max}, gpu={gpu_max}, rel={rel}"


def test_gpu_engine_bm_summary_output_alignment():
    # engine="cuda" writes BM_summary files readable via the same read_data path
    # as engine="cpu"; its overall block max matches the cpu per-block max.
    def factory():
        il = pb.InfluenceLine(IL_type="discrete")
        il.set_IL(position=[0.0, 10.0, 20.0], ordinate=[0.0, 10.0, 0.0])
        b = pb.Bridge(length=20.0, no_lane=4)
        b.add_load_effect(inf_line_surf=il, threshold=0.0)
        return b

    remove_folder(ROOT)
    cfg = pb.OutputConfig()
    cfg.set_BM_output(write_summary=True)
    sim = pb.Simulation(output_dir=ROOT)
    sim.add_sim(bridge=factory(), traffic=_loader(), output_config=cfg,
                time_step=TIME_STEP, min_gvw=0, tag="cpu", engine="cpu")
    sim.run(no_core=1)
    sim2 = pb.Simulation(output_dir=ROOT)
    sim2.add_sim(bridge=factory(), traffic=_loader(),
                 time_step=TIME_STEP, min_gvw=0, tag="gpu", engine="cuda")
    sim2.run(no_core=1)

    cpu_bm = sim.get_output()["cpu"].read_data("BM_summary")
    gpu_bm = sim2.get_output()["gpu"].read_data("BM_summary")

    assert set(cpu_bm) == set(gpu_bm), f"BM_summary file keys differ: {set(cpu_bm)} vs {set(gpu_bm)}"
    cdf = next(iter(cpu_bm.values()))
    gdf = next(iter(gpu_bm.values()))
    assert "Block Index" in gdf.columns
    cpu_overall = cdf[[c for c in cdf.columns if c != "Block Index"]].max().max()
    gpu_overall = gdf[[c for c in gdf.columns if c != "Block Index"]].max().max()
    assert abs(gpu_overall - cpu_overall) / cpu_overall < 0.01, f"cpu={cpu_overall}, gpu={gpu_overall}"
    remove_folder(ROOT)


def test_gpu_engine_streamed_equals_single_window():
    # Generated traffic is streamed in RAM-bounded windows so memory is decoupled
    # from the simulated length. Forcing many tiny windows (small per-window
    # vehicle budget) must give bit-identical block maxima to one big window:
    # each window is shifted to a local time origin that is a whole number of
    # days, and day boundaries are multiples of the sample step, so the grid
    # phase is unchanged.
    from pybtls.gpu import runner as _runner

    def gen():
        garage = pb.garage.read_garage_file(garage_path=GARAGE, garage_format=4)
        g = pb.TrafficGenerator(no_lane=4)
        for i in range(1, 5):
            lfc = pb.LaneFlowComposition(lane_index=i, lane_dir=(1 if i <= 2 else 2))
            lfc.assign_lane_data(
                hourly_truck_flow=[60] * 24, hourly_car_flow=[15] * 24,
                hourly_speed_mean=[40 / 3.6 * 10] * 24, hourly_speed_std=[5.0] * 24,
                hourly_truck_composition=[[25.0, 25.0, 25.0, 25.0] for _ in range(24)],
            )
            g.add_lane(
                vehicle_gen=pb.VehicleGenGarage(garage=garage, kernel=[[1.0, 0.08], [1.0, 0.05], [1.0, 0.02]]),
                headway_gen=pb.HeadwayGenFreeflow(), lfc=lfc,
            )
        g.set_start_time(0.0)
        return g

    def bridge():
        il = pb.InfluenceLine(IL_type="discrete")
        il.set_IL(position=[0.0, 10.0, 20.0], ordinate=[0.0, 10.0, 0.0])
        b = pb.Bridge(length=20.0, no_lane=4)
        b.add_load_effect(inf_line_surf=il, threshold=0.0)
        return b

    def bm_values(target):
        remove_folder(ROOT)
        saved = _runner._window_target_vehicles
        _runner._window_target_vehicles = lambda *a, **k: target
        try:
            sim = pb.Simulation(output_dir=ROOT)
            sim.add_sim(bridge=bridge(), traffic=gen(), no_day=6,
                        time_step=TIME_STEP, min_gvw=0, tag="g", engine="cuda", seed=7)
            sim.run(no_core=1)
            df = next(iter(sim.get_output()["g"].read_data("BM_summary").values()))
            return df[[c for c in df.columns if c != "Block Index"]].values
        finally:
            _runner._window_target_vehicles = saved
            remove_folder(ROOT)

    one = bm_values(10 ** 12)      # single window
    many = bm_values(800)          # ~1-day windows over 6 days
    assert one.shape == many.shape and one.shape[0] == 6, f"shapes {one.shape} {many.shape}"
    assert np.array_equal(one, many), f"streamed != single-window: max |d|={np.abs(one - many).max()}"


def test_gpu_engine_matches_cpu_generated_traffic():
    # engine="cuda" materializes a TrafficGenerator stream, then GPU-computes it;
    # with the same seed the generated vehicles match the cpu engine's stream, so
    # the load-effect maxima agree (within grid-sampling tolerance).
    SEED = 20260621

    def gen():
        garage = pb.garage.read_garage_file(garage_path=GARAGE, garage_format=4)
        g = pb.TrafficGenerator(no_lane=4)
        for i in range(1, 5):
            lfc = pb.LaneFlowComposition(lane_index=i, lane_dir=(1 if i <= 2 else 2))
            lfc.assign_lane_data(
                hourly_truck_flow=[80] * 24, hourly_car_flow=[20] * 24,
                hourly_speed_mean=[40 / 3.6 * 10] * 24, hourly_speed_std=[5.0] * 24,
                hourly_truck_composition=[[25.0, 25.0, 25.0, 25.0] for _ in range(24)],
            )
            g.add_lane(
                vehicle_gen=pb.VehicleGenGarage(garage=garage, kernel=[[1.0, 0.08], [1.0, 0.05], [1.0, 0.02]]),
                headway_gen=pb.HeadwayGenFreeflow(), lfc=lfc,
            )
        g.set_start_time(0.0)
        return g

    def bridge():
        il = pb.InfluenceLine(IL_type="discrete")
        il.set_IL(position=[0.0, 10.0, 20.0], ordinate=[0.0, 10.0, 0.0])
        b = pb.Bridge(length=20.0, no_lane=4)
        b.add_load_effect(inf_line_surf=il, threshold=0.0)
        return b

    remove_folder(ROOT)
    cfg = pb.OutputConfig()
    cfg.set_event_output(write_time_history=True)
    sim = pb.Simulation(output_dir=ROOT)
    sim.add_sim(bridge=bridge(), traffic=gen(), no_day=1, output_config=cfg,
                time_step=TIME_STEP, min_gvw=0, tag="cpu", engine="cpu", seed=SEED)
    sim.run(no_core=1)
    th = next(iter(sim.get_output()["cpu"].read_data("time_history").values()))
    cpu_max = th["Effect 1"].abs().max()

    sim2 = pb.Simulation(output_dir=ROOT)
    sim2.add_sim(bridge=bridge(), traffic=gen(), no_day=1,
                 time_step=TIME_STEP, min_gvw=0, tag="gpu", engine="cuda", seed=SEED)
    sim2.run(no_core=1)
    gbm = sim2.get_output()["gpu"].read_data("BM_summary")
    gpu_max = max(
        df[[c for c in df.columns if c != "Block Index"]].max().max()
        for df in gbm.values()
    )

    assert abs(gpu_max - cpu_max) / cpu_max < 0.01, f"cpu={cpu_max}, gpu={gpu_max}"
    remove_folder(ROOT)


def test_gpu_engine_matches_cpu_hogging_il():
    # A negative (hogging) influence line: the governing extreme is the most
    # negative value. The GPU engine must capture it via the fabs-peak rule,
    # not a plain max(E) (which would report ~0 for an all-negative effect).
    def factory():
        il = pb.InfluenceLine(IL_type="discrete")
        il.set_IL(position=[0.0, 10.0, 20.0], ordinate=[0.0, -10.0, 0.0])
        b = pb.Bridge(length=20.0, no_lane=4)
        b.add_load_effect(inf_line_surf=il, threshold=0.0)
        return b

    cpu_max, gpu_max = _compare(factory, 1)
    assert gpu_max[0] > 1.0, f"GPU failed to capture the hogging extreme: {gpu_max}"
    rel = np.abs(gpu_max - cpu_max) / np.maximum(np.abs(cpu_max), 1e-9)
    assert (rel < 0.01).all(), f"cpu={cpu_max}, gpu={gpu_max}, rel={rel}"


def test_gpu_engine_matches_cpu_per_lane_il():
    # A different influence line AND weight per lane (list of ILs + inf_weight):
    # the GPU per-lane path must reproduce the C++ engine's per-lane summation.
    def factory():
        il_a = pb.InfluenceLine(IL_type="discrete")
        il_a.set_IL(position=[0.0, 10.0, 20.0], ordinate=[0.0, 10.0, 0.0])
        il_b = pb.InfluenceLine(IL_type="built-in")
        il_b.set_IL(id=1, length=20.0)
        il_c = pb.InfluenceLine(IL_type="discrete")
        il_c.set_IL(position=[0.0, 10.0, 20.0], ordinate=[0.0, 6.0, 0.0])
        il_d = pb.InfluenceLine(IL_type="built-in")
        il_d.set_IL(id=1, length=20.0)
        b = pb.Bridge(length=20.0, no_lane=4)
        b.add_load_effect(inf_line_surf=[il_a, il_b, il_c, il_d],
                          inf_weight=[1.0, 2.0, 0.5, 1.5], threshold=0.0)
        return b

    cpu_max, gpu_max = _compare(factory, 1)
    rel = np.abs(gpu_max - cpu_max) / np.maximum(np.abs(cpu_max), 1e-9)
    assert (rel < 0.05).all(), f"cpu={cpu_max}, gpu={gpu_max}, rel={rel}"


def test_gpu_engine_matches_cpu_multi_vehicle_event():
    # A deterministic multi-vehicle event: two identical trucks travel side by side
    # (one per lane) and are on the 20 m bridge at the same instant, so the
    # governing load effect is the SUPERPOSITION of both -> twice a single truck's
    # peak, at a sample the C++ engine marks "No. Trucks == 2". The recorded-traffic
    # tests above only hit the multi-vehicle path statistically; this pins it and
    # checks engine="cuda" reproduces the engine="cpu" peak on that event.
    #
    # The trailing vehicle in each stream is a "sentinel": the CPU engine only
    # writes per-sample output up to the last vehicle's arrival, so without one the
    # event's on-bridge tail (where the trucks overlap) is never simulated.
    def truck(lane, t):
        v = pb.Vehicle(2)
        v.set_time(t); v.set_velocity(20.0); v.set_direction(1)
        v.set_axle_weights([100.0, 100.0]); v.set_axle_spacings([4.0])
        v.set_axle_widths([2.0, 2.0]); v.set_trans(1.8); v.set_local_lane(lane)
        return v

    def factory(no_lane):
        il = pb.InfluenceLine(IL_type="discrete")
        il.set_IL(position=[0.0, 10.0, 20.0], ordinate=[0.0, 10.0, 0.0])
        b = pb.Bridge(length=20.0, no_lane=no_lane)
        b.add_load_effect(inf_line_surf=il, threshold=0.0)
        return b

    def run(vehicles, no_lane, engine, tag):
        remove_folder(ROOT)
        ld = pb.TrafficLoader(no_lane=no_lane)
        ld.add_traffic(traffic=vehicles)
        cfg = pb.OutputConfig()
        cfg.set_event_output(write_time_history=True)
        cfg.set_BM_output(write_summary=True)
        sim = pb.Simulation(output_dir=ROOT)
        sim.add_sim(bridge=factory(no_lane), traffic=ld, no_day=1, output_config=cfg,
                    time_step=TIME_STEP, min_gvw=0, tag=tag, engine=engine)
        sim.run(no_core=1)
        return sim.get_output()[tag]

    # one truck (single lane) -> reference peak
    one_th = next(iter(run([truck(1, 50.0), truck(1, 120.0)], 1, "cpu", "cpu")
                       .read_data("time_history").values()))
    one_peak = one_th["Effect 1"].abs().max()

    # two trucks side by side -> a genuine 2-truck event
    th = next(iter(run([truck(1, 50.0), truck(2, 50.0), truck(1, 120.0)], 2, "cpu", "cpu")
                   .read_data("time_history").values()))
    two_peak = th["Effect 1"].abs().max()
    peak_trucks = th.loc[th["Effect 1"].abs().idxmax(), "No. Trucks"]

    gbm = next(iter(run([truck(1, 50.0), truck(2, 50.0), truck(1, 120.0)], 2, "cuda", "gpu")
                    .read_data("BM_summary").values()))
    gpu_peak = gbm[[c for c in gbm.columns if c != "Block Index"]].abs().max().max()
    remove_folder(ROOT)

    # the governing sample genuinely carries both trucks ...
    assert peak_trucks == 2, f"peak is not a 2-truck event (No. Trucks={peak_trucks})"
    # ... their loads superimpose to twice one truck's peak ...
    assert two_peak == pytest.approx(2.0 * one_peak, rel=0.02), f"one={one_peak} two={two_peak}"
    # ... and engine="cuda" reproduces the CPU multi-vehicle peak within grid tolerance
    assert abs(gpu_peak - two_peak) / two_peak < 0.015, f"cpu={two_peak} gpu={gpu_peak}"


def test_gpu_engine_matches_cpu_surface():
    lane_position = [(0.5, 4.0), (4.0, 7.5), (8.5, 12.0), (12.0, 15.5)]
    IS_matrix = [
        [0.0, 0.0, 8.0, 16.0],
        [0.0, 0.0, 0.0, 0.0],
        [10.0, 0.0, 5.0, 0.0],
        [20.0, 0.0, 0.0, 0.0],
    ]

    def factory():
        surf = pb.InfluenceSurface()
        surf.set_IS(IS_matrix, lane_position)
        b = pb.Bridge(length=20.0, no_lane=4)
        b.add_load_effect(inf_line_surf=surf, threshold=0.0)
        return b

    cpu_max, gpu_max = _compare(factory, 1)
    rel = np.abs(gpu_max - cpu_max) / np.maximum(np.abs(cpu_max), 1e-9)
    assert (rel < 0.03).all(), f"surface cpu={cpu_max}, gpu={gpu_max}, rel={rel}"


# ---------------------------------------------------------------------------
# Peaks-over-threshold (POT). The GPU engine rebuilds the C++ event partition
# from each vehicle's on-bridge window, so the event geometry (which events,
# no.-trucks) is bit-aligned; the peak *values/times* carry uniform-grid
# sampling noise (same source as the BM tests). Whether a borderline event's
# noisy peak crosses the threshold therefore flickers, so event counts agree
# only to ~0.1% — we validate the peak-value *distribution* (what EVA uses).
# ---------------------------------------------------------------------------
def _pot_run(threshold, tag, engine):
    cfg = pb.OutputConfig()
    cfg.set_POT_output(write_vehicle=True, write_summary=True,
                       write_counter=True, POT_size_days=1)
    il = pb.InfluenceLine(IL_type="discrete")
    il.set_IL(position=[0.0, 10.0, 20.0], ordinate=[0.0, 10.0, 0.0])
    b = pb.Bridge(length=20.0, no_lane=4)
    b.add_load_effect(inf_line_surf=il, threshold=threshold)
    sim = pb.Simulation(output_dir=ROOT)
    sim.add_sim(bridge=b, traffic=_loader(), output_config=cfg,
                time_step=TIME_STEP, min_gvw=0, tag=tag, engine=engine)
    sim.run(no_core=1)
    return sim.get_output()[tag]


def test_gpu_pot_summary_matches_cpu():
    remove_folder(ROOT)
    cpu = _pot_run(40.0, "cpu", "cpu")
    gpu = _pot_run(40.0, "gpu", "cuda")
    cdf = next(iter(cpu.read_data("POT_summary").values()))
    gdf = next(iter(gpu.read_data("POT_summary").values()))

    # event counts agree to ~0.1% (threshold flicker on light events only)
    assert abs(len(cdf) - len(gdf)) / len(cdf) < 0.01, f"cpu={len(cdf)} gpu={len(gdf)}"

    # the sorted peak-value distribution (the EVA-relevant quantity) matches
    cs = np.sort(cdf["Peak Value"].values)[::-1]
    gs = np.sort(gdf["Peak Value"].values)[::-1]
    n = min(len(cs), len(gs))
    rel = np.abs(cs[:n] - gs[:n]) / np.maximum(np.abs(cs[:n]), 1e-9)
    assert np.median(rel) < 0.01, f"median rel-err {np.median(rel)}"
    assert np.percentile(rel, 95) < 0.03, f"p95 rel-err {np.percentile(rel, 95)}"
    # the governing extreme (top peak) agrees within grid tolerance
    assert abs(cs[0] - gs[0]) / cs[0] < 0.02, f"top peak cpu={cs[0]} gpu={gs[0]}"
    remove_folder(ROOT)


def test_gpu_pot_counter_matches_cpu():
    remove_folder(ROOT)
    cpu = _pot_run(40.0, "cpu", "cpu")
    gpu = _pot_run(40.0, "gpu", "cuda")
    ccount = next(iter(cpu.read_data("POT_counter").values()))
    gcount = next(iter(gpu.read_data("POT_counter").values()))
    # same block grid (header columns + number of blocks)
    assert list(ccount.columns) == list(gcount.columns)
    assert len(ccount) == len(gcount), f"blocks cpu={len(ccount)} gpu={len(gcount)}"
    # total exceedance count per effect agrees to ~0.1%
    ctot = ccount["Effect 1"].sum()
    gtot = gcount["Effect 1"].sum()
    assert abs(ctot - gtot) / ctot < 0.01, f"counter total cpu={ctot} gpu={gtot}"
    remove_folder(ROOT)


def test_gpu_pot_vehicle_output_structure():
    # PT_V is written in the C++ POTManager format: one event-id line, a
    # per-effect summary line, then the member vehicles serialised with the
    # same Vehicle.write() the C++ engine uses. Validate it reads back and has
    # one event block per PT_S row.
    remove_folder(ROOT)
    gpu = _pot_run(200.0, "gpu", "cuda")  # higher threshold -> fewer events
    summary = next(iter(gpu.read_data("POT_summary").values()))
    veh = next(iter(gpu.read_data("POT_vehicle").values()))
    # one PT_V event block per PT_S row, and each block carries its member trucks
    assert veh["Index"].nunique() == len(summary), \
        f"PT_V events={veh['Index'].nunique()} PT_S rows={len(summary)}"
    assert (veh["No. Trucks"] == veh["Trucks"].apply(len)).all(), \
        "PT_V no.-trucks column disagrees with the listed member vehicles"
    remove_folder(ROOT)


@pytest.mark.parametrize("mode,bf", [("vertical", 0.0), ("centrifugal", 0.0), ("braking", 0.3)])
def test_gpu_load_effect_modes_match_cpu(mode, bf):
    # The GPU applies the same per-axle force coefficient as the C++ engine:
    # vertical F=W, centrifugal F=W*v^2/g, braking F=W*|a|/g (or W*braking_factor
    # when a==0). Validate each mode against engine="cpu" on the same traffic.
    def factory():
        il = pb.InfluenceLine(IL_type="discrete")
        il.set_IL(position=[0.0, 10.0, 20.0], ordinate=[0.0, 10.0, 0.0])
        il.set_mode(mode, braking_factor=bf)
        b = pb.Bridge(length=20.0, no_lane=4)
        b.add_load_effect(inf_line_surf=il, threshold=0.0)
        return b

    remove_folder(ROOT)
    cfg = pb.OutputConfig()
    cfg.set_event_output(write_time_history=True)
    sim = pb.Simulation(output_dir=ROOT)
    sim.add_sim(bridge=factory(), traffic=_loader(), output_config=cfg,
                time_step=TIME_STEP, min_gvw=0, tag="cpu", engine="cpu")
    sim.run(no_core=1)
    th = next(iter(sim.get_output()["cpu"].read_data("time_history").values()))
    cpu_peak = th["Effect 1"].abs().max()

    sim2 = pb.Simulation(output_dir=ROOT)
    sim2.add_sim(bridge=factory(), traffic=_loader(),
                 time_step=TIME_STEP, min_gvw=0, tag="gpu", engine="cuda")
    sim2.run(no_core=1)
    gbm = next(iter(sim2.get_output()["gpu"].read_data("BM_summary").values()))
    gpu_peak = gbm[[c for c in gbm.columns if c != "Block Index"]].abs().max().max()
    rel = abs(cpu_peak - gpu_peak) / max(abs(cpu_peak), 1e-9)
    assert rel < 0.015, f"{mode}: cpu={cpu_peak} gpu={gpu_peak} rel={rel}"
    remove_folder(ROOT)


def test_gpu_braking_uses_vehicle_acceleration():
    # When vehicles carry a non-zero acceleration, braking mode uses |a|/g per
    # vehicle (not the fallback). Build two identical trucks with a=-2.0 m/s^2 and
    # check the GPU braking peak == vertical peak * |a|/g.
    def trucks():
        vs = []
        for t in (0.0, 30.0):
            v = pb.Vehicle(2)
            v.set_time(t); v.set_velocity(20.0); v.set_direction(1)
            v.set_axle_weights([100.0, 100.0]); v.set_axle_spacings([4.0])
            v.set_axle_widths([2.0, 2.0]); v.set_trans(1.8); v.set_local_lane(1)
            v.set_acceleration(-2.0)
            vs.append(v)
        return vs

    def bridge(mode):
        il = pb.InfluenceLine(IL_type="discrete")
        il.set_IL(position=[0.0, 10.0, 20.0], ordinate=[0.0, 10.0, 0.0])
        il.set_mode(mode, braking_factor=0.0)
        b = pb.Bridge(length=20.0, no_lane=1)
        b.add_load_effect(inf_line_surf=il, threshold=0.0)
        return b

    def peak(mode):
        remove_folder(ROOT)
        ld = pb.TrafficLoader(no_lane=1)
        ld.add_traffic(traffic=trucks())
        sim = pb.Simulation(output_dir=ROOT)
        sim.add_sim(bridge=bridge(mode), traffic=ld, no_day=1, time_step=TIME_STEP,
                    min_gvw=0, tag="g", engine="cuda")
        sim.run(no_core=1)
        gbm = next(iter(sim.get_output()["g"].read_data("BM_summary").values()))
        return gbm[[c for c in gbm.columns if c != "Block Index"]].abs().max().max()

    pv, pb_ = peak("vertical"), peak("braking")
    remove_folder(ROOT)
    assert pb_ / pv == pytest.approx(2.0 / 9.80665, rel=0.02), f"v={pv} b={pb_}"


def test_gpu_fatigue_rainflow_matches_cpu():
    # The GPU engine reduces each block's E(t) to turning points on the device
    # and feeds them to the same C++ rainflow counter the CPU uses (ASTM
    # E1049-85, residual carried across blocks/windows). The cycle-amplitude
    # histogram must agree with engine="cpu" within grid-sampling tolerance
    # (the two sample E(t) on different grids, shifting cycle amplitudes a little).
    def factory():
        il = pb.InfluenceLine(IL_type="discrete")
        il.set_IL(position=[0.0, 10.0, 20.0], ordinate=[0.0, 10.0, 0.0])
        b = pb.Bridge(length=20.0, no_lane=4)
        b.add_load_effect(inf_line_surf=il, threshold=0.0)
        return b

    def run(engine, tag):
        cfg = pb.OutputConfig()
        cfg.set_fatigue_output(write_rainflow_output=True, rainflow_decimal=0,
                               rainflow_cut_off=0.0)
        sim = pb.Simulation(output_dir=ROOT)
        sim.add_sim(bridge=factory(), traffic=_loader(), output_config=cfg,
                    time_step=TIME_STEP, min_gvw=0, tag=tag, engine=engine)
        sim.run(no_core=1)
        return next(iter(sim.get_output()[tag].read_data("fatigue_rainflow").values()))

    remove_folder(ROOT)
    cdf, gdf = run("cpu", "cpu"), run("cuda", "gpu")
    # total cycle count agrees to ~1%
    ctot, gtot = cdf["No. Cycles"].sum(), gdf["No. Cycles"].sum()
    assert abs(ctot - gtot) / ctot < 0.01, f"total cycles cpu={ctot} gpu={gtot}"

    # the damage-relevant tail (cumulative cycles above each amplitude) agrees
    def cum_above(df, thr):
        return df[df["Amplitude"] >= thr]["No. Cycles"].sum()
    for thr in (100, 200, 400, 800):
        c, g = cum_above(cdf, thr), cum_above(gdf, thr)
        assert abs(c - g) / max(c, 1.0) < 0.05, f"cycles>= {thr}: cpu={c} gpu={g}"
    # governing amplitude within grid tolerance
    cmax, gmax = cdf["Amplitude"].max(), gdf["Amplitude"].max()
    assert abs(cmax - gmax) / cmax < 0.02, f"max amplitude cpu={cmax} gpu={gmax}"
    remove_folder(ROOT)


def test_gpu_pot_event_partition_matches_cpu_definition():
    # A pybtls "event" (the unit both BM and POT record) is a window of constant
    # on-bridge vehicle composition: the C++ engine opens one at every vehicle
    # on/off transition (cpp/src/Bridge.cpp AddNewEvent..EndEvent). The GPU POT
    # path rebuilds that partition from each vehicle's [t_on, t_off) window.
    # Verify it reproduces the C++ engine's OWN per-event stream
    # (write_each_event): the event count matches to ~1% (the residual being
    # sub-ts micro-windows that the C++ time-step grid merges but exact on/off
    # times split), and every C++ event start lands on a reconstructed window
    # boundary (median |delta| ~ 0). Host-side reconstruction only — no GPU math.
    from pybtls.gpu.runner import _collect_vehicles, _il_specs_from_bridge
    from pybtls.gpu.engine import prepare_axles
    from pybtls.gpu import pot as potmod

    def factory():
        il = pb.InfluenceLine(IL_type="discrete")
        il.set_IL(position=[0.0, 10.0, 20.0], ordinate=[0.0, 10.0, 0.0])
        b = pb.Bridge(length=20.0, no_lane=4)
        b.add_load_effect(inf_line_surf=il, threshold=0.0)
        return b

    remove_folder(ROOT)
    cfg = pb.OutputConfig()
    cfg.set_event_output(write_each_event=True)  # write every event it forms
    sim = pb.Simulation(output_dir=ROOT)
    sim.add_sim(bridge=factory(), traffic=_loader(), output_config=cfg,
                time_step=TIME_STEP, min_gvw=0, tag="cpu", engine="cpu")
    sim.run(no_core=1)
    edf = next(iter(sim.get_output()["cpu"].read_data("all_events").values()))
    cpp_start = np.sort(edf["Start Time"].values)
    cpp_n = len(edf)

    b = factory()
    il_specs, _ = _il_specs_from_bridge(b)
    vehicles, _ = _collect_vehicles(_loader(), b, None, None, None)
    from pybtls.lib import libbtls
    extracted = libbtls._extract_axle_data(vehicles, b.no_lane)
    _, _, veh = prepare_axles(extracted, il_specs, b.length, TIME_STEP, 0)
    B, win_count, _, _ = potmod.build_partition(veh["t_on"], veh["t_off"])
    rec_start = np.sort(B[:-1][win_count >= 1])  # event = window with >=1 vehicle
    rec_n = len(rec_start)

    # same event definition -> event counts agree to ~1% (sub-ts merging only)
    assert abs(cpp_n - rec_n) / cpp_n < 0.02, f"cpp={cpp_n} recon={rec_n}"

    # every C++ event start sits on a reconstructed composition-change boundary
    j = np.searchsorted(rec_start, cpp_start).clip(1, len(rec_start) - 1)
    nearest = np.minimum(np.abs(rec_start[j] - cpp_start),
                         np.abs(rec_start[j - 1] - cpp_start))
    assert np.median(nearest) < 1e-3, f"median start delta {np.median(nearest)}"
    assert (nearest <= TIME_STEP + 1e-9).mean() > 0.97, \
        f"only {(nearest <= TIME_STEP).mean():.3f} of C++ starts within one ts"
    remove_folder(ROOT)


def test_gpu_stats_matches_cpu():
    # Flow statistics (SS_C cumulative / SS_S intervals): the GPU reuses the POT
    # per-event governing value (signed largest-|E|, == CEventStatistics's
    # getMaxEffect) and the geometric event partition, so the distribution of
    # per-event maxima must agree with engine="cpu". Event/vehicle/truck counts
    # carry the same ~1% event-partition tolerance as POT (grid sampling merges
    # sub-time-step composition changes); the moment columns match within a few %.
    from pybtls.output.read.E_cumulative_statistics import read_E_CS
    from pybtls.output.read.E_interval_statistics import read_E_IS

    def factory():
        il = pb.InfluenceLine(IL_type="discrete")
        il.set_IL(position=[0.0, 10.0, 20.0], ordinate=[0.0, 10.0, 0.0])
        il2 = pb.InfluenceLine(IL_type="built-in")
        il2.set_IL(id=2, length=20.0)
        b = pb.Bridge(length=20.0, no_lane=4)
        b.add_load_effect(inf_line_surf=il, threshold=0.0)
        b.add_load_effect(inf_line_surf=il2, threshold=0.0)
        return b

    def run(engine, tag):
        cfg = pb.OutputConfig()
        cfg.set_stats_output(write_overall=True, write_intervals=True, interval_size=3600)
        sim = pb.Simulation(output_dir=ROOT)
        sim.add_sim(bridge=factory(), traffic=_loader(), output_config=cfg,
                    time_step=TIME_STEP, min_gvw=0, tag=tag, engine=engine)
        sim.run(no_core=1)
        return ROOT / tag

    remove_folder(ROOT)
    cdir, gdir = run("cpu", "cpu"), run("cuda", "gpu")

    cc, gc = read_E_CS(cdir / "SS_C_20.txt"), read_E_CS(gdir / "SS_C_20.txt")
    # both effects share one event set, so the count columns are equal across rows
    for col in ("No. Events", "No. Vehicles", "No. Trucks"):
        c, g = cc[col].to_numpy(float), gc[col].to_numpy(float)
        assert (np.abs(g - c) / c < 0.015).all(), f"{col}: cpu={c} gpu={g}"
    # the per-event-max distribution (the EVA-relevant quantity) agrees
    for col, tol in (("Max", 0.01), ("Mean", 0.03), ("Std Dev", 0.03),
                     ("Variance", 0.03), ("Skewness", 0.03), ("Kurtosis", 0.03)):
        c, g = cc[col].to_numpy(float), gc[col].to_numpy(float)
        rel = np.abs(g - c) / np.maximum(np.abs(c), 1e-9)
        assert (rel < tol).all(), f"{col}: rel={rel} cpu={c} gpu={g}"

    # SS_S interval files: same interval grid (rows + Time), silent trailing
    # intervals filled identically, and per-interval counts within tolerance
    ci, gi = read_E_IS(cdir / "SS_S_20_Eff_1.txt"), read_E_IS(gdir / "SS_S_20_Eff_1.txt")
    assert len(ci) == len(gi), f"interval rows cpu={len(ci)} gpu={len(gi)}"
    assert np.array_equal(ci["Time"].to_numpy(), gi["Time"].to_numpy())
    cev, gev = ci["No. Events"].to_numpy(float), gi["No. Events"].to_numpy(float)
    # silent/busy split agrees up to a one-interval flicker at the active boundary
    assert ((cev == 0) != (gev == 0)).sum() <= 1, "silent intervals differ by >1"
    busy = (cev > 0) & (gev > 0)  # both populated -> per-interval counts within tol
    assert (np.abs(gev[busy] - cev[busy]) / cev[busy] < 0.05).all(), "interval event counts"
    remove_folder(ROOT)


def test_gpu_surface_fused_kernel_matches_torch():
    # The fused Triton surface kernel (uniform [nx,ny] grid: direct-index
    # two-track bilinear + atomic scatter) must equal the torch _surface_ordinate
    # path bit-for-bit on a uniform grid; a non-uniform grid must fall back to the
    # (exact) torch searchsorted path rather than resample. Validate both give the
    # same block maxima as the forced-torch reference.
    from pybtls.gpu import kernels as _kernels

    lane_position = [(0.5, 4.0), (4.0, 7.5), (8.5, 12.0), (12.0, 15.5)]
    uniform = [[0.0, 0.0, 8.0, 16.0], [0.0, 0.0, 0.0, 0.0],
               [10.0, 0.0, 5.0, 0.0], [20.0, 0.0, 0.0, 0.0]]          # X,Y uniform
    nonuniform = [[0.0, 0.0, 2.0, 16.0], [0.0, 1.0, 0.5, 0.0],
                  [3.0, 0.0, 5.0, 1.0], [20.0, 2.0, 0.0, 0.0]]        # X=[0,3,20] non-uniform

    def bm(IS_matrix, force_torch):
        remove_folder(ROOT)
        saved = _kernels.triton_available
        if force_torch:
            _kernels.triton_available = lambda: False
        try:
            surf = pb.InfluenceSurface()
            surf.set_IS(IS_matrix, lane_position)
            b = pb.Bridge(length=20.0, no_lane=4)
            b.add_load_effect(inf_line_surf=surf, threshold=0.0)
            sim = pb.Simulation(output_dir=ROOT)
            sim.add_sim(bridge=b, traffic=_loader(), time_step=TIME_STEP,
                        min_gvw=0, tag="s", engine="cuda")
            sim.run(no_core=1)
            df = next(iter(sim.get_output()["s"].read_data("BM_summary").values()))
            return df[[c for c in df.columns if c != "Block Index"]].to_numpy()
        finally:
            _kernels.triton_available = saved
            remove_folder(ROOT)

    # uniform grid: fused kernel is an exact pass-through -> bit-identical to torch
    assert np.array_equal(bm(uniform, False), bm(uniform, True)), \
        "fused surface kernel != torch path on a uniform grid"
    # non-uniform grid: default run falls back to torch -> identical to forced torch
    assert np.array_equal(bm(nonuniform, False), bm(nonuniform, True)), \
        "non-uniform surface did not fall back to the torch path"


def test_gpu_stats_streamed_equals_single_window():
    # Stats over a streamed (RAM-bounded) run must equal a single-window run: the
    # per-event power sums are additive and the day-aligned windows never split a
    # vehicle, so SS_C is bit-identical regardless of window size (this is what
    # makes a 1000-year stats run use the same memory as a 1-year one).
    from pybtls.gpu import runner as _runner
    from pybtls.output.read.E_cumulative_statistics import read_E_CS

    def gen():
        garage = pb.garage.read_garage_file(garage_path=GARAGE, garage_format=4)
        g = pb.TrafficGenerator(no_lane=4)
        for i in range(1, 5):
            lfc = pb.LaneFlowComposition(lane_index=i, lane_dir=(1 if i <= 2 else 2))
            lfc.assign_lane_data(
                hourly_truck_flow=[60] * 24, hourly_car_flow=[30] * 24,
                hourly_speed_mean=[40 / 3.6 * 10] * 24, hourly_speed_std=[5.0] * 24,
                hourly_truck_composition=[[25.0, 25.0, 25.0, 25.0] for _ in range(24)])
            g.add_lane(vehicle_gen=pb.VehicleGenGarage(garage=garage, kernel=[[1.0, 0.08], [1.0, 0.05], [1.0, 0.02]]),
                       headway_gen=pb.HeadwayGenFreeflow(), lfc=lfc)
        g.set_start_time(0.0)
        return g

    def bridge():
        il = pb.InfluenceLine(IL_type="discrete")
        il.set_IL(position=[0.0, 10.0, 20.0], ordinate=[0.0, 10.0, 0.0])
        b = pb.Bridge(length=20.0, no_lane=4)
        b.add_load_effect(inf_line_surf=il, threshold=0.0)
        return b

    def stats_df(target):
        remove_folder(ROOT)
        saved = _runner._window_target_vehicles
        _runner._window_target_vehicles = lambda *a, **k: target
        try:
            cfg = pb.OutputConfig()
            cfg.set_stats_output(write_overall=True, interval_size=3600)
            sim = pb.Simulation(output_dir=ROOT)
            sim.add_sim(bridge=bridge(), traffic=gen(), no_day=6, output_config=cfg,
                        time_step=TIME_STEP, min_gvw=0, tag="s", engine="cuda", seed=11)
            sim.run(no_core=1)
            return read_E_CS(ROOT / "s" / "SS_C_20.txt")
        finally:
            _runner._window_target_vehicles = saved
            remove_folder(ROOT)

    one = stats_df(10 ** 12)     # single window
    many = stats_df(800)         # ~1-day windows over 6 days
    for col in ("No. Events", "No. Vehicles", "No. Trucks", "Min", "Max", "Mean",
                "Std Dev", "Variance", "Skewness", "Kurtosis"):
        assert np.array_equal(one[col].to_numpy(), many[col].to_numpy()), \
            f"streamed != single-window for {col}: {one[col].values} vs {many[col].values}"


def test_gpu_flow_stats_matches_cpu():
    # Flow statistics (FlowData_{dir}_{lane}.txt) are pure per-hour/per-lane vehicle
    # counts by class — no load effect, no grid sampling — so on recorded traffic
    # (identical stream) the GPU files must be byte-for-byte identical to engine=cpu,
    # including the class histogram (the GPU extracts each vehicle's classifier bin
    # in C++ and reproduces the C++ FlowData layout).
    def factory():
        il = pb.InfluenceLine(IL_type="discrete")
        il.set_IL(position=[0.0, 10.0, 20.0], ordinate=[0.0, 10.0, 0.0])
        b = pb.Bridge(length=20.0, no_lane=4)
        b.add_load_effect(inf_line_surf=il, threshold=0.0)
        return b

    def run(engine, tag):
        cfg = pb.OutputConfig()
        cfg.set_stats_output(write_flow_stats=True)
        sim = pb.Simulation(output_dir=ROOT)
        sim.add_sim(bridge=factory(), traffic=_loader(), output_config=cfg,
                    time_step=TIME_STEP, min_gvw=0, tag=tag, engine=engine)
        sim.run(no_core=1)
        return ROOT / tag

    remove_folder(ROOT)
    cdir, gdir = run("cpu", "cpu"), run("cuda", "gpu")
    cfiles = sorted(p.name for p in cdir.glob("FlowData*.txt"))
    gfiles = sorted(p.name for p in gdir.glob("FlowData*.txt"))
    assert cfiles == gfiles and cfiles, f"FlowData file set differs: {cfiles} vs {gfiles}"
    for name in cfiles:
        cpu_txt = (cdir / name).read_text()
        gpu_txt = (gdir / name).read_text()
        assert cpu_txt == gpu_txt, f"{name} differs between cpu and cuda flow stats"
    remove_folder(ROOT)


def test_gpu_engine_op_preflight():
    # The engine probes each non-CUDA backend for the ops it needs (searchsorted,
    # index_add, scatter_reduce amax) so a gap (esp. on Apple MPS) surfaces as a
    # clear error, not a cryptic mid-compute crash. Validate: no false positive
    # on a supported device, and a missing op is detected + reported.
    import torch
    from pybtls.gpu import engine as E
    from pybtls.gpu import GpuEngineError

    assert E._missing_ops(torch, torch.device("cuda")) == []
    E._require_ops(torch, torch.device("cuda"), "cuda")  # supported -> no raise

    E._OP_SUPPORT_CACHE.clear()
    orig = torch.searchsorted
    torch.searchsorted = lambda *a, **k: (_ for _ in ()).throw(RuntimeError("missing"))
    try:
        assert "searchsorted" in E._missing_ops(torch, torch.device("cpu"))
    finally:
        torch.searchsorted = orig
        E._OP_SUPPORT_CACHE.clear()
