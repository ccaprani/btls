import PyBTLS


# 目的是能单独完成尽可能小的模拟单项。完成每个函数的输入，输出值的设定。

# Please ignore the input and output of each function. 


# Vehicle
# basic funcs
def modify_vehicle(vehicle:PyBTLS.CVehicle_sp) -> PyBTLS.CVehicle_sp:
    # to modify or generate specific vehicles based on information provided. 
    pass
    # see Vehicle.cpp

def get_vehicle_classification(classification:int) -> PyBTLS.CVehicleClassification_sp:
    pass
    # CVehicleClassification_sp pVC
    # if classification == 1: # Pattern
    #     pVC = std::make_shared<CVehClassPattern>()
    # else: # Axle count
    #     pVC = std::make_shared<CVehClassAxle>()
    # return pVC


# Lane Flow  # Note the variables that are assigned to CConfigData:get().Road.*!
# basic funcs
def read_laneflow(laneflow_file:str) -> PyBTLS.CLaneFlowData:
    # to read the laneflow file. 
    pass
    # Need to modify current functions, as the laneflow_file path is currently passed by CConfigData.

def modify_laneflow(lane_flow_data:list, block_size:int=3600) -> PyBTLS.CLaneFlowData:
    # to define lane flow. lane_flow_data = [[index_lane, lane_dir, traffic_composition],[*],*], traffic_composition = [[index_block, mean_truck_flow_rate,std_truck_flow_rate,mean_traffic_velocity,std_traffic_velocity,percentage_car,percentage_truck_2Axle,percentage_truck_3Axle,percentage_truck_4Axle,percentage_truck_5Axle],[*],*]
    pass
    # CLaneFlowData LaneFlowData
    # for lane_flow in lane_flow_data:
    #     index_lane = lane_flow[0]
    #     lane_dir = lane_flow[1]
    #     no_block = len(lane_flow[2])
    #     CLaneFlowComposition lfc(index_lane, lane_dir, block_size)
    #     if traffic_composition in lane_flow[2]:
    #         lfc.addBlockData(traffic_composition) 
    #     lfc.completeData()
    #     LaneFlowData.m_vLaneComp.append(lfc)
    # LaneFlowData.SetRoadProperties()
    # return LaneFlowData


# Traffic
# basic funcs
def read_traffic_folder(pVC:PyBTLS.CVehicleClassification_sp,traffic_folder:str) -> PyBTLS.CVehModelDataGrave:
    # to read statistical info of traffic from traffic_folder. it's for grave gen.
    pass
    # CVehicleDataGrave.cpp: line 180 to 184

def modify_traffic_folder():
    # allow user to modify the PyBTLS.CVehModelDataGrave.
    pass

def gen_traffic_grave(traffic_folder:PyBTLS.CVehModelDataGrave,pVC:PyBTLS.CVehicleClassification_sp,laneflow:PyBTLS.CLaneFlowData):
    # make lanes' traffic flow generations (by grave) be ready for dosimulation(). output should be vpLanes = [CLane_sp,CLane_sp,*], SimStartTime and SimEndTime.
    pass
    # static vector<CLane_sp> vLanes
    # GetGeneratorLanes(pVC, vLanes):
    #     ...
    #     CLaneGenTraffic(gen_model = grave): *
    #     CVehicleGenGrave(laneflow): *
    #     ...
    #     return vpLanes, SimStartTime, SimEndTime
    # Need lots of modifications! 

def read_garage_file():
    pass

def modify_garage_file():
    pass

def read_kernel_file(a_series_of_input) -> list:
    # to set kernel for garage gen.
    pass

def modify_kernel_file():
    pass

def gen_traffic_garage(garage_file:str,kernel_file:str,pVC:PyBTLS.CVehicleClassification_sp,laneflow:PyBTLS.CLaneFlowData):
    # make lanes' traffic flow generations (by garage) be ready for dosimulation(). output should be vpLanes = [CLane_sp,CLane_sp,*], SimStartTime and SimEndTime.
    pass
    # static vector<CLane_sp> vLanes
    # GetGeneratorLanes(pVC, vLanes):
    #     ...
    #     CLaneGenTraffic(gen_model = garage): *
    #     CVehicleGenGarage(laneflow): *
    #     ...
    #     return vpLanes, SimStartTime, SimEndTime
    # Need lots of modifications! 

def read_const_vehicle():
    pass

def modify_const_vehicle():
    pass

def gen_traffic_const(const_file:str,pVC:PyBTLS.CVehicleClassification_sp,laneflow:PyBTLS.CLaneFlowData):
    # make lanes' traffic flow generations (by const) be ready for dosimulation(). output should be vpLanes = [CLane_sp,CLane_sp,*], SimStartTime and SimEndTime.
    pass
    # static vector<CLane_sp> vLanes
    # GetGeneratorLanes(pVC, vLanes):
    #     ...
    #     CLaneGenTraffic(gen_model = const): *
    #     CVehicleGenConst(laneflow): *
    #     ...
    #     return vpLanes, SimStartTime, SimEndTime
    # Need lots of modifications! 

def load_traffic(pVC:PyBTLS.CVehicleClassification_sp,traffic_file:str):
    # make lanes' traffic flows that are from a traffic record be ready for dosimulation(). output should be vpLanes = [CLane_sp,CLane_sp,*], SimStartTime and SimEndTime.
    pass
    # GetTrafficFileLanes(pVC, traffic_file):
    #     static vector<CLane_sp> vpLanes
    #     ...
    #     return vpLanes, SimStartTime, SimEndTime


# Bridge
# basic funcs
def read_bridge(bridge_file:str,inf_line_file:str,inf_surf_file:str) -> list:
    # to read the *bridge.txt file and return. output should be [PyBTLS.CBridge_sp,PyBTLS.CBridge_sp,*]
    pass
    # CReadILFile readIL
    # CBridgeFile BridgeFile(bridge_file,readIL.getInfLines(inf_line_file,0),readIL.getInfLines(inf_surf_file,1))
    # return BridgeFile.getBridges()

def modify_bridge(index:int,span:float,no_lanes:int,no_load_effs:int) -> PyBTLS.CBridge_sp:
    # to set and return a new bridge (no lane info). 
    pass
    # May need a new creator function in C++. 

def assign_inf_line(bridge:PyBTLS.CBridge_sp,index_lane:int,index_load_eff:int,index_inf_line:int=None,inf_line:PyBTLS.CInfluenceLine=None,factor:float=1.0,threshold:float=0.0) -> PyBTLS.CBridge_sp:
    # to assign an inf line to the input bridge. 
    if index_inf_line != None:
        # use intergrated inf line.
        pass
    elif inf_line != None:
        # use user defined inf line.
        pass
    # May need to define a new function. 

def show_bridge_info(bridge:PyBTLS.CBridge_sp) -> None:
    # to print bridge info in terminal, to check. 
    print("*bridge info*")


# Influence Line & Surface
# basic funcs
def read_inf_line(inf_line_file:str) -> list:
    # to read the inf line file. [PyBTLS.CInfluenceLine,PyBTLS.CInfluenceLine,*]
    pass
    # return CReadILFile::CReadILFile(inf_line_file, 0)

def modify_inf_line(a_series_of_inputs,inf_line:PyBTLS.CInfluenceLine) -> PyBTLS.CInfluenceLine:
    # to set an inf line. [[x],[z]]
    pass
    # CInfluenceLine infline
    # infline.setIL([x],[z])
    # return infline

def read_inf_surface(inf_surf_file:str) -> list:
    # to read the inf line file. [PyBTLS.CInfluenceLine,PyBTLS.CInfluenceLine,*]
    pass
    # return CReadILFile::CReadILFile(inf_surf_file, 1)

def modify_inf_surf(a_series_of_inputs,inf_surf:PyBTLS.CInfluenceLine) -> PyBTLS.CInfluenceLine:
    # to set an inf surface. data_points = [[0.0,y,y,*],[x,z,z,*],...,[x,z,z,*]], lane_location = [y,y,*]
    pass
    # CInfluenceLine infline
    # CInfluenceSurface IS
    # IS.setLanes(lane_location)
    # IS.setIS(data_points)
    # infline.setIL(IS)
    # return infline

def set_load_effcts(inf_line,load_factor,threshold):
    # to set load effest to the input bridge. 
    # load_effs = [[index_load_eff:int, [index_lane:int, inf_file:PyBTLS.CInfluenceLine or int, factor:double], ... , [index_lane:int, inf_file:PyBTLS.CInfluenceLine or int, factor:double], threshold:float], [*], *]
    pass
    # May need to define a new function. 
    # return load_effs?


# Run
def do_simulation(vehicle_class:PyBTLS.CVehicleClassification_sp, bridges:list, lane_traffic:list, StartTime:float, EndTime:float):
    # to do the simulation. bridges: [PyBTLS.CBridge_sp,PyBTLS.CBridge_sp,*], lanes_traffic: [PyBTLS.CLaneGenTraffic_sp,PyBTLS.CLaneGenTraffic_sp,*].
    pass

def do_simulation_single_vehicle(vehicle_class:PyBTLS.CVehicleClassification_sp, bridges:list, vehicle:PyBTLS.CVehicle):
    pass
    # replace the vehicle to PrepareSim.cpp: line 123. 


# Output --- Change to a better data format for output (SQL?). Only store few buffer-size data in memory for max..


