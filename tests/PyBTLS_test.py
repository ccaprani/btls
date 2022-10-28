import PyBTLS


py_config = PyBTLS.PyConfigData()

# py_config.ReadData("BTLSin.txt")
# py_config.Gen_GEN_CAR = True

py_config.VehGenGrave(lanes_file="./LaneFlowData_80.csv", traffic_folder="../Traffic/Auxerre", no_days=10, truck_track_width=190.0, lane_eccentricity_std=0.0)

# py_config.VehGenConstant(lanes_file="./LaneFlowData_80.csv", constant_file="./constant_vehicle.csv", no_days=10, lane_eccentricity_std=0.0)

# py_config.VehGenGarage(lanes_file="./LaneFlowData_80.csv", garage_file="./garage.txt", file_format=4, kernel_file="./kernels.csv", no_days=10, lane_eccentricity_std=20)


py_config.FlowGenNHM(vehicle_classification=1, traffic_folder="../Traffic/Auxerre")
print(py_config.Gen_TRAFFIC_FOLDER)

# py_config.FlowGenConstant(vehicle_classification=1, constant_speed=36, constant_gap=10)

# py_config.FlowGenCongestion(vehicle_classification=1, congested_spacing=26.1, congested_speed=49.7, congested_gap_coef_var=0.05)

# py_config.FlowGenFreeFlow(vehicle_classification=1)


py_config.Simulation(bridge_file="./1-ABT6111Bridges.txt", infline_file="./1-ABT6111ILS.txt", infsurf_file="./IS.txt", calc_time_step=0.1, min_gvw=35)


# py_config.OutputGeneral()

py_config.OutputVehicleFile(write_vehicle_file=True,write_flow_stats=True,buffer_size=100000)

# py_config.OutputBlockMax(write_blockmax=True,write_summary=True)

# py_config.OutputPOT(write_pot=True)

# py_config.OutputStats(write_stats=True)

# py_config.OutputFatigue(do_rainflow=True)


vehicle_classification = PyBTLS.get_vehicle_classification(py_config)

start_time = 0.0
end_time = 0.0

bridge_list, py_config = PyBTLS.prepare_bridges(py_config) 

lane_list, end_time, py_config = PyBTLS.get_generator_lanes(vehicle_classification,start_time,py_config)
for i in range(len(bridge_list)):
    bridge_list[i].initialize_data_manager(start_time)

PyBTLS.do_simulation(vehicle_classification,bridge_list,lane_list,start_time,end_time,py_config)
