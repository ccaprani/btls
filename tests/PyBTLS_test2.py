import PyBTLS


garage_file = "./garage.txt"
vehicle_classification = PyBTLS.c_module.VehClassPattern()
vehicle_loader = PyBTLS.c_module.VehicleTrafficFile(vehicle_classification,False,False,0.0)
vehicle_loader.read(garage_file,4)
vehicle_list = vehicle_loader.get_vehicles()
for vehicle in vehicle_list:
    print(vehicle.get_class().m_Desc)
