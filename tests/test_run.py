import pybtls as pb
import os
import shutil


def main_func():
    # set bridge
    bridge_1_lane_position = [(0.5, 4.0), (4.0, 7.5), (8.5, 12.0), (12.0, 15.5)]
    IS_matrix_1 = [
        [0.0, 0.0, 4.0, 8.0, 12.0, 16.0],
        [0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
        [10.0, 0.0, 2.5, 5.0, 2.5, 0.0],
        [20.0, 0.0, 0.0, 0.0, 0.0, 0.0],
    ]
    inf_surf_1 = pb.InfluenceSurface()
    inf_surf_1.set_IS(IS_matrix_1, bridge_1_lane_position)

    bridge_2_lane_position = [(0.5, 4.0), (4.0, 7.5), (8.5, 12.0), (12.0, 15.5)]
    IS_matrix_2 = [
        [0.0, 0.0, 4.0, 8.0, 12.0, 16.0],
        [0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
        [20.0, 0.0, 5.0, 10.0, 5.0, 0.0],
        [40.0, 0.0, 0.0, 0.0, 0.0, 0.0],
    ]
    inf_surf_2 = pb.InfluenceSurface()
    inf_surf_2.set_IS(IS_matrix_2, bridge_2_lane_position)

    bridge_1 = pb.Bridge(length=20.0, no_lane=4)
    bridge_1.add_load_effect(inf_line_surf=inf_surf_1, threshold=0.0)
    bridge_1.add_load_effect(
        inf_line_surf=inf_surf_1, threshold=0.0
    )  # multi load effects test

    bridge_2 = pb.Bridge(length=40.0, no_lane=4)
    bridge_2.add_load_effect(inf_line_surf=inf_surf_2, threshold=0.0)

    # set lane flow compositions
    lfc_1 = pb.LaneFlowComposition(lane_index=1, lane_dir=1)
    lfc_1.assign_lane_data(
        hourly_truck_flow=[100] * 24,
        hourly_car_flow=[0] * 24,
        hourly_speed_mean=[80 / 3.6 * 10] * 24,
        hourly_speed_std=[10.0] * 24,
        hourly_truck_composition=[[25.0, 25.0, 25.0, 25.0] for _ in range(24)],
    )

    lfc_2 = pb.LaneFlowComposition(lane_index=2, lane_dir=1)
    lfc_2.assign_lane_data(
        hourly_truck_flow=[100] * 24,
        hourly_car_flow=[0] * 24,
        hourly_speed_mean=[80 / 3.6 * 10] * 24,
        hourly_speed_std=[10.0] * 24,
        hourly_truck_composition=[[25.0, 25.0, 25.0, 25.0] for _ in range(24)],
    )

    lfc_3 = pb.LaneFlowComposition(lane_index=3, lane_dir=2)
    lfc_3.assign_lane_data(
        hourly_truck_flow=[100] * 24,
        hourly_car_flow=[0] * 24,
        hourly_speed_mean=[80 / 3.6 * 10] * 24,
        hourly_speed_std=[10.0] * 24,
        hourly_truck_composition=[[25.0, 25.0, 25.0, 25.0] for _ in range(24)],
    )

    lfc_4 = pb.LaneFlowComposition(lane_index=4, lane_dir=2)
    lfc_4.assign_lane_data(
        hourly_truck_flow=[100] * 24,
        hourly_car_flow=[0] * 24,
        hourly_speed_mean=[80 / 3.6 * 10] * 24,
        hourly_speed_std=[10.0] * 24,
        hourly_truck_composition=[[25.0, 25.0, 25.0, 25.0] for _ in range(24)],
    )

    # set vehicle generator
    garage_list = pb.GarageProcessing.load_garage_file("./test_data/garage.txt")
    kernel = [[1.0, 0.08], [1.0, 0.05], [1.0, 0.02]]

    vehicle_gen_1 = pb.VehicleGenGarage(garage=garage_list, kernel=kernel)

    vehicle = pb.Vehicle(no_axle=3)
    vehicle.set_axle_weights([100.0, 100.0, 100.0])
    vehicle.set_axle_spacings([3.0, 7.0])
    vehicle.set_axle_widths([2.0, 2.0, 2.0])

    vehicle_gen_2 = pb.VehicleGenNominal(nominal_vehicle=vehicle, COV_list=[1.0, 0.1])

    vehicle_gen_3 = pb.VehicleGenGrave(traffic_site="Auxerre")

    # set headway generator
    headway_gen_1 = pb.HeadwayGenNHM()

    headway_gen_2 = pb.HeadwayGenConstant(constant_speed=36.0, constant_gap=5.0)

    headway_gen_3 = pb.HeadwayGenCongested(
        congested_spacing=26.1, congested_speed=36.0, congested_gap_coef_var=0.05
    )

    headway_gen_4 = pb.HeadwayGenFreeflow()

    # # assemble traffic generator
    traffic_gen = pb.TrafficGenerator(no_lane=4)
    traffic_gen.add_lane(
        vehicle_gen=vehicle_gen_1, headway_gen=headway_gen_1, lfc=lfc_1
    )
    traffic_gen.add_lane(
        vehicle_gen=vehicle_gen_1, headway_gen=headway_gen_2, lfc=lfc_2
    )
    traffic_gen.add_lane(
        vehicle_gen=vehicle_gen_2, headway_gen=headway_gen_3, lfc=lfc_3
    )
    traffic_gen.add_lane(
        vehicle_gen=vehicle_gen_3, headway_gen=headway_gen_4, lfc=lfc_4
    )
    traffic_gen.set_start_time(0.0)

    # or, use traffic loader
    traffic_loader = pb.TrafficLoader(no_lane=4)
    traffic_loader.add_traffic(
        "./test_data/test_traffic_file.txt",
        file_format="MON",
        use_average_speed=False,
        use_const_speed=False,
        const_speed_value=36.0,
    )

    # set output, which will be written to HDD
    output_config_1 = pb.OutputConfig()
    output_config_1.set_event_output(write_time_history=True, write_each_event=True)
    output_config_1.set_BM_output(
        write_vehicle=True, write_summary=True, write_mixed=True
    )
    output_config_1.set_POT_output(
        write_vehicle=True, write_summary=True, write_counter=True
    )
    output_config_1.set_fatigue_output(
        write_fatigue_event=True, write_rainflow_output=True
    )
    output_config_1.set_stats_output(
        write_flow_stats=True, write_overall=True, write_intervals=True
    )

    output_config_2 = pb.OutputConfig()
    output_config_2.set_vehicle_file_output(
        write_vehicle_file=True,
        vehicle_file_format=4,
        vehicle_file_name="proof_traffic_file.txt",
    )
    output_config_2.set_stats_output(
        write_flow_stats=True, write_overall=True, write_intervals=True
    )

    # set simulation
    sim_task = pb.Simulation()
    sim_task.add_sim(
        bridge=bridge_1,
        traffic_generator=traffic_gen,
        no_day=1,
        output_config=output_config_1,
        time_step=0.1,
        min_gvw=35,
        active_lane=[1, 2],
        track_progress=False,
    )
    sim_task.add_sim(
        bridge=bridge_2,
        traffic_generator=traffic_gen,
        no_day=1,
        output_config=output_config_1,
        time_step=0.1,
        min_gvw=35,
        track_progress=False,
    )
    sim_task.add_sim(
        bridge=bridge_2,
        traffic_generator=traffic_loader,
        output_config=output_config_2,
        time_step=0.1,
        min_gvw=0,
        track_progress=False,
    )
    sim_task.add_sim(bridge=bridge_1, vehicle=vehicle)
    sim_task.add_sim(bridge=bridge_2, vehicle=vehicle, active_lane=[1])
    sim_task.add_sim(bridge=bridge_2, vehicle=vehicle, active_lane=[2])
    sim_task.add_sim(bridge=bridge_2, vehicle=vehicle, active_lane=[3])
    sim_task.add_sim(bridge=bridge_2, vehicle=vehicle, active_lane=[4])

    # run simulation simultaneously
    sim_task.run(no_core=6)


def test_run():
    output_folder_list = [
        "Sim_1",
        "Sim_2",
        "Sim_3",
        "Sim_4",
        "Sim_5",
        "Sim_6",
        "Sim_7",
        "Sim_8",
    ]

    for output_folder in output_folder_list:
        try:
            shutil.rmtree(output_folder)
        except:
            pass

    main_func()

    for output_folder in output_folder_list:
        shutil.rmtree(output_folder)


if __name__ == "__main__":
    test_run()
