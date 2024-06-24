// py_main.cpp
// the main file for the PyBTLS Build

#include "PrepareSim.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl/filesystem.h"


namespace py = pybind11;


PYBIND11_MODULE(libbtls, m) {
	m.def("get_info", &preamble);
	m.def("run", &run, "Run the simulation.", py::arg("BTLSin_file"));
	py::class_<CConfigDataCore> cconfigdatacore(m, "_ConfigDataCore");
		cconfigdatacore.def(py::init<>())
			.def_readwrite("_Road", &CConfigDataCore::Road)
			.def_readwrite("_Gen", &CConfigDataCore::Gen)
			.def_readwrite("_Traffic", &CConfigDataCore::Traffic)
			.def_readwrite("_Output", &CConfigDataCore::Output)
			.def("_setRoad", &CConfigDataCore::setRoad, "set CConfigDataCore.Road attribute.", py::arg("no_lane"), py::arg("no_dir"), py::arg("no_lanes_dir1"), py::arg("no_lanes_dir2"))
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
		py::class_<CConfigDataCore::Road_Config> road_config(cconfigdatacore, "_Road_Config");
			road_config.def_readwrite("LANES_FILE", &CConfigDataCore::Road_Config::LANES_FILE)
				.def_readwrite("NO_LANES_DIR1", &CConfigDataCore::Road_Config::NO_LANES_DIR1)
				.def_readwrite("NO_LANES_DIR2", &CConfigDataCore::Road_Config::NO_LANES_DIR2)
				.def_readwrite("NO_LANES", &CConfigDataCore::Road_Config::NO_LANES)
				.def_readwrite("NO_DIRS", &CConfigDataCore::Road_Config::NO_DIRS);
		py::class_<CConfigDataCore::Gen_Config> gen_config(cconfigdatacore, "_Gen_Config");
			gen_config.def_readwrite("TRAFFIC_FOLDER", &CConfigDataCore::Gen_Config::TRAFFIC_FOLDER)
				.def_readwrite("GEN_TRAFFIC", &CConfigDataCore::Gen_Config::GEN_TRAFFIC)
				.def_readwrite("NO_DAYS", &CConfigDataCore::Gen_Config::NO_DAYS)
				.def_readwrite("TRUCK_TRACK_WIDTH", &CConfigDataCore::Gen_Config::TRUCK_TRACK_WIDTH)
				.def_readwrite("LANE_ECCENTRICITY_STD", &CConfigDataCore::Gen_Config::LANE_ECCENTRICITY_STD)
				.def_readwrite("KERNEL_TYPE", &CConfigDataCore::Gen_Config::KERNEL_TYPE)
				.def_readwrite("NO_OVERLAP_LENGTH", &CConfigDataCore::Gen_Config::NO_OVERLAP_LENGTH);
		py::class_<CConfigDataCore::Traffic_Config> traffic_config(cconfigdatacore, "_Traffic_Config");
			traffic_config.def_readwrite("CLASSIFICATION", &CConfigDataCore::Traffic_Config::CLASSIFICATION)
				.def_readwrite("VEHICLE_MODEL", &CConfigDataCore::Traffic_Config::VEHICLE_MODEL)
				.def_readwrite("HEADWAY_MODEL", &CConfigDataCore::Traffic_Config::HEADWAY_MODEL)
				.def_readwrite("CONGESTED_SPACING", &CConfigDataCore::Traffic_Config::CONGESTED_SPACING)
				.def_readwrite("CONGESTED_SPEED", &CConfigDataCore::Traffic_Config::CONGESTED_SPEED)
				.def_readwrite("CONGESTED_GAP", &CConfigDataCore::Traffic_Config::CONGESTED_GAP)
				.def_readwrite("CONGESTED_GAP_COEF_VAR", &CConfigDataCore::Traffic_Config::CONGESTED_GAP_COEF_VAR)
				.def_readwrite("CONSTANT_SPEED", &CConfigDataCore::Traffic_Config::CONSTANT_SPEED)
				.def_readwrite("CONSTANT_GAP", &CConfigDataCore::Traffic_Config::CONSTANT_GAP);
		py::class_<CConfigDataCore::Output_Config> output_config(cconfigdatacore, "_Output_Config");
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
		cvehicle.def(py::init<size_t>(), 
				R"(
				The Vehicle class is inherited from the CVehicle class in the C++ BTLS library. \n
				It is used to create a customized vehicle object. 

				Parameters
				----------
				no_axles : int\n
					Number of axles.
				)", 
				py::arg("no_axles"))
			.def("set_velocity", &CVehicle::setVelocity, 
				R"(
				Set vehicle velocity.

				Parameters
				----------
				velocity : float\n
					The velocity of the vehicle, in m/s.
				)", 
				py::arg("velocity"))
			.def("set_local_from_global_lane", &CVehicle::setLocalFromGlobalLane, 
				R"(
				Set the local lane index of the vehicle from its 1-based global index.\n
				This method needs to be called after the vehicle direction is set. 

				Parameters
				----------
				global_lane_index : int\n
					The 1-based global lane index of the vehicle.\n
				no_lanes : int\n
					The number of lanes in the direction of the vehicle.
				)", 
				py::arg("global_lane_index"), py::arg("no_lanes"))
			.def("set_direction", &CVehicle::setDirection, 
				R"(
				Set the direction of the vehicle.

				Parameters
				----------
				direction : Literal[1,2]\n
					The direction of the vehicle. 
				)", 
				py::arg("direction"))
			.def("set_time", &CVehicle::setTime, 
				R"(
				Set the show-up time of the vehicle.

				Parameters
				----------
				time : float\n
					The showing time of the vehicle, in seconds.
				)", 
				py::arg("time"))
			.def("set_trans", &CVehicle::setTrans, 
				R"(
				Set the vehicle transverse position.

				Parameters
				----------
				trans : float\n
					The transverse position of the vehicle on its lane, in metres.
				)", 
				py::arg("trans"))
			.def("_setHead", &CVehicle::setHead, 
				"Set the head id of the vehicle.", 
				py::arg("head"))
			.def("set_local_lane", &CVehicle::setLocalLane, 
				R"(
				Set the local lane index of the vehicle.

				Parameters
				----------
				local_lane_index : int\n
					The 1-based local lane index of the vehicle.
				)", 
				py::arg("local_lane_index"))
			.def("set_axle_weights", 
				[](CVehicle_sp self, std::vector<double> weights) {
					for (size_t i = 0; i < weights.size(); i++) {
						self->setAW(i, weights[i]);
					}
					self->setGVW(std::accumulate(weights.begin(), weights.end(), 0.0));
				},
				R"(
				Set the vehicle axle weights.\n
				The gross vehicle weight is calculated as the sum of the axle weights automatically.

				Parameters
				----------
				weights : list[float]\n
					The vehicle axle weights.
				)", 
				py::arg("axle_weights"))
			.def("set_axle_weight", 
				[](CVehicle_sp self, size_t index, double weight) {
					self->setGVW(self->getGVW() + weight - self->getAW(index));
					self->setAW(index, weight);
				},
				R"(
				Set the specified axle weight.\n
				The gross vehicle weight is updated automatically.

				Parameters
				----------
				index : int\n
					The 0-based index of the axle.\n
				weight : float\n
					The axle weight.
				)", 
				py::arg("index"), py::arg("weight"))
			.def("set_axle_spacings", 
				[](CVehicle_sp self, std::vector<double> spacings) {
					for (size_t i = 0; i < spacings.size(); i++) {
						self->setAS(i, spacings[i]);
					}
					self->setLength(std::accumulate(spacings.begin(), spacings.end(), 0.0));
				},
				R"(
				Set the vehicle axle spacings.\n
				The vehicle length is calculated as the sum of the axle spacings automatically.

				Parameters
				----------
				spacings : list[float]\n
					The vehicle axle spacings.\n
					The last spacing should always be zero. 
				)", 
				py::arg("spacings"))
			.def("set_axle_spacing", 
				[](CVehicle_sp self, size_t index, double spacing) {
					self->setLength(self->getLength() + spacing - self->getAS(index));
					self->setAS(index, spacing);
				},
				R"(
				Set the specified axle spacing.\n
				The vehicle length is updated automatically.

				Parameters
				----------
				index : int\n
					The 0-based index of the axle.\n
				spacing : float\n
					The axle spacing.
				)",
				py::arg("index"), py::arg("spacing"))
			.def("set_axle_widths", 
				[](CVehicle_sp self, std::vector<double> widths) {
					for (size_t i = 0; i < widths.size(); i++) {
						self->setAT(i, widths[i]);
					}
				},
				R"(
				Set the vehicle axle widths.

				Parameters
				----------
				widths : list[float]\n
					The vehicle axle widths.
				)",
				py::arg("widths"))
			.def("set_axle_width", 
				[](CVehicle_sp self, size_t index, double width) {
					self->setAT(index, width);
				},
				R"(
				Set the specified axle width.

				Parameters
				----------
				index : int\n
					The 0-based index of the axle.\n
				width : float\n
					The axle width.
				)",
				py::arg("index"), py::arg("width"))
			.def("get_length", &CVehicle::getLength, "Get the vehicle length.")
			.def("get_velocity", &CVehicle::getVelocity, "Get the vehicle velocity.")
			.def("get_gvw", &CVehicle::getGVW, "Get the gross vehicle weight of the vehicle.")
			.def("get_no_axles", &CVehicle::getNoAxles, "Get the number of axles of the vehicle.")
			.def("get_axle_weights", 
				[](CVehicle_sp self) {
					std::vector<double> weights;
					for (size_t i = 0; i < self->getNoAxles(); i++) {
						weights.push_back(self->getAW(i));
					}
					return weights;
				},
				"Get the vehicle axle weights.")
			.def("get_axle_weight", &CVehicle::getAW, 
				R"(
				Get the specified axle weight.

				Parameters
				----------
				index : int\n
					The 0-based index of the axle.
				)", 
				py::arg("index"))
			.def("get_axle_spacings", 
				[](CVehicle_sp self) {
					std::vector<double> spacings;
					for (size_t i = 0; i < self->getNoAxles(); i++) {
						spacings.push_back(self->getAS(i));
					}
					return spacings;
				},
				"Get the vehicle axle spacings.")
			.def("get_axle_spacing", &CVehicle::getAS, 
				R"(
				Get the value of the specified axle spacing.

				Parameters
				----------
				index : int\n
					The 0-based index of the axle.
				)", 
				py::arg("index"))
			.def("get_axle_widths", 
				[](CVehicle_sp self) {
					std::vector<double> widths;
					for (size_t i = 0; i < self->getNoAxles(); i++) {
						widths.push_back(self->getAT(i));
					}
					return widths;
				},
				"Get the vehicle axle widths.")
			.def("get_axle_width", &CVehicle::getAT, 
				R"(
				Get the value of the specified axle width.

				Parameters
				----------
				index : int\n
					The 0-based index of the axle.
				)", 
				py::arg("index"))
			.def("get_time", &CVehicle::getTime, "Get the show-up time of the vehicle.")
			.def("get_trans", &CVehicle::getTrans, "Get the vehicle transverse position on its lane.")
			.def("get_direction", &CVehicle::getDirection, "Get the vehicle direction (1 or 2).")
			.def("_getGlobalLane", &CVehicle::getGlobalLane, "Get the 1-based global lane index of the vehicle.", py::arg("no_lanes"))
			.def("get_local_lane", &CVehicle::getLocalLane, "Get the 1-based local lane index of the vehicle.")
			.def("get_all_properties", &CVehicle::getPropInTuple, "Get all the vehicle properties in a tuple.")
			.def("set_all_properties", &CVehicle::setPropByTuple, 
				"Set all the vehicle properties from a tuple.", 
				py::arg("prop_tuple"))
			.def("_create", &CVehicle::create, py::arg("str"), py::arg("format"))
			.def(py::pickle(
				[](CVehicle_sp self) {  // __getstate__

					return self->getPropInTuple();
				},
				[](py::tuple propTuple) {  // __setstate__

					CVehicle_sp vehicle = std::make_shared<CVehicle>();
					vehicle->setPropByTuple(propTuple);
					
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

