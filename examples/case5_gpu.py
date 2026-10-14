"""
This demo runs the same simulation twice - once on the default C++ "cpu"
engine, once on the experimental GPU engine (``engine="cuda"``) - and
compares their block-maximum load effects.

The two engines sample the continuous load-effect history on different
time grids, so they are not bit-identical by design; see
docs/source/gpu_engine.rst for the full rationale. Using the same seed
gives both engines the same generated-traffic realisation, so their
block maxima are expected to agree to well under 1%.

If no CUDA GPU / PyTorch install is available, the script prints a
message and skips the GPU run (exit code stays 0).

Output is written to an "output" subfolder next to this script, the
same convention as case1.py-case4.py. Re-running the script deletes its
own tag subfolders first, since pybtls refuses to reuse an existing tag
folder.

**Notice**: Due to Python multiprocessing, it is essential to define
the simulation in a function.
"""

import pybtls as pb
from pybtls.gpu import is_available
from pathlib import Path


def build_bridge():
    il = pb.InfluenceLine("built-in")
    il.set_IL(id=1, length=20.0)
    bridge = pb.Bridge(length=20.0, no_lane=1)
    bridge.add_load_effect(inf_line_surf=il, threshold=0.0)
    return bridge


def build_traffic():
    lfc = pb.LaneFlowComposition(lane_index=1, lane_dir=1)
    lfc.assign_lane_data(
        hourly_truck_flow=[200] * 24,
        hourly_car_flow=[0] * 24,
        hourly_speed_mean=[80 / 3.6 * 10] * 24,  # dm/s
        hourly_speed_std=[10 / 3.6 * 10] * 24,  # dm/s
        hourly_truck_composition=[[23, 2.8, 31.7, 42.5]]
        * 24,  # VehicleGenGrave needs this
    )
    traffic_gen = pb.TrafficGenerator(no_lane=1)
    traffic_gen.add_lane(
        vehicle_gen=pb.VehicleGenGrave(traffic_site="Auxerre"),
        headway_gen=pb.HeadwayGenFreeflow(),
        lfc=lfc,
    )
    return traffic_gen


def run_engine(engine, tag, out_dir):
    output_config = pb.OutputConfig()
    output_config.set_BM_output(write_summary=True)

    # (remove any previous output for this tag, since pybtls refuses to reuse the dir)
    sim_task = pb.Simulation(out_dir, overwrite=True)
    sim_task.add_sim(
        bridge=build_bridge(),
        traffic=build_traffic(),
        no_day=2,
        output_config=output_config,
        tag=tag,
        seed=42,  # same seed -> same generated traffic on both engines
        engine=engine,
    )
    # GPU tasks must be run with no_core=1: queuing several with no_core>1
    # would serialise on the one shared device (no speed-up) while each
    # worker replicates its window in host RAM and VRAM (risking OOM) - see
    # "Running in parallel" in docs/source/gpu_engine.rst. A single-sim run
    # already defaults to one core; no_core=1 just makes this explicit.
    sim_task.run(no_core=1)

    bm_summary = sim_task.get_output()[tag].read_data("BM_summary")
    df = next(iter(bm_summary.values()))
    value_cols = [c for c in df.columns if c != "Block Index"]
    return df[value_cols].abs().max().max()


def main():
    out_dir = Path(__file__).parent / "output"

    cpu_max = run_engine("cpu", "Case5-CPU", out_dir)
    print(f"CPU engine block maximum:  {cpu_max:.3f}")

    if not is_available():
        print("No CUDA GPU / PyTorch install detected - skipping the GPU run.")
        return

    gpu_max = run_engine("cuda", "Case5-GPU", out_dir)
    print(f"GPU engine block maximum:  {gpu_max:.3f}")

    rel_diff = abs(gpu_max - cpu_max) / abs(cpu_max)
    print(f"Relative difference: {rel_diff * 100:.3f}% (expect well under 1%)")


if __name__ == "__main__":
    main()
