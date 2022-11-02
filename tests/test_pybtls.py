import PyBTLS


config = PyBTLS.ConfigData()

# config.ReadData("BTLSin.txt")

config.VehGenGrave(lanes_file="./LaneFlowData_80.csv", traffic_folder="../Traffic/Auxerre", no_days=10, truck_track_width=190.0, lane_eccentricity_std=0.0)

# config.VehGenConstant(lanes_file="./LaneFlowData_80.csv", constant_file="./constant_vehicle.csv", no_days=10, lane_eccentricity_std=0.0)

# config.VehGenGarage(lanes_file="./LaneFlowData_80.csv", garage_file="./garage.txt", file_format=4, kernel_file="./kernels.csv", no_days=10, lane_eccentricity_std=20)


config.FlowGenNHM(vehicle_classification=1, traffic_folder="../Traffic/Auxerre")

# config.FlowGenConstant(vehicle_classification=1, constant_speed=36, constant_gap=10)

# config.FlowGenCongestion(vehicle_classification=1, congested_spacing=26.1, congested_speed=49.7, congested_gap_coef_var=0.05)

# config.FlowGenFreeFlow(vehicle_classification=1)


config.Simulation(bridge_file="./1-ABT6111Bridges.txt", infline_file="./1-ABT6111ILS.txt", infsurf_file="./IS.txt", calc_time_step=0.1, min_gvw=35)


# config.OutputGeneral()

config.OutputVehicleFile(write_vehicle_file=True,vehicle_file_format=4,write_flow_stats=True,buffer_size=100000)

# config.OutputBlockMax(write_blockmax=True,write_summary=True)

# config.OutputPOT(write_pot=True)

# config.OutputStats(write_stats=True)

# config.OutputFatigue(do_rainflow=True)


PyBTLS.run()
