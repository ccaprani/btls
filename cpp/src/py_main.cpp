// py_main.cpp
// the main file for the PyBTLS Build

#include "PrepareSim.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl/filesystem.h"


using namespace std;
namespace py = pybind11;


void get_info() {
	cout << "---------------------------------------------" << endl;
	cout << "Bridge Traffic Load Simulation - C.C. Caprani" << endl;
	cout << "                Version 1.3.7			      " << endl;
	cout << "---------------------------------------------" << endl << endl;
};


PYBIND11_MODULE(_core, m) {
	m.doc() = ("PyBTLS is for short-to-mid span bridge traffic loading simulation.");
	m.def("get_info", &get_info);
	// py::class_<CConfigData> cconfigdata(m, "ConfigData");
	py::class_<CConfigDataCore> cconfigdatacore(m, "ConfigDataCore");
		cconfigdatacore.def(py::init<>())
			.def("ReadData", &CConfigDataCore::ReadData, py::arg("in_file"))
			.def("VehGenGrave", &CConfigDataCore::VehGenGrave, py::arg("lanes_file"), py::arg("traffic_folder"), py::arg("no_days"), py::arg("truck_track_width"), py::arg("lane_eccentricity_std"))
			.def("VehGenGarage", &CConfigDataCore::VehGenGarage, py::arg("lanes_file"), py::arg("garage_file"), py::arg("file_format"), py::arg("kernel_file"), py::arg("no_days"), py::arg("lane_eccentricity_std"))
			.def("VehGenConstant", &CConfigDataCore::VehGenConstant, py::arg("lanes_file"), py::arg("constant_file"), py::arg("no_days"), py::arg("lane_eccentricity_std"))
			.def("FlowGenNHM", &CConfigDataCore::FlowGenNHM, py::arg("vehicle_classification"), py::arg("traffic_folder"))
			.def("FlowGenConstant", &CConfigDataCore::FlowGenConstant, py::arg("vehicle_classification"), py::arg("constant_speed"), py::arg("constant_gap"))
			.def("FlowGenCongestion", &CConfigDataCore::FlowGenCongestion, py::arg("vehicle_classification"), py::arg("congested_spacing"), py::arg("congested_speed"), py::arg("congested_gap_coef_var"))
			.def("FlowGenFreeFlow", &CConfigDataCore::FlowGenFreeFlow, py::arg("vehicle_classification"))
			.def("TrafficRead", &CConfigDataCore::TrafficRead, py::arg("traffic_file"), py::arg("file_format"), py::arg("use_constant_speed"), py::arg("use_ave_speed"), py::arg("const_speed"))
			.def("Simulation", &CConfigDataCore::Simulation, py::arg("bridge_file"), py::arg("infline_file"), py::arg("infsurf_file"), py::arg("calc_time_step"), py::arg("min_gvw"))
			.def("OutputGeneral", &CConfigDataCore::OutputGeneral, py::arg("write_time_history")=false, py::arg("write_each_event")=false, py::arg("write_fatigue_event")=false, py::arg("write_buffer_size")=1000)
			.def("OutputVehicleFile", &CConfigDataCore::OutputVehicleFile, py::arg("write_vehicle_file")=false, py::arg("vehicle_file_format")=4, py::arg("vehicle_file_name")="Vehicles.txt", py::arg("buffer_size")=10000, py::arg("write_flow_stats")=false)
			.def("OutputBlockMax", &CConfigDataCore::OutputBlockMax, py::arg("write_blockmax")=false, py::arg("write_vehicle")=false, py::arg("write_summary")=false, py::arg("write_mixed")=false, py::arg("block_size_days")=1, py::arg("block_size_secs")=0, py::arg("buffer_size")=10000)
			.def("OutputPOT", &CConfigDataCore::OutputPOT, py::arg("write_pot")=false, py::arg("write_vehicle")=false, py::arg("write_summary")=false, py::arg("write_counter")=false, py::arg("pot_size_days")=1, py::arg("pot_size_secs")=0, py::arg("buffer_size")=10000)
			.def("OutputStats", &CConfigDataCore::OutputStats, py::arg("write_stats")=false, py::arg("write_cumulative")=false, py::arg("write_intervals")=false, py::arg("interval_size")=3600, py::arg("buffer_size")=10000)
			.def("OutputFatigue", &CConfigDataCore::OutputFatigue, py::arg("do_rainflow")=false, py::arg("decimal")=3, py::arg("cut_off_value")=0.0, py::arg("write_buffer_size")=10000)
			.def_readwrite("Road", &CConfigDataCore::Road)
			.def_readwrite("Gen", &CConfigDataCore::Gen)
			.def_readwrite("Read", &CConfigDataCore::Read)
			.def_readwrite("Traffic", &CConfigDataCore::Traffic)
			.def_readwrite("Sim", &CConfigDataCore::Sim)
			.def_readwrite("Output", &CConfigDataCore::Output);
		py::class_<CConfigDataCore::Road_Config> road_config(cconfigdatacore, "Road_Config");
			road_config.def_readwrite("LANES_FILE", &CConfigDataCore::Road_Config::LANES_FILE)
				.def_readwrite("NO_LANES_DIR1", &CConfigDataCore::Road_Config::NO_LANES_DIR1)
				.def_readwrite("NO_LANES_DIR2", &CConfigDataCore::Road_Config::NO_LANES_DIR2)
				.def_readwrite("NO_LANES", &CConfigDataCore::Road_Config::NO_LANES)
				.def_readwrite("NO_DIRS", &CConfigDataCore::Road_Config::NO_DIRS);
		py::class_<CConfigDataCore::Gen_Config> gen_config(cconfigdatacore, "Gen_Config");
			gen_config.def_readwrite("TRAFFIC_FOLDER", &CConfigDataCore::Gen_Config::TRAFFIC_FOLDER)
				.def_readwrite("GEN_TRAFFIC", &CConfigDataCore::Gen_Config::GEN_TRAFFIC)
				.def_readwrite("NO_DAYS", &CConfigDataCore::Gen_Config::NO_DAYS)
				.def_readwrite("TRUCK_TRACK_WIDTH", &CConfigDataCore::Gen_Config::TRUCK_TRACK_WIDTH)
				.def_readwrite("LANE_ECCENTRICITY_STD", &CConfigDataCore::Gen_Config::LANE_ECCENTRICITY_STD)
				.def_readwrite("NO_OVERLAP_LENGTH", &CConfigDataCore::Gen_Config::NO_OVERLAP_LENGTH);
		py::class_<CConfigDataCore::Read_Config> read_config(cconfigdatacore, "Read_Config");
			read_config.def_readwrite("READ_FILE", &CConfigDataCore::Read_Config::READ_FILE)
				.def_readwrite("TRAFFIC_FILE", &CConfigDataCore::Read_Config::TRAFFIC_FILE)
				.def_readwrite("GARAGE_FILE", &CConfigDataCore::Read_Config::GARAGE_FILE)
				.def_readwrite("KERNEL_FILE", &CConfigDataCore::Read_Config::KERNEL_FILE)
				.def_readwrite("CONSTANT_FILE", &CConfigDataCore::Read_Config::CONSTANT_FILE)
				.def_readwrite("FILE_FORMAT", &CConfigDataCore::Read_Config::FILE_FORMAT)
				.def_readwrite("USE_CONSTANT_SPEED", &CConfigDataCore::Read_Config::USE_CONSTANT_SPEED)
				.def_readwrite("USE_AVE_SPEED", &CConfigDataCore::Read_Config::USE_AVE_SPEED)
				.def_readwrite("CONST_SPEED", &CConfigDataCore::Read_Config::CONST_SPEED);
		py::class_<CConfigDataCore::Traffic_Config> traffic_config(cconfigdatacore, "Traffic_Config");
			traffic_config.def_readwrite("VEHICLE_MODEL", &CConfigDataCore::Traffic_Config::VEHICLE_MODEL)
				.def_readwrite("HEADWAY_MODEL", &CConfigDataCore::Traffic_Config::HEADWAY_MODEL)
				.def_readwrite("CLASSIFICATION", &CConfigDataCore::Traffic_Config::CLASSIFICATION)
				.def_readwrite("CONGESTED_SPACING", &CConfigDataCore::Traffic_Config::CONGESTED_SPACING)
				.def_readwrite("CONGESTED_SPEED", &CConfigDataCore::Traffic_Config::CONGESTED_SPEED)
				.def_readwrite("CONGESTED_GAP", &CConfigDataCore::Traffic_Config::CONGESTED_GAP)
				.def_readwrite("CONGESTED_GAP_COEF_VAR", &CConfigDataCore::Traffic_Config::CONGESTED_GAP_COEF_VAR)
				.def_readwrite("CONSTANT_SPEED", &CConfigDataCore::Traffic_Config::CONSTANT_SPEED)
				.def_readwrite("CONSTANT_GAP", &CConfigDataCore::Traffic_Config::CONSTANT_GAP);
		py::class_<CConfigDataCore::Sim_Config> sim_config(cconfigdatacore, "Sim_Config");
			sim_config.def_readwrite("CALC_LOAD_EFFECTS", &CConfigDataCore::Sim_Config::CALC_LOAD_EFFECTS)
				.def_readwrite("BRIDGE_FILE", &CConfigDataCore::Sim_Config::BRIDGE_FILE)
				.def_readwrite("INFLINE_FILE", &CConfigDataCore::Sim_Config::INFLINE_FILE)
				.def_readwrite("INFSURF_FILE", &CConfigDataCore::Sim_Config::INFSURF_FILE)
				.def_readwrite("CALC_TIME_STEP", &CConfigDataCore::Sim_Config::CALC_TIME_STEP)
				.def_readwrite("MIN_GVW", &CConfigDataCore::Sim_Config::MIN_GVW);
		py::class_<CConfigDataCore::Output_Config> output_config(cconfigdatacore, "Output_Config");
			output_config.def_readwrite("WRITE_TIME_HISTORY", &CConfigDataCore::Output_Config::WRITE_TIME_HISTORY)
				.def_readwrite("WRITE_EACH_EVENT", &CConfigDataCore::Output_Config::WRITE_EACH_EVENT)
				.def_readwrite("WRITE_EVENT_BUFFER_SIZE", &CConfigDataCore::Output_Config::WRITE_EVENT_BUFFER_SIZE)
				.def_readwrite("WRITE_FATIGUE_EVENT", &CConfigDataCore::Output_Config::WRITE_FATIGUE_EVENT)
				.def_readwrite("VehicleFile", &CConfigDataCore::Output_Config::VehicleFile)
				.def_readwrite("BlockMax", &CConfigDataCore::Output_Config::BlockMax)
				.def_readwrite("POT", &CConfigDataCore::Output_Config::POT)
				.def_readwrite("Stats", &CConfigDataCore::Output_Config::Stats)
				.def_readwrite("Fatigue", &CConfigDataCore::Output_Config::Fatigue);
			py::class_<CConfigDataCore::Output_Config::VehicleFile_Config> vehiclefile_config(output_config, "VehicleFile_Config");
				vehiclefile_config.def_readwrite("WRITE_VEHICLE_FILE", &CConfigDataCore::Output_Config::VehicleFile_Config::WRITE_VEHICLE_FILE)
					.def_readwrite("FILE_FORMAT", &CConfigDataCore::Output_Config::VehicleFile_Config::FILE_FORMAT)
					.def_readwrite("VEHICLE_FILENAME", &CConfigDataCore::Output_Config::VehicleFile_Config::VEHICLE_FILENAME)
					.def_readwrite("WRITE_VEHICLE_BUFFER_SIZE", &CConfigDataCore::Output_Config::VehicleFile_Config::WRITE_VEHICLE_BUFFER_SIZE)
					.def_readwrite("WRITE_FLOW_STATS", &CConfigDataCore::Output_Config::VehicleFile_Config::WRITE_FLOW_STATS);
			py::class_<CConfigDataCore::Output_Config::BlockMax_Config> blockmax_config(output_config, "BlockMax_Config");
				blockmax_config.def_readwrite("WRITE_BM", &CConfigDataCore::Output_Config::BlockMax_Config::WRITE_BM)
					.def_readwrite("WRITE_BM_VEHICLES", &CConfigDataCore::Output_Config::BlockMax_Config::WRITE_BM_VEHICLES)
					.def_readwrite("WRITE_BM_SUMMARY", &CConfigDataCore::Output_Config::BlockMax_Config::WRITE_BM_SUMMARY)
					.def_readwrite("WRITE_BM_MIXED", &CConfigDataCore::Output_Config::BlockMax_Config::WRITE_BM_MIXED)
					.def_readwrite("BLOCK_SIZE_DAYS", &CConfigDataCore::Output_Config::BlockMax_Config::BLOCK_SIZE_DAYS)
					.def_readwrite("BLOCK_SIZE_SECS", &CConfigDataCore::Output_Config::BlockMax_Config::BLOCK_SIZE_SECS)
					.def_readwrite("WRITE_BM_BUFFER_SIZE", &CConfigDataCore::Output_Config::BlockMax_Config::WRITE_BM_BUFFER_SIZE);
			py::class_<CConfigDataCore::Output_Config::POT_Config> pot_config(output_config, "POT_Config");
				pot_config.def_readwrite("WRITE_POT", &CConfigDataCore::Output_Config::POT_Config::WRITE_POT)
					.def_readwrite("WRITE_POT_VEHICLES", &CConfigDataCore::Output_Config::POT_Config::WRITE_POT_VEHICLES)
					.def_readwrite("WRITE_POT_SUMMARY", &CConfigDataCore::Output_Config::POT_Config::WRITE_POT_SUMMARY)
					.def_readwrite("WRITE_POT_COUNTER", &CConfigDataCore::Output_Config::POT_Config::WRITE_POT_COUNTER)
					.def_readwrite("POT_COUNT_SIZE_DAYS", &CConfigDataCore::Output_Config::POT_Config::POT_COUNT_SIZE_DAYS)
					.def_readwrite("POT_COUNT_SIZE_SECS", &CConfigDataCore::Output_Config::POT_Config::POT_COUNT_SIZE_SECS)
					.def_readwrite("WRITE_POT_BUFFER_SIZE", &CConfigDataCore::Output_Config::POT_Config::WRITE_POT_BUFFER_SIZE);
			py::class_<CConfigDataCore::Output_Config::Stats_Config> stats_config(output_config, "Stats_Config");
				stats_config.def_readwrite("WRITE_STATS", &CConfigDataCore::Output_Config::Stats_Config::WRITE_STATS)
					.def_readwrite("WRITE_SS_CUMULATIVE", &CConfigDataCore::Output_Config::Stats_Config::WRITE_SS_CUMULATIVE)
					.def_readwrite("WRITE_SS_INTERVALS", &CConfigDataCore::Output_Config::Stats_Config::WRITE_SS_INTERVALS)
					.def_readwrite("WRITE_SS_INTERVAL_SIZE", &CConfigDataCore::Output_Config::Stats_Config::WRITE_SS_INTERVAL_SIZE)
					.def_readwrite("WRITE_SS_BUFFER_SIZE", &CConfigDataCore::Output_Config::Stats_Config::WRITE_SS_BUFFER_SIZE);
			py::class_<CConfigDataCore::Output_Config::Fatigue_Config> fatigue_config(output_config, "Fatigue_Config");
				fatigue_config.def_readwrite("DO_FATIGUE_RAINFLOW", &CConfigDataCore::Output_Config::Fatigue_Config::DO_FATIGUE_RAINFLOW)
					.def_readwrite("RAINFLOW_DECIMAL", &CConfigDataCore::Output_Config::Fatigue_Config::RAINFLOW_DECIMAL)
					.def_readwrite("RAINFLOW_CUTOFF", &CConfigDataCore::Output_Config::Fatigue_Config::RAINFLOW_CUTOFF)
					.def_readwrite("WRITE_RAINFLOW_BUFFER_SIZE", &CConfigDataCore::Output_Config::Fatigue_Config::WRITE_RAINFLOW_BUFFER_SIZE);
	py::class_<CVehicle, CVehicle_sp> cvehicle(m, "Vehicle");
		cvehicle.def(py::init<>())
			.def("is_car", &CVehicle::IsCar)
			.def("__lt__", &CVehicle::operator<, py::arg("another_vehicle"))
			.def("set_head", &CVehicle::setHead, py::arg("head"))
			.def("write", &CVehicle::Write, py::arg("file_type"))
			.def("create", &CVehicle::create, py::arg("str"), py::arg("format"))
			.def("set_time", &CVehicle::setTime, py::arg("time"))
			.def("set_length", &CVehicle::setLength, py::arg("length"))
			.def("set_velocity", &CVehicle::setVelocity, py::arg("velocity"))
			.def("set_local_lane", &CVehicle::setLocalLane, py::arg("lane_index"))
			.def("set_global_lane", &CVehicle::setGlobalLane, py::arg("lane_index"), py::arg("no_road_lanes"))
			.def("set_direction", &CVehicle::setDirection, py::arg("direction"))
			.def("set_gvw", &CVehicle::setGVW, py::arg("weight"))
			.def("set_no_axles", &CVehicle::setNoAxles, py::arg("no_axles"))
			.def("set_axle_weight", &CVehicle::setAW, py::arg("axle_index"), py::arg("weight"))
			.def("set_axle_spacing", &CVehicle::setAS, py::arg("axle_index"), py::arg("spacing"))
			.def("set_axle_width", &CVehicle::setAT, py::arg("axle_index"), py::arg("width"))
			.def("set_trans", &CVehicle::setTrns, py::arg("trans_position"))
			.def("set_bridge_times", &CVehicle::setBridgeTimes, py::arg("bridge_length"))
			.def("set_time_on_bridge", &CVehicle::setTimeOnBridge)
			.def("set_no_bridge_lanes", &CVehicle::setBridgeLaneNo, py::arg("mp_bridge_lanes"))
			.def("set_track_width", &CVehicle::setTrackWidth, py::arg("truck_width"))
			.def("set_lane_eccentricity", &CVehicle::setLaneEccentricity, py::arg("eccentricity"))
			.def("set_class", &CVehicle::setClass, py::arg("vehicle_class"))
			.def("get_time_str", &CVehicle::getTimeStr)
			.def("get_head", &CVehicle::getHead)
			.def("get_time", &CVehicle::getTime)
			.def("get_length", &CVehicle::getLength)
			.def("get_velocity", &CVehicle::getVelocity)
			.def("get_local_lane", &CVehicle::getLocalLane)
			.def("get_global_lane", &CVehicle::getGlobalLane, py::arg("no_road_lanes"))
			.def("get_direction", &CVehicle::getDirection)
			.def("get_gvw", &CVehicle::getGVW)
			.def("get_no_axles", &CVehicle::getNoAxles)
			.def("get_axle_weight", &CVehicle::getAW, py::arg("axle_index"))
			.def("get_axle_spacing", &CVehicle::getAS, py::arg("axle_index"))
			.def("get_axle_width", &CVehicle::getAT, py::arg("axle_index"))
			.def("get_trans", &CVehicle::getTrans)
			.def("get_time_on_bridge", &CVehicle::getTimeOnBridge)
			.def("get_time_off_bridge", &CVehicle::getTimeOffBridge)
			.def("is_on_bridge", &CVehicle::IsOnBridge, py::arg("at_time"))
			.def("get_no_bridge_lanes", &CVehicle::getBridgeLaneNo)
			.def("get_track_width", &CVehicle::getTrackWidth)
			.def("get_lane_eccentricity", &CVehicle::getLaneEccentricity)
			.def("get_class", &CVehicle::getClass);
	py::class_<Classification> classification(m, "Classification");
		classification.def(py::init<size_t, string>(), py::arg("index"), py::arg("desc"))
			.def(py::init<size_t,string, string>(), py::arg("index"), py::arg("str"), py::arg("desc"))
			.def("__eq__", &Classification::operator==, py::arg("another_classification"))
			.def("tie", &Classification::tie)
			.def_readwrite("m_ID", &Classification::m_ID)
			.def_readwrite("m_String", &Classification::m_String)
			.def_readwrite("m_Desc", &Classification::m_Desc);
	py::class_<CVehicleClassification, CVehicleClassification_sp> cvehicleclassification(m, "VehicleClassification");
	py::class_<CVehClassAxle, CVehicleClassification, CVehClassAxle_sp> cvehclassaxle(m, "VehClassAxle");
		cvehclassaxle.def(py::init<>())
			.def("set_class", &CVehClassAxle::setClassification, py::arg("vehicle"));
	py::class_<CVehClassPattern, CVehicleClassification, CVehClassPattern_sp> cvehclasspattern(m, "VehClassPattern");
		cvehclasspattern.def(py::init<>())
			.def("set_class", &CVehClassPattern::setClassification, py::arg("vehicle"));
	py::class_<CBridge, CBridge_sp> cbridge(m, "Bridge");
		cbridge.def(py::init<CConfigDataCore&>(), py::arg("config"))
			.def("set_calc_time_step", &CBridge::setCalcTimeStep, py::arg("calc_time_step"))
			.def("add_vehicle", &CBridge::AddVehicle, py::arg("vehicle"))
			.def("update", &CBridge::Update, py::arg("next_arrival_time"), py::arg("current_time"))
			.def("finish", &CBridge::Finish)
			.def("initialize_data_manager", &CBridge::InitializeDataMgr, py::arg("sim_start_time"));
	py::class_<CBridgeLane> cbridgelane(m, "BridgeLane");
	py::class_<CInfluenceLine> cinfluenceline(m, "InfluenceLine");
		cinfluenceline.def(py::init<>())
			.def("get_ordinate", &CInfluenceLine::getOrdinate, py::arg("x"))
			.def("get_no_points", &CInfluenceLine::getNoPoints)
			.def("get_load_ffect", &CInfluenceLine::getLoadEffect, py::arg("axle_list"))
			.def("get_type", &CInfluenceLine::getType, "return: 1-expression, 2-discrete, 3-Surface")
			.def("get_inf_surface", &CInfluenceLine::getIS)
			.def("set_inf_line", py::overload_cast<size_t, double>(&CInfluenceLine::setIL), py::arg("intergrated_inf_line_index"), py::arg("bridge_length"))
			.def("set_inf_line", py::overload_cast<vector<double>, vector<double> >(&CInfluenceLine::setIL), py::arg("h_ordinate_list"), py::arg("v_ordinate_list"))
			.def("set_inf_line", py::overload_cast<CInfluenceSurface>(&CInfluenceLine::setIL), py::arg("inf_surface"))
			.def("get_length", &CInfluenceLine::getLength)
			.def("set_index", &CInfluenceLine::setIndex, py::arg("index"))
			.def("get_index", &CInfluenceLine::getIndex)
			.def("set_weight", &CInfluenceLine::setWeight, py::arg("weight"));
	py::class_<CInfluenceSurface> cinfluencesurface(m, "InfluenceSurface");
		cinfluencesurface.def(py::init<>());
	py::class_<CAxle> caxle(m, "Axle");
	py::class_<CLane, CLane_sp> clane(m, "Lane");
	py::class_<CLaneFileTraffic, CLane, CLaneFileTraffic_sp> clanefiletraffic(m, "LaneFileTraffic");
		clanefiletraffic.def(py::init<>())
			.def("set_lane_data", &CLaneFileTraffic::setLaneData, py::arg("dirn"), py::arg("lane_index"))
			.def("add_vehicle", &CLaneFileTraffic::addVehicle, py::arg("vehicle"))
			.def("set_first_arrival_time", &CLaneFileTraffic::setFirstArrivalTime)
			.def("get_no_vehicles", &CLaneFileTraffic::GetNoVehicles)
			.def("get_direction", &CLaneFileTraffic::GetDirection)
			.def("get_next_vehicle", &CLaneFileTraffic::GetNextVehicle)
			.def("get_next_arrival_time", &CLaneFileTraffic::GetNextArrivalTime);
	py::class_<CLaneGenTraffic, CLane, CLaneGenTraffic_sp> clanegentraffic(m, "LaneGenTraffic");
		clanegentraffic.def(py::init<CConfigDataCore&>(), py::arg("config"))
			.def("set_lane_data", py::overload_cast<CVehicleClassification_sp, CLaneFlowComposition, const double>(&CLaneGenTraffic::setLaneData), py::arg("vehicle_classification"), py::arg("lane_flow_composition"), py::arg("start_time"))
			.def("get_next_vehicle", &CLaneGenTraffic::GetNextVehicle, py::return_value_policy::reference)  // return a shared_ptr reference like in PrepareSim.cpp
			.def("get_next_arrival_time", &CLaneGenTraffic::GetNextArrivalTime);
	py::class_<CVehicleBuffer> cvehiclebuffer(m, "VehicleBuffer");
		cvehiclebuffer.def(py::init<CConfigDataCore&, CVehicleClassification_sp, double>(), py::arg("config"), py::arg("vehicle_classification"), py::arg("start_time"))
			.def("add_vehicle", &CVehicleBuffer::AddVehicle, py::arg("vehicle"))
			.def("flush_buffer", &CVehicleBuffer::FlushBuffer);
	py::class_<CReadILFile> creadilfile(m, "ReadILFile");
		creadilfile.def(py::init<>())
			.def("get_inf_lines", py::overload_cast<filesystem::path, unsigned int>(&CReadILFile::getInfLines), py::arg("inf_file"), py::arg("line0_or_surface1"));
	py::class_<CCSVParse> ccsvparse(m, "CSVParse");
	py::class_<CBridgeFile> cbridgefile(m, "BridgeFile");
		cbridgefile.def(py::init<CConfigDataCore&, vector<CInfluenceLine>, vector<CInfluenceLine>>(), py::arg("config"), py::arg("inf_line"), py::arg("inf_surface"))
			.def("get_bridges", &CBridgeFile::getBridges)
			.def("get_max_bridge_length", &CBridgeFile::getMaxBridgeLength);
	py::class_<CLaneFlowComposition> claneflowcomposition(m, "LaneFlowComposition");
	py::class_<CBlockMaxEvent> cblockmaxevent(m, "BlockMaxEvent");
	py::class_<COutputManagerBase> coutputmanagerbase(m, "OutputManagerBase");
	py::class_<CBlockMaxManager, COutputManagerBase> cblockmaxmanager(m, "BlockMaxManager");
	py::class_<CPOTManager, COutputManagerBase> cpotmanager(m, "POTManager");
	py::class_<CFatigueManager, COutputManagerBase> cfatiguemanager(m, "FatigueManager");
	py::class_<CCalcEffect> ccalceffect(m, "CalcEffect");
	py::class_<CRNGWrapper> crngwrapper(m, "RNGWrapper");
	py::class_<CDistribution> cdistribution(m, "Distribution");
	py::class_<CEffect> ceffect(m, "Effect");
	py::class_<CEvent> cevent(m, "Event");
	py::class_<CEventBuffer> ceventbuffer(m, "EventBuffer");
	py::class_<CEventManager> ceventmanager(m, "EventManager");
	py::class_<CEventStatistics> ceventstatistics(m, "EventStatistics");
	py::class_<CGenerator, CGenerator_sp> cgenerator(m, "Generator");
	py::class_<CFlowGenerator, CGenerator, CFlowGenerator_sp> cflowgenerator(m, "FlowGenerator");
	py::class_<CFlowGenNHM, CFlowGenerator, CFlowGenNHM_sp> cflowgennhm(m, "FlowGenNHM");
	py::class_<CFlowGenCongested, CFlowGenerator, CFlowGenCongested_sp> cflowgencongested(m, "FlowGenCongested");
	py::class_<CFlowGenPoisson, CFlowGenerator, CFlowGenPoisson_sp> cflowgenpoisson(m, "FlowGenPoisson");
	py::class_<CFlowGenConstant, CFlowGenerator, CFlowGenConstant_sp> cflowgenconstant(m, "FlowGenConstant");
	py::class_<CModelData, CModelData_sp> cmodeldata(m, "ModelData");
	py::class_<CLaneFlowData, CModelData, CLaneFlowData_sp> claneflowdata(m, "LaneFlowData");
		claneflowdata.def(py::init<CConfigDataCore&>(), py::arg("config"))
			.def("read_data_in", &CLaneFlowData::ReadDataIn)
			.def("get_no_dirn", &CLaneFlowData::getNoDirn)
			.def("get_no_lanes", &CLaneFlowData::getNoLanes)
			.def("get_no_lanes_dir1", &CLaneFlowData::getNoLanesDir1)
			.def("get_no_lanes_dir2", &CLaneFlowData::getNoLanesDir2)
			.def("get_lane_composition", &CLaneFlowData::getLaneComp, py::arg("lane_index"));
	py::class_<CFlowModelData, CModelData, CFlowModelData_sp> cflowmodeldata(m, "FlowModelData");
	py::class_<CFlowModelDataNHM, CFlowModelData, CFlowModelDataNHM_sp> cflowmodeldatanhm(m, "FlowModelDataNHM");
	py::class_<CFlowModelDataCongested, CFlowModelData, CFlowModelDataCongested_sp> cflowmodeldatacongested(m, "FlowModelDataCongested");
	py::class_<CFlowModelDataPoisson, CFlowModelData, CFlowModelDataPoisson_sp> cflowmodeldatapoisson(m, "FlowModelDataPoisson");
	py::class_<CFlowModelDataConstant, CFlowModelData, CFlowModelDataConstant_sp> cflowmodeldataconstant(m, "FlowModelDataConstant");
	py::class_<CStatsManager> cstatsmanager(m, "StatsManager");
	py::class_<CTriModalNormal> ctrimodalnormal(m, "TriModalNormal");
	py::class_<CFlowRateData> cflowratedata(m, "FlowRateData");
	py::class_<CVehicleGenerator, CGenerator, CVehicleGenerator_sp> cvehiclegenerator(m, "VehicleGenerator");
	py::class_<CVehicleGenConstant, CVehicleGenerator, CVehicleGenConstant_sp> cvehiclegenconstant(m, "VehicleGenConstant");
		cvehiclegenconstant.def(py::init<CVehModelDataConstant_sp>(), py::arg("vehicle_model_data"))
			.def("generator", &CVehicleGenConstant::Generate, py::arg("lane_block"));
	py::class_<CVehicleGenGarage, CVehicleGenerator, CVehicleGenGarage_sp> cvehiclegengarage(m, "VehicleGenGarage");
		cvehiclegengarage.def(py::init<CVehModelDataGarage_sp>(), py::arg("vehicle_model_data"))
			.def("generator", &CVehicleGenGarage::Generate, py::arg("lane_block"));
	py::class_<CVehicleGenGrave, CVehicleGenerator, CVehicleGenGrave_sp> cvehiclegengrave(m, "VehicleGenGrave");
		cvehiclegengrave.def(py::init<CVehModelDataGrave_sp>(), py::arg("vehicle_model_data"))
			.def("generator", &CVehicleGenGrave::Generate, py::arg("lane_block"));
	py::class_<CVehicleModelData, CModelData, CVehicleModelData_sp> cvehiclemodeldata(m, "VehicleModelData");
	py::class_<CVehicleTrafficFile> cvehicletrafficfile(m, "VehicleTrafficFile");
		cvehicletrafficfile.def(py::init<CVehicleClassification_sp, bool, bool, double>(), py::arg("vehicle_classification"), py::arg("use_const_speed"), py::arg("use_ave_speed"), py::arg("const_speed_value"))
			.def("read", &CVehicleTrafficFile::Read, py::arg("file"), py::arg("format"))
			.def("get_no_days", &CVehicleTrafficFile::getNoDays)
			.def("get_no_lanes", &CVehicleTrafficFile::getNoLanes)
			.def("get_no_dirn", &CVehicleTrafficFile::getNoDirn)
			.def("get_no_lanes_dir1", &CVehicleTrafficFile::getNoLanesDir1)
			.def("get_no_lanes_dir2", &CVehicleTrafficFile::getNoLanesDir2)
			.def("get_no_vehicles", &CVehicleTrafficFile::getNoVehicles)
			.def("get_next_vehicle", &CVehicleTrafficFile::getNextVehicle)
			.def("get_vehicles", &CVehicleTrafficFile::getVehicles)
			.def("get_start_time", &CVehicleTrafficFile::getStartTime)
			.def("get_end_time", &CVehicleTrafficFile::getEndTime);
	py::class_<CVehModelDataConstant, CVehicleModelData, CVehModelDataConstant_sp> cvehmodeldataconstant(m, "VehModelDataConstant");
		cvehmodeldataconstant.def(py::init<CConfigDataCore&, CVehicleClassification_sp, CLaneFlowComposition>(), py::arg("config"), py::arg("vehicle_classification"), py::arg("lane_flow_composition"));
	py::class_<CVehModelDataGarage, CVehicleModelData, CVehModelDataGarage_sp> cvehmodeldatagarage(m, "VehModelDataGarage");
		cvehmodeldatagarage.def(py::init<CConfigDataCore&, CVehicleClassification_sp, CLaneFlowComposition>(), py::arg("config"), py::arg("vehicle_classification"), py::arg("lane_flow_composition"))
			.def("assign_garage", &CVehModelDataGarage::assignGarage, py::arg("vehicle_list"));
	py::class_<CVehModelDataGrave, CVehicleModelData, CVehModelDataGrave_sp> cvehmodeldatagrave(m, "VehModelDataGrave");
		cvehmodeldatagrave.def(py::init<CConfigDataCore&, CVehicleClassification_sp, CLaneFlowComposition>(), py::arg("config"), py::arg("vehicle_classification"), py::arg("lane_flow_composition"));
	py::class_<Normal> normal(m, "Normal");
	py::enum_<EFlowModel>(m, "EFlowModel").export_values();
	py::enum_<EVehicleModel>(m, "EVehicleModel").export_values();
};
