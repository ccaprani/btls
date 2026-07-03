"""
This demo splits one simulation into independent day-chunks and runs
them in parallel across cores, then reads the merged result back as if
it had been run in a single piece.

The bridge:
    - Has two 3.5m-width lanes.
    - Has a length of 30m.
    - Has one load effect (mid-span bending moment, built-in IL id=1).

The traffic flow:
    - Vehicles are generated from the Grave model, one lane each direction.
    - Headways are in freeflow condition (Poisson arrival model).

``add_sim(..., no_chunk=4, seed=42)`` splits the 4 simulated days into 4
one-day chunks; chunk i runs with RNG seed 42+i. ``run(no_core=4)`` runs
all 4 chunks in parallel. ``get_output()`` then returns a single merged
view - the same interface as an unchunked run - while the per-chunk
results also remain on disk for inspection. See docs/source/parallel.rst
for the merge rules and reproducibility guarantees, and
docs/source/notebooks/parallel_sim_example.ipynb for the notebook this
script mirrors.

Output is written to an "output" subfolder next to this script, the
same convention as case1.py-case4.py (the folder is not deleted
automatically).

**Notice**: Due to Python multiprocessing, it is essential to define
the simulation in a function.
"""

import pybtls as pb
from pathlib import Path


def main():
    il = pb.InfluenceLine("built-in")
    il.set_IL(id=1, length=30.0)
    bridge = pb.Bridge(length=30.0, no_lane=2)
    bridge.add_load_effect(inf_line_surf=il, threshold=0.0)

    traffic_gen = pb.TrafficGenerator(no_lane=2)
    for lane_index, lane_dir in [(1, 1), (2, 2)]:
        lfc = pb.LaneFlowComposition(lane_index=lane_index, lane_dir=lane_dir)
        lfc.assign_lane_data(
            hourly_truck_flow=[120] * 24,
            hourly_car_flow=[20] * 24,
            hourly_speed_mean=[80 / 3.6 * 10] * 24,  # dm/s
            hourly_speed_std=[10 / 3.6 * 10] * 24,  # dm/s
            hourly_truck_composition=[[23, 2.8, 31.7, 42.5]] * 24,  # VehicleGenGrave needs this
        )
        traffic_gen.add_lane(
            vehicle_gen=pb.VehicleGenGrave(traffic_site="Auxerre"),
            headway_gen=pb.HeadwayGenFreeflow(),
            lfc=lfc,
        )

    output_config = pb.OutputConfig()
    output_config.set_BM_output(write_summary=True)

    sim_task = pb.Simulation(Path(__file__).parent / "output")
    sim_task.add_sim(
        bridge=bridge,
        traffic=traffic_gen,
        no_day=4,
        output_config=output_config,
        tag="Case6-Parallel",
        seed=42,
        no_chunk=4,
    )
    sim_task.run(no_core=4)

    output = sim_task.get_output()["Case6-Parallel"]
    print("available outputs:", output.get_summary())
    print(f"chunks: {output.no_chunk} | master seed: {output.master_seed}")

    # A single merged view over all 4 chunks - reads exactly like an
    # unchunked run.
    bm_summary = output.read_data("BM_summary")
    for name, df in bm_summary.items():
        print(name)
        print(df)  # 4 daily block-maxima rows, merged across all chunks

    # The per-chunk results also remain on disk (under chunk_000..chunk_003)
    # and can be read individually, e.g. for convergence checks.
    first_chunk_bm = output.read_chunk_data("BM_summary")[0]
    for name, df in first_chunk_bm.items():
        print(name)
        print(df)  # 1 daily block from the first chunk only


if __name__ == "__main__":
    main()
