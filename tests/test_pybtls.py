import PyBTLS


def test_run():
    config = PyBTLS.ConfigData()
    # config.read_data("BTLSin.txt")


    # config.veh_gen_grave(
    #     lanes_file="./LaneFlowData_80.csv",
    #     traffic_folder="../Traffic/Auxerre",
    #     no_days=10,
    #     truck_track_width=190.0,
    #     lane_eccentricity_std=0.0,
    # )

    # config.veh_gen_nominal(lanes_file="./LaneFlowData_80.csv", constant_file="./constant_vehicle.csv", no_days=10, lane_eccentricity_std=0.0, kernel_type=0)

    # config.veh_gen_garage(lanes_file="./LaneFlowData_80.csv", garage_file="./garage.txt", file_format=4, kernel_file="./kernels.csv", no_days=10, lane_eccentricity_std=20, kernel_type=0)

    # config.flow_gen_NHM(vehicle_classification=1, traffic_folder="../Traffic/Auxerre")

    # config.flow_gen_constant(vehicle_classification=1, constant_speed=36, constant_gap=10)

    # config.flow_gen_congestion(vehicle_classification=1, congested_spacing=26.1, congested_speed=49.7, congested_gap_coef_var=0.05)

    # config.flow_gen_freeflow(vehicle_classification=1)


    config.traffic_read(traffic_file="test_vehicle_file.txt", file_format=4, use_constant_speed=False, use_ave_speed=False, const_speed=80.0)


    config.simulation(
        bridge_file="./1-ABT6111Bridges.txt",
        infline_file="./1-ABT6111ILS.txt",
        infsurf_file="./IS.txt",
        calc_time_step=0.1,
        min_gvw=35,
    )


    # config.output_general()

    # config.output_vehicle_file(write_vehicle_file=True, write_flow_stats=True, buffer_size=100000)

    # config.output_BM(write_blockmax=True,write_summary=True)

    # config.output_POT(write_pot=True)

    # config.output_stats(write_stats=True)

    # config.output_fatigue(do_rainflow=True)


    PyBTLS.run()


if __name__ == '__main__':
    test_run()

