"""
This demo is for the case generating a traffic flow and passing them over a bridge.

The bridge:
    - Has four 3.5m-width lanes.
    - Has a length of 20m.
    - Has a width of 16m.
    - Has one load effect being considered.

The traffic flow:
    - Vehicles are generated from Garage model.
    - Headways are in freeflow condition (Poisson arrival model).

**Notice**: Due to Python multiprocessing, it is essential to define the simulation in a function.
"""

import pybtls as pb
from pathlib import Path


def main():
    # set the load effect by using a 2D influence surface
    lanes_position = [(0.5, 4.0), (4.0, 7.5), (8.5, 12.0), (12.0, 15.5)]
    IS_matrix = [
        [0.0, 0.0, 4.0, 8.0, 12.0, 16.0],
        [0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
        [10.0, 0.0, 2.5, 5.0, 2.5, 0.0],
        [20.0, 0.0, 0.0, 0.0, 0.0, 0.0],
    ]
    load_effect = pb.InfluenceSurface()
    load_effect.set_IS(IS_matrix, lanes_position)

    # set the bridge
    bridge = pb.Bridge(length=20.0, no_lane=4)
    bridge.add_load_effect(inf_line_surf=load_effect, threshold=0.0)

    # set lane flow compositions (here we just use the same flow for all lanes)
    lfc_1 = pb.LaneFlowComposition(lane_index=1, lane_dir=1)
    lfc_1.assign_lane_data(
        hourly_truck_flow=[100] * 24,
        hourly_car_flow=[0] * 24,
        hourly_speed_mean=[80 / 3.6 * 10] * 24,
        hourly_speed_std=[10.0] * 24,
    )

    lfc_2 = pb.LaneFlowComposition(lane_index=2, lane_dir=1)
    lfc_2.assign_lane_data(
        hourly_truck_flow=[100] * 24,
        hourly_car_flow=[0] * 24,
        hourly_speed_mean=[80 / 3.6 * 10] * 24,
        hourly_speed_std=[10.0] * 24,
    )

    lfc_3 = pb.LaneFlowComposition(lane_index=3, lane_dir=2)
    lfc_3.assign_lane_data(
        hourly_truck_flow=[100] * 24,
        hourly_car_flow=[0] * 24,
        hourly_speed_mean=[80 / 3.6 * 10] * 24,
        hourly_speed_std=[10.0] * 24,
    )

    lfc_4 = pb.LaneFlowComposition(lane_index=4, lane_dir=2)
    lfc_4.assign_lane_data(
        hourly_truck_flow=[100] * 24,
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

    # # assemble traffic generator
    traffic_gen = pb.TrafficGenerator(no_lane=4)
    traffic_gen.add_lane(vehicle_gen=vehicle_gen, headway_gen=headway_gen, lfc=lfc_1)
    traffic_gen.add_lane(vehicle_gen=vehicle_gen, headway_gen=headway_gen, lfc=lfc_2)
    traffic_gen.add_lane(vehicle_gen=vehicle_gen, headway_gen=headway_gen, lfc=lfc_3)
    traffic_gen.add_lane(vehicle_gen=vehicle_gen, headway_gen=headway_gen, lfc=lfc_4)
    # traffic_gen.set_start_time(0.0)  # optional, the default start_time is 0.0 unless you want to change it.

    # set output, which will be written to HDD (here we output all possible results)
    output_config = pb.OutputConfig()
    output_config.set_event_output(write_time_history=True, write_each_event=True)
    output_config.set_vehicle_file_output(
        write_vehicle_file=True,
        vehicle_file_format=4,
        vehicle_file_name="test_traffic_file.txt",
    )
    output_config.set_BM_output(
        write_vehicle=True, write_summary=True, write_mixed=True
    )
    output_config.set_POT_output(
        write_vehicle=True, write_summary=True, write_counter=True
    )
    output_config.set_stats_output(
        write_flow_stats=True, write_overall=True, write_intervals=True
    )
    output_config.set_fatigue_output(
        write_fatigue_event=True, write_rainflow_output=True
    )

    # set simulation
    sim_task = pb.Simulation(Path(__file__).parent / "output")
    sim_task.add_sim(
        bridge=bridge,
        traffic=traffic_gen,
        no_day=10,
        output_config=output_config,
        time_step=0.1,
        min_gvw=35,
        # active_lane=[1,2,3,4],  # optional, if not set, all lanes will be active.
        # track_progress=False,  # optional, if True, the progress print will show up.
        tag="Case2",
    )

    # run simulation
    sim_task.run(no_core=1)


if __name__ == "__main__":
    main()
