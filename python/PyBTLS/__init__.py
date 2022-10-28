from PyBTLS.cpp import PyConfigData, Vehicle, VehClassAxle, VehClassPattern, Bridge, InfluenceLine, InfluenceSurface, LaneFileTraffic, LaneGenTraffic, VehicleBuffer, ReadILFile, BridgeFile, LaneFlowData, VehicleGenConstant, VehicleGenGarage, VehicleGenGrave, VehicleTrafficFile, VehModelDataConstant, VehModelDataGarage, VehModelDataGrave
# from PyBTLS.python import Settings, FatigueCalculation


def get_vehicle_classification(py_config):
    if py_config.Traffic_CLASSIFICATION == 1:
        vehicle_classification = VehClassPattern()
    else:
        vehicle_classification = VehClassAxle()
    return vehicle_classification


def prepare_bridges(py_config):
    read_IL = ReadILFile()

    bridge_file = BridgeFile(py_config.Sim_BRIDGE_FILE, read_IL.get_inf_lines(
    py_config.Sim_INFLINE_FILE, 0), read_IL.get_inf_lines(py_config.Sim_INFSURF_FILE, 1), py_config)

    bridge_list = bridge_file.get_bridges()
    py_config.Gen_NO_OVERLAP_LENGTH = bridge_file.get_max_bridge_length()

    for i in range(len(bridge_list)):
        bridge_list[i].set_calc_time_step(py_config.Sim_CALC_TIME_STEP)

    return bridge_list, py_config


def get_generator_lanes(vehicle_classification, start_time, py_config):

    end_time = (py_config.Gen_NO_DAYS)*24*3600.0

    lane_flow_data = LaneFlowData(py_config)
    lane_flow_data.read_data_in()
    lane_flow_data.get_lane_composition(0)

    py_config.Road_NO_LANES = lane_flow_data.get_no_lanes()
    py_config.Road_NO_DIRS = lane_flow_data.get_no_dirn()
    py_config.Road_NO_LANES_DIR1 = lane_flow_data.get_no_lanes_dir1()
    py_config.Road_NO_LANES_DIR2 = lane_flow_data.get_no_lanes_dir2()

    lane_list = []
    for i in range(lane_flow_data.get_no_lanes()):
        lane = LaneGenTraffic(py_config)
        lane.set_lane_data(vehicle_classification, lane_flow_data.get_lane_composition(i), start_time, py_config)
        lane_list.append(lane)
    
    return lane_list, end_time, py_config


def do_simulation(vehicle_classification, bridge_list, lane_list, sim_start_time, sim_end_time, py_config):
    vehicle_buffer = VehicleBuffer(vehicle_classification, sim_start_time, py_config)
    current_time = sim_start_time
    current_day = int(sim_start_time/86400)
    sim_process_print = ""

    print("Starting simulation...")
    print("Day complete...")

    while current_time <= sim_end_time:
        lane_list = sorted(lane_list, key=lambda t: t.get_next_arrival_time())
        next_arrival_time = lane_list[0].get_next_arrival_time()

        vehicle = lane_list[0].get_next_vehicle()
        vehicle_buffer.add_vehicle(vehicle)

        if py_config.Sim_CALC_LOAD_EFFECTS:
            for i in range(len(bridge_list)):
                bridge_list[i].update(next_arrival_time, current_time)
                if vehicle.get_gvw() > py_config.Sim_MIN_GVW:
                    bridge_list[i].add_vehicle(vehicle)

        current_time = vehicle.get_time()
        if current_time > 86400*(current_day+1):
            current_day += 1
            sim_process_print += "\t" + str(current_day)
            if current_day % 10 == 0:
                print(sim_process_print)
                sim_process_print = ""

    if py_config.Sim_CALC_LOAD_EFFECTS:
        for i in range(len(bridge_list)):
            bridge_list[i].finish()

    vehicle_buffer.flush_buffer()


if __name__ == "__main__":
    pass

