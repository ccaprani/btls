"""
This demo is for the case generating a traffic flow by multiple gen models, which is an add-on to the case2. 

There are four lanes of traffic: 
    - Lane 1: Vehicles are generated from Grave model, and headways are in NHM condition.
    - Lane 2: Vehicles are generated from Nominal model, and headways are in Congested condition. 
    - Lane 3: Vehicles are generated from Garage model, and headways are in Constant condition. 
    - Lane 4: Vehicles are generated from Garage model, and headways are in Freeflow condition (Poisson arrival model). 

**Notice**: Due to Python multiprocessing, it is essential to define the simulation in a function. 
"""

import pybtls as pb
from pathlib import Path


def main():
    # set lane flow compositions
    lfc_1 = pb.LaneFlowComposition(lane_index=1, lane_dir=1)
    lfc_1.assign_lane_data(
        hourly_truck_flow=[100] * 24,
        hourly_car_flow=[0] * 24,
        hourly_speed_mean=[80 / 3.6 * 10] * 24,
        hourly_speed_std=[10.0] * 24,
        hourly_truck_composition=[
            [25.0, 25.0, 25.0, 25.0] for _ in range(24)
        ],  # only the Grave model requires this info.
    )

    lfc_2 = pb.LaneFlowComposition(lane_index=2, lane_dir=1)
    lfc_2.assign_lane_data(hourly_truck_flow=[100] * 24, hourly_car_flow=[0] * 24)

    lfc_3 = pb.LaneFlowComposition(lane_index=3, lane_dir=2)
    lfc_3.assign_lane_data(hourly_truck_flow=[100] * 24, hourly_car_flow=[0] * 24)

    lfc_4 = pb.LaneFlowComposition(lane_index=4, lane_dir=2)
    lfc_4.assign_lane_data(
        hourly_truck_flow=[100] * 24,
        hourly_car_flow=[0] * 24,
        hourly_speed_mean=[80 / 3.6 * 10] * 24,
        hourly_speed_std=[10.0] * 24,
    )

    # set the Garage vehicle generator
    kernel = [[1.0, 0.08], [1.0, 0.05], [1.0, 0.02]]
    vehicle_gen_Garage = pb.VehicleGenGarage(
        garage=Path(__file__).parent / "garage.txt", kernel=kernel, garage_format=4
    )

    # set the Nominal vehicle generator
    vehicle = pb.Vehicle(no_axles=3)
    vehicle.set_axle_weights([100.0, 100.0, 100.0])
    vehicle.set_axle_spacings([3.0, 7.0, 0.0])
    vehicle.set_axle_widths([2.0, 2.0, 2.0])

    vehicle_gen_Nominal = pb.VehicleGenNominal(
        nominal_vehicle=vehicle, COV_list=[1.0, 0.1]
    )

    # set the Grave vehicle generator
    vehicle_gen_Grave = pb.VehicleGenGrave(traffic_site="Auxerre")

    # set the headway generators
    headway_gen_NHM = pb.HeadwayGenNHM()

    headway_gen_Congested = pb.HeadwayGenCongested(
        congested_spacing=26.1, congested_speed=36.0, congested_gap_coef_var=0.05
    )

    headway_gen_Constant = pb.HeadwayGenConstant(constant_speed=36.0, constant_gap=5.0)

    headway_gen_Freeflow = pb.HeadwayGenFreeflow()

    # # assemble traffic generator
    traffic_gen = pb.TrafficGenerator(no_lane=4)
    traffic_gen.add_lane(
        vehicle_gen=vehicle_gen_Grave, headway_gen=headway_gen_NHM, lfc=lfc_1
    )
    traffic_gen.add_lane(
        vehicle_gen=vehicle_gen_Nominal, headway_gen=headway_gen_Congested, lfc=lfc_2
    )
    traffic_gen.add_lane(
        vehicle_gen=vehicle_gen_Garage, headway_gen=headway_gen_Constant, lfc=lfc_3
    )
    traffic_gen.add_lane(
        vehicle_gen=vehicle_gen_Garage, headway_gen=headway_gen_Freeflow, lfc=lfc_4
    )
    # traffic_gen.set_start_time(0.0)  # optional, the default start_time is 0.0 unless you want to change it.

    # set output, which will be written to HDD (here we only want the generated traffic)
    output_config = pb.OutputConfig()
    output_config.set_vehicle_file_output(
        write_vehicle_file=True,
        vehicle_file_format=4,
        vehicle_file_name="test_traffic_file.txt",
    )

    # set simulation
    sim_task = pb.Simulation(Path(__file__).parent / "output")
    sim_task.add_sim(
        traffic=traffic_gen,
        no_day=10,
        output_config=output_config,
        # active_lane=[1,2,3,4],  # optional, if not set, all lanes will be active.
        # track_progress=False,  # optional, if True, the progress print will show up.
        tag="Case4",
    )

    # run simulation
    sim_task.run(no_core=1)


if __name__ == "__main__":
    main()
