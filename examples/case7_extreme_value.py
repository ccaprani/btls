"""
This demo is for the case fitting extreme value distributions to simulation output.

The bridge:
    - Has two lanes in the same direction.
    - Has a length of 20m.
    - Has one load effect (built-in IL 1, mid-span bending moment).

The traffic flow:
    - Vehicles are generated from Garage model.
    - Headways are in freeflow condition (Poisson arrival model).

The post-processing:
    - Block-maximum (BM) output with hourly blocks -> GEV fit -> return level.
    - Peak-over-threshold (POT) output -> GPD fit -> return level.

**Notice**: This is a 2-day demo, so the fitted parameters and the extrapolated
100-year characteristic values are for demonstration only. A real extreme value
analysis needs years of simulated traffic and larger blocks (see the BM and POT
tutorial notebooks, which use 100 years with yearly blocks).
"""

import pybtls as pb
import shutil
from pathlib import Path

# The threshold above which the simulation records POT peak events, in kNm.
POT_RECORD_THRESHOLD = 1000.0


def main():
    # set the load effect by using the built-in influence line 1 (mid-span bending)
    load_effect = pb.InfluenceLine("built-in")
    load_effect.set_IL(id=1, length=20.0)

    # set the bridge; the threshold is the POT recording threshold in kNm
    bridge = pb.Bridge(length=20.0, no_lane=2)
    bridge.add_load_effect(inf_line_surf=load_effect, threshold=POT_RECORD_THRESHOLD)

    # set lane flow compositions (trucks only, constant hourly flow)
    lfc_1 = pb.LaneFlowComposition(lane_index=1, lane_dir=1)
    lfc_1.assign_lane_data(
        hourly_truck_flow=[100] * 24,
        hourly_car_flow=[0] * 24,
        hourly_speed_mean=[80 / 3.6 * 10] * 24,  # in dm/s
        hourly_speed_std=[10.0] * 24,  # in dm/s
    )

    lfc_2 = pb.LaneFlowComposition(lane_index=2, lane_dir=1)
    lfc_2.assign_lane_data(
        hourly_truck_flow=[25] * 24,
        hourly_car_flow=[0] * 24,
        hourly_speed_mean=[80 / 3.6 * 10] * 24,
        hourly_speed_std=[10.0] * 24,
    )

    # set vehicle generator
    garage_list = pb.garage.read_garage_file(Path(__file__).parent / "garage.txt", 4)
    kernel = [[1.0, 0.08], [1.0, 0.05], [1.0, 0.02]]
    vehicle_gen = pb.VehicleGenGarage(garage=garage_list, kernel=kernel)

    # set headway generator
    headway_gen = pb.HeadwayGenFreeflow()

    # assemble traffic generator
    traffic_gen = pb.TrafficGenerator(no_lane=2)
    traffic_gen.add_lane(vehicle_gen=vehicle_gen, headway_gen=headway_gen, lfc=lfc_1)
    traffic_gen.add_lane(vehicle_gen=vehicle_gen, headway_gen=headway_gen, lfc=lfc_2)

    # set output: BM summary with hourly blocks (2 days -> 48 block maxima),
    # and POT summary of peaks above the recording threshold
    output_config = pb.OutputConfig()
    output_config.set_BM_output(
        write_summary=True, block_size_days=0, block_size_secs=3600
    )
    output_config.set_POT_output(write_summary=True)

    # set and run a 2-day simulation
    # (remove any previous Case7 output, since pybtls refuses to reuse the dir)
    output_root = Path(__file__).parent / "output"
    shutil.rmtree(output_root / "Case7", ignore_errors=True)
    sim_task = pb.Simulation(output_root)
    sim_task.add_sim(
        bridge=bridge,
        traffic=traffic_gen,
        no_day=2,
        output_config=output_config,
        time_step=0.1,
        min_gvw=35,
        tag="Case7",
    )
    sim_task.run(no_core=1)

    # read the BM and POT summary outputs (load effect 1 on the 20m bridge)
    sim_output = sim_task.get_output()
    bm_data = sim_output["Case7"].read_data("BM_summary")["BM_S_20_Eff_1"]
    pot_data = sim_output["Case7"].read_data("POT_summary")["PT_S_20_Eff_1"]

    # --- GEV fit to hourly block maxima of 1-truck events ---
    block_maxima = bm_data["1-Truck Event"]
    gev_fit = pb.post_processing.fit_gev(block_maxima, block_size_days=1 / 24)
    print("GEV fit to hourly block maxima (1-truck events):")
    print(
        f"  shape xi = {gev_fit.shape:.4f}, "
        f"loc mu = {gev_fit.loc:.0f} kNm, "
        f"scale sigma = {gev_fit.scale:.0f} kNm"
    )
    print(f"  100-year characteristic value: {gev_fit.return_level(100):.0f} kNm")

    # --- GPD fit to peaks over threshold ---
    peaks = pot_data["Peak Value"]
    # average number of threshold exceedances per year (250 simulated days)
    no_years = 2 / pb.post_processing.DAYS_PER_YEAR
    n_rate = (peaks > POT_RECORD_THRESHOLD).sum() / no_years
    gpd_fit = pb.post_processing.fit_gpd(
        peaks, threshold=POT_RECORD_THRESHOLD, n_peaks_per_year=n_rate
    )
    print(f"GPD fit to {gpd_fit.n_exceedances} peaks over {POT_RECORD_THRESHOLD} kNm:")
    print(
        f"  shape xi = {gpd_fit.shape:.4f}, " f"scale sigma = {gpd_fit.scale:.0f} kNm"
    )
    print(f"  100-year characteristic value: {gpd_fit.return_level(100):.0f} kNm")


if __name__ == "__main__":
    main()
