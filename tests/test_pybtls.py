import PyBTLS


def test_run():
    config = PyBTLS.ConfigData()

    # config.read_data("BTLSin.txt")

    config.veh_gen_grave(
        vehicle_classifier=1,
        traffic_folder="../Traffic/Auxerre",
        truck_track_width=190.0,
        lane_eccentricity_std=0.0,
        kernel_type=1
    )

    # config.veh_gen_nominal(vehicle_classifier=1, nominal_file="./nominal_vehicle.csv", lane_eccentricity_std=0.0, kernel_type=1)

    # config.veh_gen_garage(vehicle_classifier=1, garage_file="./garage.txt", file_format=4, kernel_file="./kernels.csv", lane_eccentricity_std=0.0, kernel_type=1)

    config.flow_gen_NHM(lanes_file="./LaneFlowData_80.csv", traffic_folder="../Traffic/Auxerre", no_days=10)

    # config.flow_gen_constant(lanes_file="./LaneFlowData_80.csv", constant_speed=36, constant_gap=10, no_days=10)

    # config.flow_gen_congested(lanes_file="./LaneFlowData_80.csv", congested_spacing=26.1, congested_speed=49.7, congested_gap_coef_var=0.05, no_days=10)

    # config.flow_gen_freeflow(lanes_file="./LaneFlowData_80.csv", no_days=10)

    config.simulation(
        bridge_file="./1-ABT6111Bridges.txt",
        infline_file="./1-ABT6111ILS.txt",
        infsurf_file="./IS.txt",
        calc_time_step=0.1,
        min_gvw=35,
    )

    # config.output_general()

    config.output_vehicle_file(
        write_vehicle_file=True, write_flow_stats=True, buffer_size=100000
    )

    # config.output_BM(write_blockmax=True,write_summary=True)

    # config.output_POT(write_pot=True)

    config.output_vehicle_file(write_vehicle_file=True,vehicle_file_format=4,write_flow_stats=True,buffer_size=100000)

    # config.output_stats(write_stats=True)

    # config.output_fatigue(do_rainflow=True)

    PyBTLS.run()
