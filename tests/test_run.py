import pybtls as pb
import pandas as pd
import pytest
import shutil
from pathlib import Path


def main_func():
    # Set bridges
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

    # Set lane flow compositions
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

    # Set vehicle generator
    garage = pb.garage.read_garage_file(Path(__file__).parent/"test_data/garage.txt", 4)
    kernel = [[1.0, 0.08], [1.0, 0.05], [1.0, 0.02]]
    vehicle_gen_1 = pb.VehicleGenGarage(garage=garage, kernel=kernel)

    vehicle = pb.Vehicle(no_axle=3)
    vehicle.set_axle_weights([100.0, 100.0, 100.0])
    vehicle.set_axle_spacings([3.0, 7.0, 0.0])
    vehicle.set_axle_widths([2.0, 2.0, 2.0])

    vehicle_gen_2 = pb.VehicleGenNominal(nominal_vehicle=vehicle, COV_list=[1.0, 0.1])

    vehicle_gen_3 = pb.VehicleGenGrave(traffic_site="Auxerre")

    # Set headway generator
    headway_gen_1 = pb.HeadwayGenNHM()

    headway_gen_2 = pb.HeadwayGenConstant(constant_speed=36.0, constant_gap=5.0)

    headway_gen_3 = pb.HeadwayGenCongested(
        congested_spacing=26.1, congested_speed=36.0, congested_gap_coef_var=0.05
    )

    headway_gen_4 = pb.HeadwayGenFreeflow()

    # Assemble traffic generator
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

    # or, Use traffic loader
    traffic_loader = pb.TrafficLoader(no_lane=4)
    traffic_loader.add_traffic(
        traffic=Path(__file__).parent/"test_data/test_traffic_file.txt",
        traffic_format=4,
        use_average_speed=False,
        use_const_speed=False,
        const_speed_value=36.0,
    )

    # Set output, which will be written to HDD
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
    sim_task = pb.Simulation(Path(__file__).parent/"temp")
    sim_task.add_sim(
        bridge=bridge_1,
        traffic_generator=traffic_gen,
        no_day=1,
        output_config=output_config_1,
        time_step=0.1,
        min_gvw=35,
        active_lane=[1, 2],
        track_progress=False,
        tag="Sim_1",
    )
    sim_task.add_sim(
        bridge=bridge_2,
        traffic_generator=traffic_gen,
        no_day=1,
        output_config=output_config_1,
        time_step=0.1,
        min_gvw=35,
        track_progress=False,
        tag="Sim_2",
    )
    sim_task.add_sim(
        bridge=bridge_2,
        traffic_generator=traffic_loader,
        output_config=output_config_2,
        time_step=0.1,
        min_gvw=0,
        track_progress=False,
        tag="Sim_3",
    )
    sim_task.add_sim(bridge=bridge_1, vehicle=vehicle, tag="Sim_4")
    sim_task.add_sim(bridge=bridge_2, vehicle=vehicle, active_lane=[1], tag="Sim_5")
    sim_task.add_sim(bridge=bridge_2, vehicle=vehicle, active_lane=[2], tag="Sim_6")
    sim_task.add_sim(bridge=bridge_2, vehicle=vehicle, active_lane=[3], tag="Sim_7")
    sim_task.add_sim(bridge=bridge_2, vehicle=vehicle, active_lane=[4], tag="Sim_8")

    # run simulation simultaneously
    sim_task.run(no_core=4)

    sim_out = sim_task.get_output()

    for output in sim_out.values():
        for key in output.get_summary():
            for data_dict in output.read_data(key):
                for data in data_dict.values():
                    assert isinstance(data, pd.DataFrame)
                

@pytest.mark.skip(reason="This test is too long.")
def test_run():
    output_folder_list = [Path(__file__)/f"temp/Sim_{i+1}" for i in range(8)]

    for output_folder in output_folder_list:
        try:
            shutil.rmtree(output_folder)
        except:
            pass

    main_func()

    for output_folder in output_folder_list:
        shutil.rmtree(output_folder)
