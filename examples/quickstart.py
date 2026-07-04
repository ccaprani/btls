"""
Quickstart: the smallest complete PyBTLS run.

Generates one day of traffic on a single lane and passes it over a
20 m bridge carrying one load effect (a built-in influence line), then
reads back the resulting block-maximum (i.e. worst value of the day)
load effect.

Output is written to an "output" subfolder next to this script, the
same convention as case1.py-case4.py (the folder is not deleted
automatically).

**Notice**: Due to Python multiprocessing, it is essential to define
the simulation in a function.
"""

import pybtls as pb
from pathlib import Path


def main():
    # A LaneFlowComposition (LFC) is the per-lane traffic "recipe" (hourly
    # flow/speed/composition); it doesn't generate vehicles itself, the
    # vehicle_gen/headway_gen below do that.
    lfc = pb.LaneFlowComposition(lane_index=1, lane_dir=1)
    lfc.assign_lane_data(
        hourly_truck_flow=[100] * 24,
        hourly_car_flow=[0] * 24,
        hourly_speed_mean=[80 / 3.6 * 10] * 24,  # dm/s: (km/h / 3.6) * 10
        hourly_speed_std=[10 / 3.6 * 10] * 24,  # dm/s
        hourly_truck_composition=[[23, 2.8, 31.7, 42.5]]
        * 24,  # % by no. axles (2/3/4/5+), needed by VehicleGenGrave
    )

    # VehicleGenGrave draws vehicles from a built-in traffic-site model;
    # HeadwayGenFreeflow spaces them out with a Poisson arrival process.
    vehicle_gen = pb.VehicleGenGrave(traffic_site="Auxerre")
    headway_gen = pb.HeadwayGenFreeflow()

    traffic_gen = pb.TrafficGenerator(no_lane=1)
    traffic_gen.add_lane(vehicle_gen=vehicle_gen, headway_gen=headway_gen, lfc=lfc)

    # One load effect: built-in id=1 is the mid-span bending moment of a
    # simply supported beam spanning the bridge length.
    il = pb.InfluenceLine("built-in")
    il.set_IL(id=1, length=20.0)
    bridge = pb.Bridge(length=20.0, no_lane=1)
    bridge.add_load_effect(inf_line_surf=il, threshold=0.0)

    # OutputConfig toggles which result files get written; here we only ask
    # for block maxima (BM), the largest load effect per block (1 day here).
    output_config = pb.OutputConfig()
    output_config.set_BM_output(write_summary=True)

    sim_task = pb.Simulation(Path(__file__).parent / "output")
    sim_task.add_sim(
        bridge=bridge,
        traffic=traffic_gen,
        no_day=1,
        output_config=output_config,
        tag="Quickstart",
    )
    sim_task.run()

    # Read the block-maxima back as a DataFrame and print the day's result.
    bm_summary = sim_task.get_output()["Quickstart"].read_data("BM_summary")
    df = next(iter(bm_summary.values()))
    print(df)

    value_cols = [c for c in df.columns if c != "Block Index"]
    print(f"Day-1 block maximum load effect: {df[value_cols].iloc[0, 0]:.2f}")


if __name__ == "__main__":
    main()
