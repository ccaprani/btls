// py_main.cpp
// the main file for the PyBTLS Build

#include "PrepareSim.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"


//extern CConfigData g_ConfigData;
using namespace std;
namespace py = pybind11;


class BTLS {
public:
	BTLS () {
		this->get_info();
		CConfigData::get();
		this->StartTime = 0.0;
	 	this->EndTime = 0.0;
	};

	void get_info () {
		cout << "---------------------------------------------" << endl;
		cout << "Bridge Traffic Load Simulation - C.C. Caprani" << endl;
		cout << "                Version 1.3.5			      " << endl;
		cout << "---------------------------------------------" << endl << endl;
	};

	void set_road_config (CConfigData::Road_Config& road_config) {
		CConfigData::get().Road = road_config;
	};

	void set_gen_config (CConfigData::Gen_Config& gen_config) {
		CConfigData::get().Gen = gen_config;
	};

	void set_read_config (CConfigData::Read_Config& read_config) {
		CConfigData::get().Read = read_config;
	};

	void set_traffic_config (CConfigData::Traffic_Config& traffic_config) {
		CConfigData::get().Traffic = traffic_config;
	};

	void set_sim_config (CConfigData::Sim_Config& sim_config) {
		CConfigData::get().Sim = sim_config;
	};

	void set_output_config (CConfigData::Output_Config& output_config) {
		CConfigData::get().Output = output_config;
	};

	void set_output_general_config (bool WRITE_TIME_HISTORY, bool WRITE_EACH_EVENT, int WRITE_EVENT_BUFFER_SIZE, bool WRITE_FATIGUE_EVENT) {
		CConfigData::get().Output.WRITE_TIME_HISTORY = WRITE_TIME_HISTORY;
		CConfigData::get().Output.WRITE_EACH_EVENT = WRITE_EACH_EVENT;
		CConfigData::get().Output.WRITE_EVENT_BUFFER_SIZE = WRITE_EVENT_BUFFER_SIZE;
		CConfigData::get().Output.WRITE_FATIGUE_EVENT = WRITE_FATIGUE_EVENT;
	};

	void set_output_vehiclefile_config (CConfigData::Output_Config::VehicleFile_Config& output_vehiclefile_config) {
		CConfigData::get().Output.VehicleFile = output_vehiclefile_config;
	};

	void set_output_blockmax_config (CConfigData::Output_Config::BlockMax_Config& output_blockmax_config) {
		CConfigData::get().Output.BlockMax = output_blockmax_config;
	};

	void set_output_pot_config (CConfigData::Output_Config::POT_Config& output_pot_config) {
		CConfigData::get().Output.POT = output_pot_config;
	};

	void set_output_stats_config (CConfigData::Output_Config::Stats_Config& output_stats_config) {
		CConfigData::get().Output.Stats = output_stats_config;
	};

	void set_output_fatigue_config (CConfigData::Output_Config::Fatigue_Config& output_fatigue_config) {
		CConfigData::get().Output.Fatigue = output_fatigue_config;
	};

	void set_time_config (CConfigData::Time_Config& time_config) {
		CConfigData::get().Time = time_config;
	}

	void set_program_mode (int program_mode) {
		CConfigData::get().Mode.PROGRAM_MODE = program_mode;
		switch(CConfigData::get().Mode.PROGRAM_MODE)
		{
		case 1:
			CConfigData::get().Gen.GEN_TRAFFIC = true;
			CConfigData::get().Read.READ_FILE = false;
			CConfigData::get().Sim.CALC_LOAD_EFFECTS = true;
			break;
		case 2:
			CConfigData::get().Gen.GEN_TRAFFIC = true;
			CConfigData::get().Sim.CALC_LOAD_EFFECTS = false;
			CConfigData::get().Read.READ_FILE = false;
			break;
		case 3:
			CConfigData::get().Gen.GEN_TRAFFIC = false;
			CConfigData::get().Sim.CALC_LOAD_EFFECTS = true;
			CConfigData::get().Read.READ_FILE = true;
			break;
		}
		if (CConfigData::get().Mode.PROGRAM_MODE == 2) {
			CConfigData::get().Output.VehicleFile.WRITE_VEHICLE_FILE = true;
			}
		CConfigData::get().Traffic.CONGESTED_SPEED = CConfigData::get().Traffic.CONGESTED_SPEED/3.6;	// km/h to m/s
		CConfigData::get().Traffic.CONGESTED_GAP = CConfigData::get().Traffic.CONGESTED_SPACING/CConfigData::get().Traffic.CONGESTED_SPEED;
		cout << "Program Mode: " << CConfigData::get().Mode.PROGRAM_MODE << endl << endl;
	};

	int run (string file_BTLSin) {

		// Load configs from BTLSin.txt.
		if (!CConfigData::get().ReadData(file_BTLSin) )
		{
			cout << "BTLSin file could not be opened" << endl;
			cout << "Using default values" << endl;
		}
		cout << "Program Mode: " << CConfigData::get().Mode.PROGRAM_MODE << endl << endl;

		CVehicleClassification_sp pVC = this->get_vehicle_classification();

		// Bridges are read in first, to set max bridge length, needed for flow generators.
		vector<CBridge_sp> vBridges = this->get_bridges();

		// Now read generator info for lanes.
		vector<CLane_sp> vLanes = this->get_lanes(pVC);
		
		// Now we know the time, we can tell bridge data managers when to start.
		this->do_simulation(pVC, vBridges, vLanes, this->StartTime, this->EndTime);

		// Reset the Times.
		this->StartTime = 0.0;
		this->EndTime = 0.0;

		return 1;
	};

	int run () {

		cout << "Program Mode: " << CConfigData::get().Mode.PROGRAM_MODE << endl << endl;

		CVehicleClassification_sp pVC = this->get_vehicle_classification();

		// Bridges are read in first, to set max bridge length, needed for flow generators.
		vector<CBridge_sp> vBridges = this->get_bridges();

		// Now read generator info for lanes.
		vector<CLane_sp> vLanes = this->get_lanes(pVC);
		
		// Now we know the time, we can tell bridge data managers when to start.
		this->do_simulation(pVC, vBridges, vLanes, this->StartTime, this->EndTime);

		// Reset the Times.
		this->StartTime = 0.0;
		this->EndTime = 0.0;

		return 1;
	};

	CVehicleClassification_sp get_vehicle_classification () {
		CVehicleClassification_sp pVC;
		switch (CConfigData::get().Traffic.CLASSIFICATION) {
			case 1: // Pattern
				pVC = std::make_shared<CVehClassPattern>(); break;
			default: // Axle count
				pVC = std::make_shared<CVehClassAxle>(); break;
			}
		return pVC;
	};

	vector<CBridge_sp>& get_bridges () {
		static vector<CBridge_sp> vBridges;
		if (CConfigData::get().Sim.CALC_LOAD_EFFECTS) {
			vBridges = PrepareBridges();
			}
		return vBridges;
	};

	vector<CLane_sp>& get_lanes (CVehicleClassification_sp pVC) {
		static vector<CLane_sp> vLanes;
		if (CConfigData::get().Gen.GEN_TRAFFIC) {
			GetGeneratorLanes(pVC, vLanes, this->StartTime, this->EndTime);
			}
		if (CConfigData::get().Read.READ_FILE) {
			GetTrafficFileLanes(pVC, vLanes, this->StartTime, this->EndTime);
			}
		return vLanes;
	};

	int do_simulation (CVehicleClassification_sp pVC, vector<CBridge_sp> vBridges, vector<CLane_sp> vLanes, double StartTime, double EndTime) {
		for (auto& it : vBridges) {
			it->InitializeDataMgr(StartTime);
			}
		clock_t start = clock();
		doSimulation(pVC, vBridges, vLanes, StartTime, EndTime);
		cout << endl << "Simulation complete" << endl;
		clock_t end = clock();
		cout << endl << "Duration of analysis: " << std::fixed << std::setprecision(3) << ((double)(end) - (double)(start))/((double)CLOCKS_PER_SEC) << " s" << endl;
		return 1;
	};

	double StartTime;
	double EndTime;
};

PYBIND11_MODULE(_core, m) {
	m.doc() = "PyBTLS is for short-to-mid span bridge traffic loading simulation.";
	// m.def("test_c_rainflow", [](vector<double>& series, int ndigits = -1, int nbins = -1, double binsize = -1.0){CRainflow rainflow_alg = CRainflow(); return rainflow_alg.countCycles(series,ndigits,nbins,binsize);}, py::arg("series"), py::arg("ndigits")=-1, py::arg("nbins")=-1, py::arg("binsize")=-1.0);
	py::class_<BTLS> btls(m, "BTLS");
		btls.def(py::init<>())
			.def("set_road_config", &BTLS::set_road_config)
			.def("set_gen_config", &BTLS::set_gen_config)
			.def("set_read_config", &BTLS::set_read_config)
			.def("set_traffic_config", &BTLS::set_traffic_config)
			.def("set_sim_config", &BTLS::set_sim_config)
			.def("set_output_config", &BTLS::set_output_config)
			.def("set_output_general_config", &BTLS::set_output_general_config)
			.def("set_output_vehiclefile_config", &BTLS::set_output_vehiclefile_config)
			.def("set_output_blockmax_config", &BTLS::set_output_blockmax_config)
			.def("set_output_pot_config", &BTLS::set_output_pot_config)
			.def("set_output_stats_config", &BTLS::set_output_stats_config)
			.def("set_output_fatigue_config", &BTLS::set_output_fatigue_config)
			.def("set_time_config", &BTLS::set_time_config)
			.def("set_program_mode", &BTLS::set_program_mode)
			.def("run", py::overload_cast<string>(&BTLS::run), "To run BTLS by BTLSin.txt.", py::arg("file_BTLSin"))
			.def("run", py::overload_cast<>(&BTLS::run), "To run BTLS.")
			.def("get_vehicle_classification", &BTLS::get_vehicle_classification, py::return_value_policy::reference)
			.def("get_bridges", &BTLS::get_bridges, py::return_value_policy::reference)
			.def("get_lanes", &BTLS::get_lanes, py::return_value_policy::reference)
			.def("do_simulation", &BTLS::do_simulation)
			.def_readwrite("StartTime", &BTLS::StartTime)
			.def_readwrite("EndTime", &BTLS::EndTime);
	py::class_<CConfigData> cconfigdata(m, "ConfigData");
		py::class_<CConfigData::Road_Config> road_config(cconfigdata, "Road_Config");
			road_config.def(py::init<string, size_t, size_t, size_t, size_t>())
				.def_readwrite("LANES_FILE", &CConfigData::Road_Config::LANES_FILE)
				.def_readwrite("NO_LANES_DIR1", &CConfigData::Road_Config::NO_LANES_DIR1)
				.def_readwrite("NO_LANES_DIR2", &CConfigData::Road_Config::NO_LANES_DIR2)
				.def_readwrite("NO_LANES", &CConfigData::Road_Config::NO_LANES)
				.def_readwrite("NO_DIRS", &CConfigData::Road_Config::NO_DIRS);
		py::class_<CConfigData::Gen_Config> gen_config(cconfigdata, "Gen_Config");
			gen_config.def(py::init<string, bool, size_t, double, double, double>())
				.def_readwrite("TRAFFIC_FOLDER", &CConfigData::Gen_Config::TRAFFIC_FOLDER)
				.def_readwrite("GEN_TRAFFIC", &CConfigData::Gen_Config::GEN_TRAFFIC)
				.def_readwrite("NO_DAYS", &CConfigData::Gen_Config::NO_DAYS)
				.def_readwrite("TRUCK_TRACK_WIDTH", &CConfigData::Gen_Config::TRUCK_TRACK_WIDTH)
				.def_readwrite("LANE_ECCENTRICITY_STD", &CConfigData::Gen_Config::LANE_ECCENTRICITY_STD)
				.def_readwrite("NO_OVERLAP_LENGTH", &CConfigData::Gen_Config::NO_OVERLAP_LENGTH);
		py::class_<CConfigData::Read_Config> read_config(cconfigdata, "Read_Config");
			read_config.def(py::init<bool, string, string, string, string, unsigned int, bool, bool, double>())
				.def_readwrite("READ_FILE", &CConfigData::Read_Config::READ_FILE)
				.def_readwrite("TRAFFIC_FILE", &CConfigData::Read_Config::TRAFFIC_FILE)
				.def_readwrite("GARAGE_FILE", &CConfigData::Read_Config::GARAGE_FILE)
				.def_readwrite("KERNEL_FILE", &CConfigData::Read_Config::KERNEL_FILE)
				.def_readwrite("CONSTANT_FILE", &CConfigData::Read_Config::CONSTANT_FILE)
				.def_readwrite("FILE_FORMAT", &CConfigData::Read_Config::FILE_FORMAT)
				.def_readwrite("USE_CONSTANT_SPEED", &CConfigData::Read_Config::USE_CONSTANT_SPEED)
				.def_readwrite("USE_AVE_SPEED", &CConfigData::Read_Config::USE_AVE_SPEED)
				.def_readwrite("CONST_SPEED", &CConfigData::Read_Config::CONST_SPEED);
		py::class_<CConfigData::Traffic_Config> traffic_config(cconfigdata, "Traffic_Config");
			traffic_config.def(py::init<int, int, int, double, double, double, double>())
				.def_readwrite("VEHICLE_MODEL", &CConfigData::Traffic_Config::VEHICLE_MODEL)
				.def_readwrite("HEADWAY_MODEL", &CConfigData::Traffic_Config::HEADWAY_MODEL)
				.def_readwrite("CLASSIFICATION", &CConfigData::Traffic_Config::CLASSIFICATION)
				.def_readwrite("CONGESTED_SPACING", &CConfigData::Traffic_Config::CONGESTED_SPACING)
				.def_readwrite("CONGESTED_SPEED", &CConfigData::Traffic_Config::CONGESTED_SPEED)
				.def_readwrite("CONGESTED_GAP", &CConfigData::Traffic_Config::CONGESTED_GAP)
				.def_readwrite("CONGESTED_GAP_COEF_VAR", &CConfigData::Traffic_Config::CONGESTED_GAP_COEF_VAR);
		py::class_<CConfigData::Sim_Config> sim_config(cconfigdata, "Sim_Config");
			sim_config.def(py::init<bool, string, string, string, double, int>())
				.def_readwrite("CALC_LOAD_EFFECTS", &CConfigData::Sim_Config::CALC_LOAD_EFFECTS)
				.def_readwrite("BRIDGE_FILE", &CConfigData::Sim_Config::BRIDGE_FILE)
				.def_readwrite("INFLINE_FILE", &CConfigData::Sim_Config::INFLINE_FILE)
				.def_readwrite("INFSURF_FILE", &CConfigData::Sim_Config::INFSURF_FILE)
				.def_readwrite("CALC_TIME_STEP", &CConfigData::Sim_Config::CALC_TIME_STEP)
				.def_readwrite("MIN_GVW", &CConfigData::Sim_Config::MIN_GVW);
		py::class_<CConfigData::Output_Config> output_config(cconfigdata, "Output_Config");
			output_config.def(py::init<bool, bool, int, bool, CConfigData::Output_Config::VehicleFile_Config, CConfigData::Output_Config::BlockMax_Config, CConfigData::Output_Config::POT_Config, CConfigData::Output_Config::Stats_Config, CConfigData::Output_Config::Fatigue_Config>())
				.def_readwrite("WRITE_TIME_HISTORY", &CConfigData::Output_Config::WRITE_TIME_HISTORY)
				.def_readwrite("WRITE_EACH_EVENT", &CConfigData::Output_Config::WRITE_EACH_EVENT)
				.def_readwrite("WRITE_EVENT_BUFFER_SIZE", &CConfigData::Output_Config::WRITE_EVENT_BUFFER_SIZE)
				.def_readwrite("WRITE_FATIGUE_EVENT", &CConfigData::Output_Config::WRITE_FATIGUE_EVENT)
				.def_readwrite("VehicleFile", &CConfigData::Output_Config::VehicleFile)
				.def_readwrite("BlockMax", &CConfigData::Output_Config::BlockMax)
				.def_readwrite("POT", &CConfigData::Output_Config::POT)
				.def_readwrite("Stats", &CConfigData::Output_Config::Stats)
				.def_readwrite("Fatigue", &CConfigData::Output_Config::Fatigue);
			py::class_<CConfigData::Output_Config::VehicleFile_Config> vehiclefile_config(output_config, "VehicleFile_Config");
				vehiclefile_config.def(py::init<bool, unsigned int, string, int, bool>())
					.def_readwrite("WRITE_VEHICLE_FILE", &CConfigData::Output_Config::VehicleFile_Config::WRITE_VEHICLE_FILE)
					.def_readwrite("FILE_FORMAT", &CConfigData::Output_Config::VehicleFile_Config::FILE_FORMAT)
					.def_readwrite("VEHICLE_FILENAME", &CConfigData::Output_Config::VehicleFile_Config::VEHICLE_FILENAME)
					.def_readwrite("WRITE_VEHICLE_BUFFER_SIZE", &CConfigData::Output_Config::VehicleFile_Config::WRITE_VEHICLE_BUFFER_SIZE)
					.def_readwrite("WRITE_FLOW_STATS", &CConfigData::Output_Config::VehicleFile_Config::WRITE_FLOW_STATS);
			py::class_<CConfigData::Output_Config::BlockMax_Config> blockmax_config(output_config, "BlockMax_Config");
				blockmax_config.def(py::init<bool, bool, bool, bool, int, int, int>())
					.def_readwrite("WRITE_BM", &CConfigData::Output_Config::BlockMax_Config::WRITE_BM)
					.def_readwrite("WRITE_BM_VEHICLES", &CConfigData::Output_Config::BlockMax_Config::WRITE_BM_VEHICLES)
					.def_readwrite("WRITE_BM_SUMMARY", &CConfigData::Output_Config::BlockMax_Config::WRITE_BM_SUMMARY)
					.def_readwrite("WRITE_BM_MIXED", &CConfigData::Output_Config::BlockMax_Config::WRITE_BM_MIXED)
					.def_readwrite("BLOCK_SIZE_DAYS", &CConfigData::Output_Config::BlockMax_Config::BLOCK_SIZE_DAYS)
					.def_readwrite("BLOCK_SIZE_SECS", &CConfigData::Output_Config::BlockMax_Config::BLOCK_SIZE_SECS)
					.def_readwrite("WRITE_BM_BUFFER_SIZE", &CConfigData::Output_Config::BlockMax_Config::WRITE_BM_BUFFER_SIZE);
			py::class_<CConfigData::Output_Config::POT_Config> pot_config(output_config, "POT_Config");
				pot_config.def(py::init<bool, bool, bool, bool, int, int, int>())
					.def_readwrite("WRITE_POT", &CConfigData::Output_Config::POT_Config::WRITE_POT)
					.def_readwrite("WRITE_POT_VEHICLES", &CConfigData::Output_Config::POT_Config::WRITE_POT_VEHICLES)
					.def_readwrite("WRITE_POT_SUMMARY", &CConfigData::Output_Config::POT_Config::WRITE_POT_SUMMARY)
					.def_readwrite("WRITE_POT_COUNTER", &CConfigData::Output_Config::POT_Config::WRITE_POT_COUNTER)
					.def_readwrite("POT_COUNT_SIZE_DAYS", &CConfigData::Output_Config::POT_Config::POT_COUNT_SIZE_DAYS)
					.def_readwrite("POT_COUNT_SIZE_SECS", &CConfigData::Output_Config::POT_Config::POT_COUNT_SIZE_SECS)
					.def_readwrite("WRITE_POT_BUFFER_SIZE", &CConfigData::Output_Config::POT_Config::WRITE_POT_BUFFER_SIZE);
			py::class_<CConfigData::Output_Config::Stats_Config> stats_config(output_config, "Stats_Config");
				stats_config.def(py::init<bool, bool, bool, int, int>())
					.def_readwrite("WRITE_STATS", &CConfigData::Output_Config::Stats_Config::WRITE_STATS)
					.def_readwrite("WRITE_SS_CUMULATIVE", &CConfigData::Output_Config::Stats_Config::WRITE_SS_CUMULATIVE)
					.def_readwrite("WRITE_SS_INTERVALS", &CConfigData::Output_Config::Stats_Config::WRITE_SS_INTERVALS)
					.def_readwrite("WRITE_SS_INTERVAL_SIZE", &CConfigData::Output_Config::Stats_Config::WRITE_SS_INTERVAL_SIZE)
					.def_readwrite("WRITE_SS_BUFFER_SIZE", &CConfigData::Output_Config::Stats_Config::WRITE_SS_BUFFER_SIZE);
			py::class_<CConfigData::Output_Config::Fatigue_Config> fatigue_config(output_config, "Fatigue_Config");
				fatigue_config.def(py::init<bool, int, double, int>())
					.def_readwrite("DO_FATIGUE_RAINFLOW", &CConfigData::Output_Config::Fatigue_Config::DO_FATIGUE_RAINFLOW)
					.def_readwrite("RAINFLOW_DECIMAL", &CConfigData::Output_Config::Fatigue_Config::RAINFLOW_DECIMAL)
					.def_readwrite("RAINFLOW_CUTOFF", &CConfigData::Output_Config::Fatigue_Config::RAINFLOW_CUTOFF)
					.def_readwrite("WRITE_RAINFLOW_BUFFER_SIZE", &CConfigData::Output_Config::Fatigue_Config::WRITE_RAINFLOW_BUFFER_SIZE);
		py::class_<CConfigData::Time_Config> time_config(cconfigdata, "Time_Config");
			time_config.def(py::init<int, int, int, int, int, int>())
				.def_readwrite("DAYS_PER_MT", &CConfigData::Time_Config::DAYS_PER_MT)
				.def_readwrite("MTS_PER_YR", &CConfigData::Time_Config::MTS_PER_YR)
				.def_readwrite("HOURS_PER_DAY", &CConfigData::Time_Config::HOURS_PER_DAY)
				.def_readwrite("SECS_PER_HOUR", &CConfigData::Time_Config::SECS_PER_HOUR)
				.def_readwrite("MINS_PER_HOUR", &CConfigData::Time_Config::MINS_PER_HOUR)
				.def_readwrite("SECS_PER_MIN", &CConfigData::Time_Config::SECS_PER_MIN);
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
			.def("set_time_on_bridge", &CVehicle::setTimeOnBridge, py::arg("bridge_length"))
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
		cvehclassaxle.def(py::init<>());
	py::class_<CVehClassPattern, CVehicleClassification, CVehClassPattern_sp> cvehclasspattern(m, "VehClassPattern");
		cvehclasspattern.def(py::init<>());
	py::class_<CBridge, CBridge_sp> cbridge(m, "Bridge");
	py::class_<CBridgeLane> cbridgelane(m, "BridgeLane");
	py::class_<CInfluenceLine> cinfluenceline(m, "InfluenceLine");
	py::class_<CInfluenceSurface> cinfluencesurface(m, "InfluenceSurface");
	py::class_<CAxle> caxle(m, "Axle");
	py::class_<CLane, CLane_sp> clane(m, "Lane");
	py::class_<CLaneFileTraffic, CLane, CLaneFileTraffic_sp> clanefiletraffic(m, "LaneFileTraffic");
	py::class_<CLaneGenTraffic, CLane, CLaneGenTraffic_sp> clanegentraffic(m, "LaneGenTraffic");
	py::class_<CVehicleBuffer> cvehiclebuffer(m, "VehicleBuffer");
	py::class_<CReadILFile> creadilfile(m, "ReadILFile");
	py::class_<CCSVParse> ccsvparse(m, "CSVParse");
	py::class_<CBridgeFile> cbridgefile(m, "BridgeFile");
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
	py::class_<CVehicleGenGarage, CVehicleGenerator, CVehicleGenGarage_sp> cvehiclegengarage(m, "VehicleGenGarage");
	py::class_<CVehicleGenGrave, CVehicleGenerator, CVehicleGenGrave_sp> cvehiclegengrave(m, "VehicleGenGrave");
	py::class_<CVehicleModelData, CModelData, CVehicleModelData_sp> cvehiclemodeldata(m, "VehicleModelData");
	py::class_<CVehicleTrafficFile> cvehicletrafficfile(m, "VehicleTrafficFile");
		cvehicletrafficfile.def(py::init<CVehicleClassification_sp, bool, bool, double>(), py::arg("vehicle_classification"), py::arg("use_const_speed"), py::arg("use_ave_speed"), py::arg("const_speed_value"))
			.def("read", &CVehicleTrafficFile::Read, py::arg("file"), py::arg("format"))
			.def("get_no_vehicles", &CVehicleTrafficFile::getNoVehicles)
			.def("get_vehicles", &CVehicleTrafficFile::getVehicles);
	py::class_<CVehModelDataConstant, CVehicleModelData, CVehModelDataConstant_sp> cvehmodeldataconstant(m, "VehModelDataConstant");
	py::class_<CVehModelDataGarage, CVehicleModelData, CVehModelDataGarage_sp> cvehmodeldatagarage(m, "VehModelDataGarage");
	py::class_<CVehModelDataGrave, CVehicleModelData, CVehModelDataGrave_sp> cvehmodeldatagrave(m, "VehModelDataGrave");
	// py::class_<Normal> normal(m, "Normal");
	// py::enum_<EFlowModel>(m, "EFlowModel").export_values();
	// py::enum_<EVehicleModel>(m, "EVehicleModel").export_values();
};
