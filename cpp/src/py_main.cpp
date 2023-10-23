// py_main.cpp
// the main file for the PyBTLS Build

#include "PrepareSim.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl/filesystem.h"


namespace py = pybind11;


PYBIND11_MODULE(_core, m) {
	m.doc() = ("PyBTLS is for short-to-mid span bridge traffic loading simulation.");
	m.def("get_info", &preamble);
	m.def("run", &run, "Run the simulation.", py::arg("in_file")="./BTLSin.txt");
	py::class_<CConfigDataCore> cconfigdatacore(m, "_ConfigDataCore");
		cconfigdatacore.def(py::init<>())
			.def("_setVehGenGrave", &CConfigDataCore::setVehGenGrave, "Config for grave vehicle generation method.", py::arg("vehicle_classifier"), py::arg("traffic_folder"), py::arg("truck_track_width"), py::arg("lane_eccentricity_std"))
			.def("_setVehGenGarage", &CConfigDataCore::setVehGenGarage, "Config for garage vehicle generation method.", py::arg("vehicle_classifier"), py::arg("garage_file"), py::arg("file_format"), py::arg("kernel_file"), py::arg("lane_eccentricity_std"), py::arg("kernel_type"))
			.def("_setVehGenNominal", &CConfigDataCore::setVehGenNominal, "Config for nominal vehicle generation method.", py::arg("vehicle_classifier"), py::arg("nominal_file"), py::arg("lane_eccentricity_std"), py::arg("kernel_type"))
			.def("_setFlowGenNHM", &CConfigDataCore::setFlowGenNHM, "Config for NHM flow generation method.", py::arg("lanes_file"), py::arg("traffic_folder"), py::arg("no_days"))
			.def("_setFlowGenConstant", &CConfigDataCore::setFlowGenConstant, "Config for constant flow generation method.", py::arg("lanes_file"), py::arg("constant_speed"), py::arg("constant_gap"), py::arg("no_days"))
			.def("_setFlowGenCongested", &CConfigDataCore::setFlowGenCongested, "Config for congestion flow generation method.", py::arg("lanes_file"), py::arg("congested_spacing"), py::arg("congested_speed"), py::arg("congested_gap_coef_var"), py::arg("no_days"))
			.def("_setFlowGenFreeflow", &CConfigDataCore::setFlowGenFreeflow, "Config for freeflow flow generation method.", py::arg("lanes_file"), py::arg("no_days"))
			.def("set_event_output", &CConfigDataCore::setEventOutput, "Config for recording all load events to HDD. Parameters: write_time_history, write_each_event, buffer_size", py::arg("write_time_history")=false, py::arg("write_each_event")=false, py::arg("buffer_size")=10000)
			.def("set_vehicle_file_output", &CConfigDataCore::setVehicleFileOutput, "Config for recording the vehicles (in the generated traffic) to HDD. Parameters: write_vehicle_file, vehicle_file_format, vehicle_file_name, buffer_size.", py::arg("write_vehicle_file")=false, py::arg("vehicle_file_format")=4, py::arg("vehicle_file_name")="output_traffic.txt", py::arg("buffer_size")=10000)
			.def("set_BM_output", &CConfigDataCore::setBlockMaxOutput, "Config for conducting block-max analysis to load events and writing data to HDD. Parameters: write_vehicle, write_summary, write_mixed, block_size_days, block_size_secs, buffer_size.", py::arg("write_vehicle")=false, py::arg("write_summary")=false, py::arg("write_mixed")=false, py::arg("block_size_days")=1, py::arg("block_size_secs")=0, py::arg("buffer_size")=10000)
			.def("set_POT_output", &CConfigDataCore::setPOTOutput, "Config for conducting peak-over-threshold analysis to load events and writing data to HDD. Parameters: write_vehicle, write_summary, write_counter, pot_size_days, pot_size_secs, buffer_size.", py::arg("write_vehicle")=false, py::arg("write_summary")=false, py::arg("write_counter")=false, py::arg("pot_size_days")=1, py::arg("pot_size_secs")=0, py::arg("buffer_size")=10000)
			.def("set_stats_output", &CConfigDataCore::setStatsOutput, "Config for doing statistic to the flow and writing data to HDD. Parameters: write_flow_stats, write_overall, write_intervals, interval_size, buffer_size.", py::arg("write_flow_stats")=false, py::arg("write_overall")=false, py::arg("write_intervals")=false, py::arg("interval_size")=3600, py::arg("buffer_size")=10000)
			.def("set_fatigue_output", &CConfigDataCore::setFatigueOutput, "Config for recording fatigue event, doing rainflow count and writing data to HDD. Parameters: write_fatigue_event, write_rainflow_output, rainflow_decimal, rainflow_cut_off, buffer_size.", py::arg("write_fatigue_event")=false, py::arg("write_rainflow_output")=false, py::arg("rainflow_decimal")=1, py::arg("rainflow_cut_off")=0.0, py::arg("buffer_size")=10000)
			.def("_setRoadProperties", &CConfigDataCore::setRoadProperties, "set config.Road attribute.", py::arg("no_lane"), py::arg("no_dir"), py::arg("no_lanes_dir1"), py::arg("no_lanes_dir2"))
			.def_readwrite("_Traffic", &CConfigDataCore::Traffic)
			.def(py::pickle(
				[](CConfigDataCore& self) {  // __getstate__
					py::dict attribute_dict;

					py::dict road_dict;
					road_dict["NO_LANES_DIR1"] = self.Road.NO_LANES_DIR1;
					road_dict["NO_LANES_DIR2"] = self.Road.NO_LANES_DIR2;
					road_dict["NO_LANES"] = self.Road.NO_LANES;
					road_dict["NO_DIRS"] = self.Road.NO_DIRS;

					py::dict gen_dict;
					gen_dict["TRAFFIC_FOLDER"] = self.Gen.TRAFFIC_FOLDER;
					gen_dict["TRUCK_TRACK_WIDTH"] = self.Gen.TRUCK_TRACK_WIDTH;
					gen_dict["LANE_ECCENTRICITY_STD"] = self.Gen.LANE_ECCENTRICITY_STD;
					gen_dict["KERNEL_TYPE"] = self.Gen.KERNEL_TYPE;

					py::dict traffic_dict;
					traffic_dict["CLASSIFICATION"] = self.Traffic.CLASSIFICATION;
					traffic_dict["CONGESTED_SPACING"] = self.Traffic.CONGESTED_SPACING;
					traffic_dict["CONGESTED_SPEED"] = self.Traffic.CONGESTED_SPEED;
					traffic_dict["CONGESTED_GAP"] = self.Traffic.CONGESTED_GAP;
					traffic_dict["CONGESTED_GAP_COEF_VAR"] = self.Traffic.CONGESTED_GAP_COEF_VAR;
					traffic_dict["CONSTANT_SPEED"] = self.Traffic.CONSTANT_SPEED;
					traffic_dict["CONSTANT_GAP"] = self.Traffic.CONSTANT_GAP;

					py::dict output_dict;
					output_dict["WRITE_TIME_HISTORY"] = self.Output.WRITE_TIME_HISTORY;
					output_dict["WRITE_EACH_EVENT"] = self.Output.WRITE_EACH_EVENT;
					output_dict["WRITE_EVENT_BUFFER_SIZE"] = self.Output.WRITE_EVENT_BUFFER_SIZE;
					output_dict["WRITE_FATIGUE_EVENT"] = self.Output.WRITE_FATIGUE_EVENT;

					py::dict vehiclefile_dict;
					vehiclefile_dict["WRITE_VEHICLE_FILE"] = self.Output.VehicleFile.WRITE_VEHICLE_FILE;
					vehiclefile_dict["FILE_FORMAT"] = self.Output.VehicleFile.FILE_FORMAT;
					vehiclefile_dict["VEHICLE_FILENAME"] = self.Output.VehicleFile.VEHICLE_FILENAME;
					vehiclefile_dict["WRITE_VEHICLE_BUFFER_SIZE"] = self.Output.VehicleFile.WRITE_VEHICLE_BUFFER_SIZE;
					vehiclefile_dict["WRITE_FLOW_STATS"] = self.Output.VehicleFile.WRITE_FLOW_STATS;
					output_dict["VehicleFile"] = vehiclefile_dict;

					py::dict blockmax_dict;
					blockmax_dict["WRITE_BM"] = self.Output.BlockMax.WRITE_BM;
					blockmax_dict["WRITE_BM_VEHICLES"] = self.Output.BlockMax.WRITE_BM_VEHICLES;
					blockmax_dict["WRITE_BM_SUMMARY"] = self.Output.BlockMax.WRITE_BM_SUMMARY;
					blockmax_dict["WRITE_BM_MIXED"] = self.Output.BlockMax.WRITE_BM_MIXED;
					blockmax_dict["BLOCK_SIZE_DAYS"] = self.Output.BlockMax.BLOCK_SIZE_DAYS;
					blockmax_dict["BLOCK_SIZE_SECS"] = self.Output.BlockMax.BLOCK_SIZE_SECS;
					blockmax_dict["WRITE_BM_BUFFER_SIZE"] = self.Output.BlockMax.WRITE_BM_BUFFER_SIZE;
					output_dict["BlockMax"] = blockmax_dict;

					py::dict pot_dict;
					pot_dict["WRITE_POT"] = self.Output.POT.WRITE_POT;
					pot_dict["WRITE_POT_VEHICLES"] = self.Output.POT.WRITE_POT_VEHICLES;
					pot_dict["WRITE_POT_SUMMARY"] = self.Output.POT.WRITE_POT_SUMMARY;
					pot_dict["WRITE_POT_COUNTER"] = self.Output.POT.WRITE_POT_COUNTER;
					pot_dict["POT_COUNT_SIZE_DAYS"] = self.Output.POT.POT_COUNT_SIZE_DAYS;
					pot_dict["POT_COUNT_SIZE_SECS"] = self.Output.POT.POT_COUNT_SIZE_SECS;
					pot_dict["WRITE_POT_BUFFER_SIZE"] = self.Output.POT.WRITE_POT_BUFFER_SIZE;
					output_dict["POT"] = pot_dict;

					py::dict stats_dict;
					stats_dict["WRITE_STATS"] = self.Output.Stats.WRITE_STATS;
					stats_dict["WRITE_SS_CUMULATIVE"] = self.Output.Stats.WRITE_SS_CUMULATIVE;
					stats_dict["WRITE_SS_INTERVALS"] = self.Output.Stats.WRITE_SS_INTERVALS;
					stats_dict["WRITE_SS_INTERVAL_SIZE"] = self.Output.Stats.WRITE_SS_INTERVAL_SIZE;
					stats_dict["WRITE_SS_BUFFER_SIZE"] = self.Output.Stats.WRITE_SS_BUFFER_SIZE;
					output_dict["Stats"] = stats_dict;

					py::dict fatigue_dict;
					fatigue_dict["DO_FATIGUE_RAINFLOW"] = self.Output.Fatigue.DO_FATIGUE_RAINFLOW;
					fatigue_dict["RAINFLOW_DECIMAL"] = self.Output.Fatigue.RAINFLOW_DECIMAL;
					fatigue_dict["RAINFLOW_CUTOFF"] = self.Output.Fatigue.RAINFLOW_CUTOFF;
					fatigue_dict["WRITE_RAINFLOW_BUFFER_SIZE"] = self.Output.Fatigue.WRITE_RAINFLOW_BUFFER_SIZE;
					output_dict["Fatigue"] = fatigue_dict;

					attribute_dict["Road"] = road_dict;
					attribute_dict["Gen"] = gen_dict;
					attribute_dict["Traffic"] = traffic_dict;
					attribute_dict["Output"] = output_dict;

					return attribute_dict;
				},
				[](py::dict attribute_dict){  // __setstate__
					CConfigDataCore config;

					config.Road.NO_LANES_DIR1 = attribute_dict["Road"]["NO_LANES_DIR1"].cast<size_t>();
					config.Road.NO_LANES_DIR2 = attribute_dict["Road"]["NO_LANES_DIR2"].cast<size_t>();
					config.Road.NO_LANES = attribute_dict["Road"]["NO_LANES"].cast<size_t>();
					config.Road.NO_DIRS = attribute_dict["Road"]["NO_DIRS"].cast<size_t>();

					config.Gen.TRAFFIC_FOLDER = attribute_dict["Gen"]["TRAFFIC_FOLDER"].cast<std::string>();
					config.Gen.TRUCK_TRACK_WIDTH = attribute_dict["Gen"]["TRUCK_TRACK_WIDTH"].cast<double>();
					config.Gen.LANE_ECCENTRICITY_STD = attribute_dict["Gen"]["LANE_ECCENTRICITY_STD"].cast<double>();
					config.Gen.KERNEL_TYPE = attribute_dict["Gen"]["KERNEL_TYPE"].cast<int>();

					config.Traffic.CLASSIFICATION = attribute_dict["Traffic"]["CLASSIFICATION"].cast<int>();
					config.Traffic.CONGESTED_SPACING = attribute_dict["Traffic"]["CONGESTED_SPACING"].cast<double>();
					config.Traffic.CONGESTED_SPEED = attribute_dict["Traffic"]["CONGESTED_SPEED"].cast<double>();
					config.Traffic.CONGESTED_GAP = attribute_dict["Traffic"]["CONGESTED_GAP"].cast<double>();
					config.Traffic.CONGESTED_GAP_COEF_VAR = attribute_dict["Traffic"]["CONGESTED_GAP_COEF_VAR"].cast<double>();
					config.Traffic.CONSTANT_SPEED = attribute_dict["Traffic"]["CONSTANT_SPEED"].cast<double>();
					config.Traffic.CONSTANT_GAP = attribute_dict["Traffic"]["CONSTANT_GAP"].cast<double>();

					config.Output.WRITE_TIME_HISTORY = attribute_dict["Output"]["WRITE_TIME_HISTORY"].cast<bool>();
					config.Output.WRITE_EACH_EVENT = attribute_dict["Output"]["WRITE_EACH_EVENT"].cast<bool>();
					config.Output.WRITE_EVENT_BUFFER_SIZE = attribute_dict["Output"]["WRITE_EVENT_BUFFER_SIZE"].cast<size_t>();
					config.Output.WRITE_FATIGUE_EVENT = attribute_dict["Output"]["WRITE_FATIGUE_EVENT"].cast<bool>();

					config.Output.VehicleFile.WRITE_VEHICLE_FILE = attribute_dict["Output"]["VehicleFile"]["WRITE_VEHICLE_FILE"].cast<bool>();
					config.Output.VehicleFile.FILE_FORMAT = attribute_dict["Output"]["VehicleFile"]["FILE_FORMAT"].cast<size_t>();
					config.Output.VehicleFile.VEHICLE_FILENAME = attribute_dict["Output"]["VehicleFile"]["VEHICLE_FILENAME"].cast<std::string>();
					config.Output.VehicleFile.WRITE_VEHICLE_BUFFER_SIZE = attribute_dict["Output"]["VehicleFile"]["WRITE_VEHICLE_BUFFER_SIZE"].cast<size_t>();
					config.Output.VehicleFile.WRITE_FLOW_STATS = attribute_dict["Output"]["VehicleFile"]["WRITE_FLOW_STATS"].cast<bool>();

					config.Output.BlockMax.WRITE_BM = attribute_dict["Output"]["BlockMax"]["WRITE_BM"].cast<bool>();
					config.Output.BlockMax.WRITE_BM_VEHICLES = attribute_dict["Output"]["BlockMax"]["WRITE_BM_VEHICLES"].cast<bool>();
					config.Output.BlockMax.WRITE_BM_SUMMARY = attribute_dict["Output"]["BlockMax"]["WRITE_BM_SUMMARY"].cast<bool>();
					config.Output.BlockMax.WRITE_BM_MIXED = attribute_dict["Output"]["BlockMax"]["WRITE_BM_MIXED"].cast<bool>();
					config.Output.BlockMax.BLOCK_SIZE_DAYS = attribute_dict["Output"]["BlockMax"]["BLOCK_SIZE_DAYS"].cast<size_t>();
					config.Output.BlockMax.BLOCK_SIZE_SECS = attribute_dict["Output"]["BlockMax"]["BLOCK_SIZE_SECS"].cast<size_t>();
					config.Output.BlockMax.WRITE_BM_BUFFER_SIZE = attribute_dict["Output"]["BlockMax"]["WRITE_BM_BUFFER_SIZE"].cast<size_t>();

					config.Output.POT.WRITE_POT = attribute_dict["Output"]["POT"]["WRITE_POT"].cast<bool>();
					config.Output.POT.WRITE_POT_VEHICLES = attribute_dict["Output"]["POT"]["WRITE_POT_VEHICLES"].cast<bool>();
					config.Output.POT.WRITE_POT_SUMMARY = attribute_dict["Output"]["POT"]["WRITE_POT_SUMMARY"].cast<bool>();
					config.Output.POT.WRITE_POT_COUNTER = attribute_dict["Output"]["POT"]["WRITE_POT_COUNTER"].cast<bool>();
					config.Output.POT.POT_COUNT_SIZE_DAYS = attribute_dict["Output"]["POT"]["POT_COUNT_SIZE_DAYS"].cast<size_t>();
					config.Output.POT.POT_COUNT_SIZE_SECS = attribute_dict["Output"]["POT"]["POT_COUNT_SIZE_SECS"].cast<size_t>();
					config.Output.POT.WRITE_POT_BUFFER_SIZE = attribute_dict["Output"]["POT"]["WRITE_POT_BUFFER_SIZE"].cast<size_t>();

					config.Output.Stats.WRITE_STATS = attribute_dict["Output"]["Stats"]["WRITE_STATS"].cast<bool>();
					config.Output.Stats.WRITE_SS_CUMULATIVE = attribute_dict["Output"]["Stats"]["WRITE_SS_CUMULATIVE"].cast<bool>();
					config.Output.Stats.WRITE_SS_INTERVALS = attribute_dict["Output"]["Stats"]["WRITE_SS_INTERVALS"].cast<bool>();
					config.Output.Stats.WRITE_SS_INTERVAL_SIZE = attribute_dict["Output"]["Stats"]["WRITE_SS_INTERVAL_SIZE"].cast<size_t>();
					config.Output.Stats.WRITE_SS_BUFFER_SIZE = attribute_dict["Output"]["Stats"]["WRITE_SS_BUFFER_SIZE"].cast<size_t>();

					config.Output.Fatigue.DO_FATIGUE_RAINFLOW = attribute_dict["Output"]["Fatigue"]["DO_FATIGUE_RAINFLOW"].cast<bool>();
					config.Output.Fatigue.RAINFLOW_DECIMAL = attribute_dict["Output"]["Fatigue"]["RAINFLOW_DECIMAL"].cast<int>();
					config.Output.Fatigue.RAINFLOW_CUTOFF = attribute_dict["Output"]["Fatigue"]["RAINFLOW_CUTOFF"].cast<double>();
					config.Output.Fatigue.WRITE_RAINFLOW_BUFFER_SIZE = attribute_dict["Output"]["Fatigue"]["WRITE_RAINFLOW_BUFFER_SIZE"].cast<size_t>();

					return config;
				}
			));
		py::class_<CConfigDataCore::Traffic_Config> traffic_config(cconfigdatacore, "_Traffic_Config");
			traffic_config.def_readwrite("CLASSIFICATION", &CConfigDataCore::Traffic_Config::CLASSIFICATION);


	py::class_<CInfluenceLine> cinfluenceline(m, "_InfluenceLine");
		cinfluenceline.def(py::init<>())
			.def("setIndex", &CInfluenceLine::setIndex, py::arg("IL_index"))
			.def("setIL", py::overload_cast<size_t, double>(&CInfluenceLine::setIL), py::arg("built_in_IL_no"), py::arg("length"))
			.def("setIL", py::overload_cast<std::vector<double>, std::vector<double> >(&CInfluenceLine::setIL), py::arg("positions"), py::arg("ordinates"))
			.def("setIL", py::overload_cast<CInfluenceSurface>(&CInfluenceLine::setIL), py::arg("inf_surface"))
			.def("getLength", &CInfluenceLine::getLength);
	py::class_<CInfluenceSurface> cinfluencesurface(m, "_InfluenceSurface");
		cinfluencesurface.def(py::init<>())
			.def("setLanes", py::overload_cast<std::vector<std::pair<double,double>>>(&CInfluenceSurface::setLanes), py::arg("lane_positions"))
			.def("setIS", &CInfluenceSurface::setIS, py::arg("IS_matrix"));

	py::class_<CBridge, CBridge_sp> cbridge(m, "_Bridge");
		cbridge.def(py::init<CConfigDataCore&>(), py::arg("config"))
			.def("setIndex", &CBridge::setIndex, py::arg("index"))
			.def("setLength", &CBridge::setLength, "in metre.", py::arg("length"))
			.def("setNoLoadEffects", &CBridge::setNoLoadEffects, py::arg("no_LE"))
			.def("initializeLanes", &CBridge::InitializeLanes, py::arg("no_lane"))
			.def("getBridgeLane", &CBridge::getBridgeLane, py::arg("lane_index"), py::return_value_policy::reference)
			.def("setThresholds", &CBridge::setThresholds, py::arg("threshold_list"))
			.def("addVehicle", &CBridge::AddVehicle, py::arg("vehicle"))
			.def("setCalcTimeStep", &CBridge::setCalcTimeStep, py::arg("time_step"))
			.def("update", &CBridge::Update, py::arg("next_arrival_time"), py::arg("current_time"))
			.def("finish", &CBridge::Finish)
			.def("initializeDataMgr", &CBridge::InitializeDataMgr, py::arg("sim_start_time"));
	py::class_<CBridgeLane> cbridgelane(m, "_BridgeLane");
		cbridgelane.def("addLoadEffect", &CBridgeLane::addLoadEffect, py::arg("IL"), py::arg("weight"));


	py::class_<CVehicle, CVehicle_sp> cvehicle(m, "_Vehicle");
		cvehicle.def(py::init<>())
			.def("set_velocity", &CVehicle::setVelocity, "Set the velocity (dm/s) of the vehicle.", py::arg("velocity"))
			.def("set_local_from_global_lane", &CVehicle::setLocalFromGlobalLane, "Set the local lane index of the vehicle from its 1-based global index.", py::arg("global_lane_index"), py::arg("no_lanes"))
			.def("set_direction", &CVehicle::setDirection, "Set the direction of the vehicle.", py::arg("direction"))
			.def("set_time", &CVehicle::setTime, "Set the showing time of the vehicle.", py::arg("time"))
			.def("_setLocalLane", &CVehicle::setLocalLane, "Set the local lane index of the vehicle.", py::arg("local_lane_index"))
			.def("_setGVW", &CVehicle::setGVW, "Set the gross vehicle weight of the vehicle.", py::arg("weight"))
			.def("_setLength", &CVehicle::setLength, "Set the length of the vehicle.", py::arg("length"))
			.def("_setNoAxles", &CVehicle::setNoAxles, "Set the number of axles of the vehicle.", py::arg("no_axles"))
			.def("_setAxleWeight", &CVehicle::setAW, "Set the weight of the specified axle.", py::arg("axle_index"), py::arg("weight"))
			.def("_setAxleSpacing", &CVehicle::setAS, "Set the value of the specified axle spacing.", py::arg("axle_index"), py::arg("spacing"))
			.def("_setAxleWidth", &CVehicle::setAT, "Set the width of the specified axle.", py::arg("axle_index"), py::arg("width"))
			.def("get_length", &CVehicle::getLength, "Get the length of the vehicle.")
			.def("get_velocity", &CVehicle::getVelocity, "Get the velocity of the vehicle.")
			.def("get_gvw", &CVehicle::getGVW, "Get the gross vehicle weight of the vehicle.")
			.def("get_no_axles", &CVehicle::getNoAxles, "Get the number of axles of the vehicle.")
			.def("get_axle_weight", &CVehicle::getAW, "Get the weight of the specified axle.", py::arg("axle_index"))
			.def("get_axle_spacing", &CVehicle::getAS, "Get the value of the specified axle spacing.", py::arg("axle_index"))
			.def("get_axle_width", &CVehicle::getAT, "Get the width of the specified axle.", py::arg("axle_index"))
			.def("get_time", &CVehicle::getTime, "Get the showing time of the vehicle.")
			.def("_getGlobalLane", &CVehicle::getGlobalLane, "Get the 1-based global lane index of the vehicle.", py::arg("no_lanes"))
			.def(py::pickle(
				[](CVehicle& self) {  // __getstate__
					
					std::string vehInfo = self.Write(4);  // Use MON format

					return vehInfo;
				},
				[](std::string vehInfo) {  // __setstate__
					CVehicle_sp vehicle = std::make_shared<CVehicle>();

					vehicle->create(vehInfo, 4);  // Use MON format

					return vehicle;
				}
			));


	py::class_<CVehicleClassification, CVehicleClassification_sp> cvehicleclassification(m, "_VehicleClassification");
	py::class_<CVehClassAxle, CVehicleClassification, CVehClassAxle_sp> cvehclassaxle(m, "_VehClassAxle");
		cvehclassaxle.def(py::init<>());
	py::class_<CVehClassPattern, CVehicleClassification, CVehClassPattern_sp> cvehclasspattern(m, "_VehClassPattern");
		cvehclasspattern.def(py::init<>());


	py::class_<CLane, CLane_sp> clane(m, "_Lane");
		clane.def("getNextArrivalTime", &CLane::GetNextArrivalTime);
	py::class_<CLaneFileTraffic, CLane, CLaneFileTraffic_sp> clanefiletraffic(m, "_TrafficLoader");
		clanefiletraffic.def(py::init<>())
			.def("setLaneData", &CLaneFileTraffic::setLaneData, py::arg("dirn"), py::arg("lane_index"))
			.def("addVehicle", &CLaneFileTraffic::addVehicle, py::arg("vehicle"))
			.def("setFirstArrivalTime", &CLaneFileTraffic::setFirstArrivalTime)
			.def("getNoVehicles", &CLaneFileTraffic::GetNoVehicles)
			.def("getNextVehicle", &CLaneFileTraffic::GetNextVehicle, py::return_value_policy::take_ownership);
	py::class_<CLaneGenTraffic, CLane, CLaneGenTraffic_sp> clanegentraffic(m, "_TrafficGenerator");
		clanegentraffic.def(py::init<CConfigDataCore&>(), py::arg("config"))
			.def("setLaneData", py::overload_cast<CLaneFlowComposition, CVehicleGenerator_sp, CFlowGenerator_sp, const double>(&CLaneGenTraffic::setLaneData), py::arg("lane_flow_composition"), py::arg("vehicle_generator"), py::arg("headway_generator"), py::arg("start_time"))
			.def("initLane", &CLaneGenTraffic::initLane, py::arg("flow_model_data"))
			.def("getNextVehicle", &CLaneGenTraffic::GetNextVehicle, py::return_value_policy::take_ownership);


	py::class_<CLaneFlowComposition> claneflowcomposition(m, "_LaneFlowComposition");
		claneflowcomposition.def(py::init<size_t, size_t, size_t>(), py::arg("lane_index"), py::arg("lane_dir"), py::arg("block_size"))
			.def("addBlockData", &CLaneFlowComposition::addBlockData, py::arg("hourly_data"))
			.def("completeData", &CLaneFlowComposition::completeData);

	py::class_<CGenerator, CGenerator_sp> cgenerator(m, "_Generator");
	py::class_<CModelData, CModelData_sp> cmodeldata(m, "_ModelData");

	py::class_<CVehicleGenerator, CGenerator, CVehicleGenerator_sp> cvehiclegenerator(m, "_VehicleGenerator");
	py::class_<CVehicleGenNominal, CVehicleGenerator, CVehicleGenNominal_sp> cvehiclegennominal(m, "_VehicleGenNominal");
		cvehiclegennominal.def(py::init<CVehModelDataNominal_sp>(), py::arg("model_data"));
	py::class_<CVehicleGenGarage, CVehicleGenerator, CVehicleGenGarage_sp> cvehiclegengarage(m, "_VehicleGenGarage");
		cvehiclegengarage.def(py::init<CVehModelDataGarage_sp>(), py::arg("model_data"));
	py::class_<CVehicleGenGrave, CVehicleGenerator, CVehicleGenGrave_sp> cvehiclegengrave(m, "_VehicleGenGrave");
		cvehiclegengrave.def(py::init<CVehModelDataGrave_sp>(), py::arg("model_data"));

	py::class_<CVehicleModelData, CModelData, CVehicleModelData_sp> cvehiclemodeldata(m, "_VehicleModelData");
	py::class_<CVehModelDataNominal, CVehicleModelData, CVehModelDataNominal_sp> cvehmodeldatanominal(m, "_VehModelDataNominal");
		cvehmodeldatanominal.def(py::init<CConfigDataCore&, CVehicleClassification_sp, CLaneFlowComposition, CVehicle_sp, std::vector<double>>(), py::arg("config"), py::arg("vehicle_classifier"), py::arg("lane_flow_composition"), py::arg("nominal_vehicle"), py::arg("COV_list"));
	py::class_<CVehModelDataGarage, CVehicleModelData, CVehModelDataGarage_sp> cvehmodeldatagarage(m, "_VehModelDataGarage");
		cvehmodeldatagarage.def(py::init<CConfigDataCore&, CVehicleClassification_sp, CLaneFlowComposition, std::vector<CVehicle_sp>, std::vector<std::vector<double>>>(), py::arg("config"), py::arg("vehicle_classifier"), py::arg("lane_flow_composition"), py::arg("garage"), py::arg("kernel"));
	py::class_<CVehModelDataGrave, CVehicleModelData, CVehModelDataGrave_sp> cvehmodeldatagrave(m, "_VehModelDataGrave");
		cvehmodeldatagrave.def(py::init<CConfigDataCore&, CVehicleClassification_sp, CLaneFlowComposition>(), py::arg("config"), py::arg("vehicle_classifier"), py::arg("lane_flow_composition"));

	py::class_<CFlowGenerator, CGenerator, CFlowGenerator_sp> cflowgenerator(m, "_FlowGenerator");
		cflowgenerator.def("_setMaxBridgeLength", &CFlowGenNHM::setMaxBridgeLength, py::arg("max_bridge_length"));
	py::class_<CFlowGenNHM, CFlowGenerator, CFlowGenNHM_sp> cflowgennhm(m, "_FlowGenNHM");
		cflowgennhm.def(py::init<CFlowModelDataNHM_sp>(), py::arg("model_data"));
	py::class_<CFlowGenCongested, CFlowGenerator, CFlowGenCongested_sp> cflowgencongested(m, "_FlowGenCongested");
		cflowgencongested.def(py::init<CFlowModelDataCongested_sp>(), py::arg("model_data"));
	py::class_<CFlowGenPoisson, CFlowGenerator, CFlowGenPoisson_sp> cflowgenpoisson(m, "_FlowGenPoisson");
		cflowgenpoisson.def(py::init<CFlowModelDataPoisson_sp>(), py::arg("model_data"));
	py::class_<CFlowGenConstant, CFlowGenerator, CFlowGenConstant_sp> cflowgenconstant(m, "_FlowGenConstant");
		cflowgenconstant.def(py::init<CFlowModelDataConstant_sp>(), py::arg("model_data"));

	py::class_<CFlowModelData, CModelData, CFlowModelData_sp> cflowmodeldata(m, "_FlowModelData");
	py::class_<CFlowModelDataNHM, CFlowModelData, CFlowModelDataNHM_sp> cflowmodeldatanhm(m, "_FlowModelDataNHM");
		cflowmodeldatanhm.def(py::init<CConfigDataCore&, CLaneFlowComposition>(), py::arg("config"), py::arg("lane_flow_composition"));
	py::class_<CFlowModelDataCongested, CFlowModelData, CFlowModelDataCongested_sp> cflowmodeldatacongested(m, "_FlowModelDataCongested");
		cflowmodeldatacongested.def(py::init<CConfigDataCore&, CLaneFlowComposition>(), py::arg("config"), py::arg("lane_flow_composition"));
	py::class_<CFlowModelDataPoisson, CFlowModelData, CFlowModelDataPoisson_sp> cflowmodeldatapoisson(m, "_FlowModelDataPoisson");
		cflowmodeldatapoisson.def(py::init<CConfigDataCore&, CLaneFlowComposition>(), py::arg("config"), py::arg("lane_flow_composition"));
	py::class_<CFlowModelDataConstant, CFlowModelData, CFlowModelDataConstant_sp> cflowmodeldataconstant(m, "_FlowModelDataConstant");
		cflowmodeldataconstant.def(py::init<CConfigDataCore&, CLaneFlowComposition>(), py::arg("config"), py::arg("lane_flow_composition"));


	py::class_<CVehicleTrafficFile> cvehicletrafficfile(m, "_VehicleTrafficFile");
		cvehicletrafficfile.def(py::init<CVehicleClassification_sp, bool, bool, double>(), py::arg("vehicle_classification"), py::arg("use_const_speed"), py::arg("use_average_speed"), py::arg("const_speed_value"))
			.def("read", &CVehicleTrafficFile::Read, py::arg("file"), py::arg("format"))
			.def("assignTraffic", &CVehicleTrafficFile::AssignTraffic, py::arg("vehicle_list"))
			.def("getNoDays", &CVehicleTrafficFile::getNoDays)
			.def("getNoLanes", &CVehicleTrafficFile::getNoLanes)
			.def("getNoDirn", &CVehicleTrafficFile::getNoDirn)
			.def("getNoLanesDir1", &CVehicleTrafficFile::getNoLanesDir1)
			.def("getNoLanesDir2", &CVehicleTrafficFile::getNoLanesDir2)
			.def("getNoVehicles", &CVehicleTrafficFile::getNoVehicles)
			.def("getNextVehicle", &CVehicleTrafficFile::getNextVehicle, py::return_value_policy::take_ownership)
			.def("getVehicles", &CVehicleTrafficFile::getVehicles, py::return_value_policy::take_ownership);

	py::class_<CVehicleBuffer> cvehiclebuffer(m, "_VehicleBuffer");
		cvehiclebuffer.def(py::init<CConfigDataCore&, CVehicleClassification_sp, double>(), py::arg("config"), py::arg("vehicle_classifier"), py::arg("start_time"))
			.def("addVehicle", &CVehicleBuffer::AddVehicle, py::arg("vehicle"))
			.def("flushBuffer", &CVehicleBuffer::FlushBuffer);


	py::class_<CMultiModalNormal> cmultimodalnormal(m, "_MultiModalNormal");
		cmultimodalnormal.def(py::init<>())
			.def("add_mode", &CMultiModalNormal::AddMode, py::arg("w"), py::arg("m"), py::arg("s"))
			.def("get_no_modes", &CMultiModalNormal::getNoModes);
	py::class_<CDistribution> cdistribution(m, "_Distribution");
		cdistribution.def(py::init<>())
			.def(py::init<double, double, double>(), py::arg("w"), py::arg("m"), py::arg("s"))
			.def("set_shape", &CDistribution::setShape, py::arg("shape"))
			.def("set_scale", &CDistribution::setScale, py::arg("scale"))
			.def("set_location", &CDistribution::setLocation, py::arg("loc"))
			.def("get_shape", &CDistribution::getShape)
			.def("get_scale", &CDistribution::getScale)
			.def("get_location", &CDistribution::getLocation)
			.def("gen_uniform", &CDistribution::GenerateUniform)
			.def("gen_normal", py::overload_cast<>(&CDistribution::GenerateNormal))
			.def("gen_normal", py::overload_cast<double,double>(&CDistribution::GenerateNormal), py::arg("mean"), py::arg("stdev"))
			.def("gen_multimodalnormal", &CDistribution::GenerateMultiModalNormal, py::arg("mmn"))
			.def("gen_exponential", &CDistribution::GenerateExponential)
			.def("gen_lognormal", &CDistribution::GenerateLogNormal)
			.def("gen_gamma", &CDistribution::GenerateGamma)
			.def("gen_gumbel", &CDistribution::GenerateGumbel)
			.def("gen_poisson", &CDistribution::GeneratePoisson)
			.def("gen_gev", &CDistribution::GenerateGEV)
			.def("gen_triangular", py::overload_cast<>(&CDistribution::GenerateTriangular))
			.def("gen_triangular", py::overload_cast<double,double>(&CDistribution::GenerateTriangular), py::arg("mean"), py::arg("stdev"));
};

