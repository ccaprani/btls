"""
End-to-end smoke test: build influence lines/surface, bridge, traffic
generator and loader, run both simulations on two cores, read every
output, and round-trip the output manifest.
"""

import pybtls as pb
from pathlib import Path
from utils import remove_folder


def test_sim_run():
    # Influence lines: built-in, discrete, and a surface
    inf_line_built_in = pb.InfluenceLine(IL_type="built-in")
    inf_line_built_in.set_IL(id=1, length=20.0)

    inf_line_discrete = pb.InfluenceLine(IL_type="discrete")
    inf_line_discrete.set_IL(position=[0.0, 10.0, 20.0], ordinate=[0.0, 5.0, 0.0])

    lane_position = [(0.5, 4.0), (4.0, 7.5), (8.5, 12.0), (12.0, 15.5)]
    IS_matrix = [
        [0.0, 0.0, 8.0, 16.0],
        [0.0, 0.0, 0.0, 0.0],
        [10.0, 0.0, 5.0, 0.0],
        [20.0, 0.0, 0.0, 0.0],
    ]
    inf_surf = pb.InfluenceSurface()
    inf_surf.set_IS(IS_matrix, lane_position)

    # Bridge with one identical-IL effect, one unique-IL effect, one surface effect
    bridge = pb.Bridge(length=20.0, no_lane=4)
    bridge.add_load_effect(
        inf_line_surf=inf_line_built_in,
        inf_weight=[1.0, 1.0, 1.0, 1.0],
        threshold=5.0,
    )
    bridge.add_load_effect(
        inf_line_surf=[
            inf_line_built_in,
            inf_line_discrete,
            inf_line_built_in,
            inf_line_discrete,
        ],
        inf_weight=[2.0, 2.0, 2.0, 2.0],
        threshold=10.0,
    )
    bridge.add_load_effect(inf_line_surf=inf_surf, threshold=0.0)

    # Nominal vehicle
    vehicle = pb.Vehicle(no_axles=3)
    vehicle.set_axle_weights([100.0, 100.0, 100.0])
    vehicle.set_axle_spacings([3.0, 7.0, 0.0])
    vehicle.set_axle_widths([2.0, 2.0, 2.0])

    # Lane flow compositions
    lfc_list = []
    for i in range(1, 5):
        lfc = pb.LaneFlowComposition(lane_index=i, lane_dir=i // 3 + 1)
        lfc.assign_lane_data(
            hourly_truck_flow=[80] * 24,
            hourly_car_flow=[20] * 24,
            hourly_speed_mean=[40 / 3.6 * 10] * 24,
            hourly_speed_std=[5.0] * 24,
            hourly_truck_composition=[[25.0, 25.0, 25.0, 25.0] for _ in range(24)],
        )
        lfc_list.append(lfc)
    lfc_1, lfc_2, lfc_3, lfc_4 = lfc_list

    # Vehicle generators: garage, nominal, grave
    garage = pb.garage.read_garage_file(
        garage_path=Path(__file__).parent / "test_data/garage.txt", garage_format=4
    )
    kernel = [[1.0, 0.08], [1.0, 0.05], [1.0, 0.02]]
    vehicle_gen_garage = pb.VehicleGenGarage(garage=garage, kernel=kernel)
    vehicle_gen_nominal = pb.VehicleGenNominal(
        nominal_vehicle=vehicle, COV_list=[1.0, 0.1]
    )
    vehicle_gen_grave = pb.VehicleGenGrave(
        traffic_site="Auxerre", truck_track_width=190.0
    )

    # Headway generators: HeDS, constant, congested, freeflow
    headway_gen_HeDS = pb.HeadwayGenHeDS()
    headway_gen_constant = pb.HeadwayGenConstant(constant_speed=36.0, constant_gap=5.0)
    headway_gen_congested = pb.HeadwayGenCongested(
        congested_spacing=26.1, congested_speed=36.0, congested_gap_coef_var=0.05
    )
    headway_gen_freeflow = pb.HeadwayGenFreeflow()

    # Traffic generator combining all of the above
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

    # Traffic loader replaying a recorded file
    traffic_loader = pb.TrafficLoader(no_lane=4)
    traffic_loader.add_traffic(
        traffic=Path(__file__).parent / "test_data/test_traffic_file.txt",
        traffic_format=4,
        use_average_speed=False,
        use_const_speed=True,
        const_speed_value=40.0,
    )

    # Output configurations
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

    output_config_load = pb.OutputConfig()
    output_config_load.set_vehicle_file_output(
        write_vehicle_file=True,
        vehicle_file_format=4,
        vehicle_file_name="proof_traffic_file.txt",
    )
    output_config_load.set_stats_output(
        write_flow_stats=True, write_overall=True, write_intervals=True
    )

    # Run both simulations on two cores
    remove_folder(Path(__file__).parent / "temp_data")
    sim_task = pb.Simulation(output_dir=Path(__file__).parent / "temp_data")
    sim_task.add_sim(
        bridge=bridge,
        traffic=traffic_gen,
        no_day=1,
        output_config=output_config_gen,
        time_step=0.5,
        min_gvw=35,
        tag="Generate",
        track_progress=False,
    )
    sim_task.add_sim(
        bridge=bridge,
        traffic=traffic_loader,
        output_config=output_config_load,
        time_step=0.5,
        min_gvw=35,
        active_lane=[1, 2],
        tag="Load",
        track_progress=False,
    )
    sim_task.run(no_core=2)

    # Every configured output must read back as a DataFrame
    sim_output_all = sim_task.get_output()
    for sim_tag, sim_output in sim_output_all.items():
        for output_type in sim_output.get_summary():
            data = sim_output.read_data(output_type)
            assert data, f"{sim_tag}/{output_type} produced no files"

    # Manifest round-trip
    manifest_path = Path(__file__).parent / "temp_data/simulation_output.json"
    pb.save_output(sim_output_all, file_path=manifest_path)
    sim_output_loaded = pb.load_output(file_path=manifest_path)
    assert set(sim_output_loaded) == set(sim_output_all)

    # A quick numerical check: effect 2 uses the same IL as effect 1 with
    # double weight on two lanes, so 2*E1 - E2 stays near zero
    time_history_data = sim_output_loaded["Generate"].read_data("time_history")["TH_20"]
    assert (
        (2 * time_history_data["Effect 1"] - time_history_data["Effect 2"]).abs() <= 0.5
    ).all()

    # Clean up the temporary folder
    remove_folder(Path(__file__).parent / "temp_data")
