import pybtls as pb
import pytest
from pathlib import Path
from utils import remove_folder


def test_sim_run():

    # Influence line built-in
    try:
        inf_line_built_in = pb.InfluenceLine(IL_type='built-in')
        inf_line_built_in.set_IL(id=1, length=20.0)
    except Exception as e:
        pytest.fail(f"Built-in influence line setup failed: {e}")

    # Influence line discrete
    try:
        inf_line_discrete = pb.InfluenceLine(IL_type='discrete')
        inf_line_discrete.set_IL(position=[0.0, 10.0, 20.0], ordinate=[0.0, 5.0, 0.0])
    except Exception as e:
        pytest.fail(f"Discrete influence line setup failed: {e}")
    
    # Influence surface
    try:
        lane_position = [(0.5, 4.0), (4.0, 7.5), (8.5, 12.0), (12.0, 15.5)]
        IS_matrix = [
            [0.0, 0.0, 8.0, 16.0],
            [0.0, 0.0, 0.0, 0.0],
            [10.0, 0.0, 5.0, 0.0],
            [20.0, 0.0, 0.0, 0.0],
        ]
        inf_surf = pb.InfluenceSurface()
        inf_surf.set_IS(IS_matrix, lane_position)
    except Exception as e:
        pytest.fail(f"Influence surface setup failed: {e}")


    # Bridge
    bridge = pb.Bridge(length=20.0, no_lane=4)
    try:
        bridge.add_load_effect(inf_line_surf=inf_line_built_in, inf_weight=[1.0, 1.0, 1.0, 1.0], threshold=5.0)
    except Exception as e:
        pytest.fail(f"Adding load effect with identical IL failed: {e}")
    try:
        bridge.add_load_effect(inf_line_surf=[inf_line_built_in, inf_line_discrete, inf_line_built_in, inf_line_discrete], inf_weight=[2.0, 2.0, 2.0, 2.0], threshold=10.0)
    except Exception as e:
        pytest.fail(f"Adding load effect with unique ILs failed: {e}")
    try:
        bridge.add_load_effect(inf_line_surf=inf_surf, threshold=0.0)
    except Exception as e:
        pytest.fail(f"Adding load effect with influence surface failed: {e}")


    # Vehicle
    try:
        vehicle = pb.Vehicle(no_axles=3)
        vehicle.set_axle_weights([100.0, 100.0, 100.0])
        vehicle.set_axle_spacings([3.0, 7.0, 0.0])
        vehicle.set_axle_widths([2.0, 2.0, 2.0])
    except Exception as e:
        pytest.fail(f"Vehicle setup failed: {e}")


    # Lane flow composition
    try: 
        lfc_list = []
        for i in range(1, 5):
            lfc = pb.LaneFlowComposition(lane_index=i, lane_dir=i//3+1)
            lfc.assign_lane_data(
                hourly_truck_flow=[100] * 24,
                hourly_car_flow=[0] * 24,
                hourly_speed_mean=[80 / 3.6 * 10] * 24,
                hourly_speed_std=[10.0] * 24,
                hourly_truck_composition=[[25.0, 25.0, 25.0, 25.0] for _ in range(24)],
            )
            lfc_list.append(lfc)
        lfc_1, lfc_2, lfc_3, lfc_4 = lfc_list
    except Exception as e:
        pytest.fail(f"Lane flow composition setup failed: {e}")


    # Vehicle generator garage
    try:
        garage = pb.garage.read_garage_file(garage_path=Path("./test_data/garage.txt"), garage_format=4)
        kernel = [[1.0, 0.08], [1.0, 0.05], [1.0, 0.02]]
        vehicle_gen_garage = pb.VehicleGenGarage(garage=garage, kernel=kernel)
    except Exception as e:
        pytest.fail(f"Vehicle generator garage setup failed: {e}")

    # Vehicle generator nominal
    try:
        vehicle_gen_nominal = pb.VehicleGenNominal(nominal_vehicle=vehicle, COV_list=[1.0, 0.1])
    except Exception as e:
        pytest.fail(f"Vehicle generator nominal setup failed: {e}")
    
    # Vehicle generator grave
    try:
        vehicle_gen_grave = pb.VehicleGenGrave(traffic_site="Auxerre", truck_track_width=190.0)
    except Exception as e:
        pytest.fail(f"Vehicle generator grave setup failed: {e}")


    # Headway generator HeDS
    try:
        headway_gen_HeDS = pb.HeadwayGenHeDS()
    except Exception as e:
        pytest.fail(f"Headway generator HeDS setup failed: {e}")

    # Headway generator constant
    try:
        headway_gen_constant = pb.HeadwayGenConstant(constant_speed=36.0, constant_gap=5.0)
    except Exception as e:
        pytest.fail(f"Headway generator constant setup failed: {e}")

    # Headway generator congested
    try:
        headway_gen_congested = pb.HeadwayGenCongested(
            congested_spacing=26.1, congested_speed=36.0, congested_gap_coef_var=0.05
        )
    except Exception as e:
        pytest.fail(f"Headway generator congested setup failed: {e}")

    # Headway generator freeflow
    try:
        headway_gen_freeflow = pb.HeadwayGenFreeflow()
    except Exception as e:
        pytest.fail(f"Headway generator freeflow setup failed: {e}")


    # Traffic generator
    try:
        traffic_gen = pb.TrafficGenerator(no_lane=4)
        traffic_gen.add_lane(
            vehicle_gen=vehicle_gen_grave, headway_gen=headway_gen_HeDS, lfc=lfc_1
        )
        traffic_gen.add_lane(
            vehicle_gen=vehicle_gen_garage, headway_gen=headway_gen_congested, lfc=lfc_2
        )
        traffic_gen.add_lane(
            vehicle_gen=vehicle_gen_garage, headway_gen=headway_gen_freeflow, lfc=lfc_3
        )
        traffic_gen.add_lane(
            vehicle_gen=vehicle_gen_nominal, headway_gen=headway_gen_constant, lfc=lfc_4
        )
        traffic_gen.set_start_time(0.0)
    except Exception as e:
        pytest.fail(f"Traffic generator setup failed: {e}")


    # Traffic loader
    try:
        traffic_loader = pb.TrafficLoader(no_lane=4)
        traffic_loader.add_traffic(
            traffic="./test_data/test_traffic_file.txt",
            traffic_format=4,
            use_average_speed=False,
            use_const_speed=True,
            const_speed_value=40.0,
        )
    except Exception as e:
        pytest.fail(f"Traffic loader setup failed: {e}")


    # Output configuration traffic generator
    try:
        output_config_gen = pb.OutputConfig()
        output_config_gen.set_event_output(write_time_history=True, write_each_event=True)
        output_config_gen.set_BM_output(
            write_vehicle=True, write_summary=True, write_mixed=True
        )
        output_config_gen.set_POT_output(
            write_vehicle=True, write_summary=True, write_counter=True
        )
        output_config_gen.set_fatigue_output(
            write_fatigue_event=True, write_rainflow_output=True
        )
        output_config_gen.set_stats_output(
            write_flow_stats=True, write_overall=True, write_intervals=True
        )
    except Exception as e:
        pytest.fail(f"Output configuration for traffic generator setup failed: {e}")


    # Output configuration traffic loader
    try:
        output_config_load = pb.OutputConfig()
        output_config_load.set_vehicle_file_output(
            write_vehicle_file=True,
            vehicle_file_format=4,
            vehicle_file_name="proof_traffic_file.txt",
        )
        output_config_load.set_stats_output(
            write_flow_stats=True, write_overall=True, write_intervals=True
        )
    except Exception as e:
        pytest.fail(f"Output configuration for traffic loader setup failed: {e}")


    # Create simulation
    remove_folder("./temp_data")
    try:
        sim_task = pb.Simulation(output_dir=Path("./temp_data"))
        sim_task.add_sim(
            bridge=bridge,
            traffic=traffic_gen,
            no_day=10,
            output_config=output_config_gen,
            time_step=0.1,
            min_gvw=35,
            tag="Generate",
            track_progress=False,
        )
        sim_task.add_sim(
            bridge=bridge,
            traffic=traffic_loader,
            output_config=output_config_load,
            time_step=0.1,
            min_gvw=35,
            active_lane=[1, 2],
            tag="Load",
            track_progress=False,
        )
    except Exception as e:
        pytest.fail(f"Simulation setup failed: {e}")

        
    # Run simulation
    try:
        sim_task.run(no_core=2)
    except Exception as e:
        pytest.fail(f"Simulation run failed: {e}")


    # Load the simulation results
    try:
        sim_output_all = sim_task.get_output()
        for sim_tag, sim_output in sim_output_all.items():
            for output_type in sim_output.get_summary():
                for output_name in sim_output.read_data(output_type):
                    sim_output.read_data(output_type)[output_name]
    except Exception as e:
        pytest.fail(f"Read {sim_tag} {output_type} {output_name} output failed: {e}")


    # Save the simulation output
    try: 
        pb.save_output(sim_output_all, file_path=Path("./temp_data/simulation_output.pkl"))
    except Exception as e:
        pytest.fail(f"Saving simulation output failed: {e}")


    # Load the simulation output
    try:
        sim_output_loaded = pb.load_output(file_path=Path("./temp_data/simulation_output.pkl"))
    except Exception as e:
        pytest.fail(f"Loading simulation output failed: {e}")
        

    # A quick test to the simulation output
    time_history_data = sim_output_loaded["Generate"].read_data("time_history")["TH_20"]
    assert ((2*time_history_data["Effect 1"] - time_history_data["Effect 2"]).abs() <= 0.5).all()


    # Clean up the temporary folder
    remove_folder("./temp_data")

