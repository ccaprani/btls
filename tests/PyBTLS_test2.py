import PyBTLS


# read file test
garage_file = "./garage.txt"
vehicle_classification = PyBTLS.c_module.VehClassPattern()
vehicle_loader = PyBTLS.c_module.VehicleTrafficFile(vehicle_classification,False,False,0.0)
vehicle_loader.read(garage_file,4)
vehicle_list = vehicle_loader.get_vehicles()
for vehicle in vehicle_list:
    print(vehicle.get_class().m_Desc)


# step-by-step set
lane_flow = PyBTLS.cpp.LaneFlowData()
lane_flow.read_data_in("./LaneFlowData_80.csv")
lane_flow.get_lane_composition(0)

garage_model_data = PyBTLS.cpp.VehModelDataGarage(vehicle_classification,lane_flow.get_lane_composition(0))
garage_model_data.read_garage("./garage.txt")
garage_model_data.read_kernel("./kernels.csv")

garage_generator = PyBTLS.cpp.VehicleGenGarage(garage_model_data)

vehicle = garage_generator.generator(0)
vehicle_class = vehicle.get_class()
print(vehicle_class.m_Desc)
