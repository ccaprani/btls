// py_main.cpp
// the main file for the BtlsPy Build

#include <map>
#include "PrepareSim.h"
#include "CRainflow.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"


//extern CConfigData g_ConfigData;
using namespace std;
namespace py = pybind11;


void doRainflow(vector< vector< vector<double> > >& signal_data, vector< vector< map<double, double> > >& rainflow_out_count);
void countRainflow(vector< pair<double, double> >& rainflow_out, size_t i, size_t j, size_t no_load_effect, vector< vector< map<double, double> > >& rainflow_out_count);
void doSimulation(CVehicleClassification_sp pVC, vector<CBridge_sp> vBridges, vector<CLane_sp> vLanes, double SimStartTime, double SimEndTime, vector< vector< map<double, double> > >& rainflow_out_count);


// run rainflow for each load event (from all bridges, all load effects)
void doRainflow(vector< vector< vector<double> > >& signal_data, vector< vector< map<double, double> > >& rainflow_out_count) {
	size_t no_bridge = signal_data.size();
	size_t no_load_effect;
	if (rainflow_out_count.size() < no_bridge) {
		rainflow_out_count = vector<vector<map<double, double>>>(no_bridge);
	}
	Rainflow rainflow_alg = Rainflow();
	vector< pair<double, double> > rainflow_out;
	for (size_t i = 0; i < no_bridge; i++) {
		no_load_effect = signal_data[i].size();
		for (size_t j = 0; j < no_load_effect; j++) {
			rainflow_out = rainflow_alg.count_cycles(signal_data[i][j], 1);
			countRainflow(rainflow_out,i,j,no_load_effect,rainflow_out_count);
		}
	}
	return;
}

// count the rainflow output from doRainflow() (from all bridges, all load effects)
void countRainflow(vector< pair<double, double> >& rainflow_out, size_t i, size_t j, size_t no_load_effect, vector< vector< map<double, double> > >& rainflow_out_count) {
	if (rainflow_out_count[i].size() < no_load_effect) {
		rainflow_out_count[i] = vector<map<double, double>>(no_load_effect);
	}
	for (size_t k = 0; k < rainflow_out.size(); k++) {
			rainflow_out_count[i][j][rainflow_out[k].first] += rainflow_out[k].second;
	}
	return;
}

void doSimulation(CVehicleClassification_sp pVC, vector<CBridge_sp> vBridges, vector<CLane_sp> vLanes, double SimStartTime, double SimEndTime, vector< vector< map<double, double> > >& rainflow_out_count)
{
	CVehicleBuffer VehBuff(pVC, SimStartTime);
	size_t nLanes = vLanes.size();
	double curTime = SimStartTime;
	double nextTime = 0.0;
	int curDay = (int)(SimStartTime/86400);
	
	cout << "Starting simulation..." << endl;
	cout << "Day complete..." << endl;

//#pragma omp parallel
//	{
	while (curTime <= SimEndTime)
	{
		//if(curTime >= 74821.73)
		//	cout << "here" << endl;

		// find the next arrival lane and the time
		sort(vLanes.begin(), vLanes.end(), [](const CLane_sp& pL1, const CLane_sp& pL2){
			return pL1->GetNextArrivalTime() < pL2->GetNextArrivalTime();
			});
		double NextArrivalTime = vLanes[0]->GetNextArrivalTime();

		// generate the next vehicle from the lane with the next arrival time	
		// see https://stackoverflow.com/questions/18565167/non-const-lvalue-references	
		const CVehicle_sp& pVeh = vLanes[0]->GetNextVehicle();
		VehBuff.AddVehicle(pVeh);
		if (CConfigData::get().Sim.CALC_LOAD_EFFECTS)
		{
//#pragma omp for
			vector<vector<vector<double>>> vRecordEffectValues;
			for (size_t i = 0; i < vBridges.size(); i++)
			{
				// update each bridge until the next vehicle comes on
				vBridges[i]->Update(NextArrivalTime, curTime);
				// record the event for fatigue rainflow
				if (CConfigData::get().Output.DO_FATIGUE_RAINFLOW) {
					vRecordEffectValues.push_back(vBridges[i]->getEffectValues());
				}
				//vBridges[i]->UpdateMT(NextArrivalTime, curTime);
				// Add the next vehicle to the bridge, if it is not a car
				if (pVeh != nullptr && pVeh->getGVW() > CConfigData::get().Sim.MIN_GVW)
					vBridges[i]->AddVehicle(pVeh);
			}
			//for(unsigned int i = 0; i < vBridges.size(); i++)
			//	vBridges[i]->join();
			// run the rainflow algorithm
			if (CConfigData::get().Output.DO_FATIGUE_RAINFLOW) {
				doRainflow(vRecordEffectValues,rainflow_out_count);
			}
		}

		// update the current time to that of the vehicle just added
		// and delete it
		if (pVeh != nullptr)
		{
			curTime = pVeh->getTime();
			CVehicle_sp *pVeh = nullptr;
		}
		else	// finish
			curTime = SimEndTime + 1.0;

		// Keep informing the user
		if (curTime > (double)(86400)*(curDay + 1))
		{
			curDay += 1;
			cout << curDay;
			curDay % 10 == 0 ? cout << endl : cout << '\t';
		}
	}
//	}
	cout << endl;
	
	if(CConfigData::get().Sim.CALC_LOAD_EFFECTS)
	{
		for(unsigned int i = 0; i < vBridges.size(); i++)
			vBridges[i]->Finish();
	}

	VehBuff.FlushBuffer();
}

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

	void set_output_general_config (bool WRITE_TIME_HISTORY, bool WRITE_EACH_EVENT, int WRITE_EVENT_BUFFER_SIZE, bool WRITE_FATIGUE_EVENT, bool DO_FATIGUE_RAINFLOW) {
		CConfigData::get().Output.WRITE_TIME_HISTORY = WRITE_TIME_HISTORY;
		CConfigData::get().Output.WRITE_EACH_EVENT = WRITE_EACH_EVENT;
		CConfigData::get().Output.WRITE_EVENT_BUFFER_SIZE = WRITE_EVENT_BUFFER_SIZE;
		CConfigData::get().Output.WRITE_FATIGUE_EVENT = WRITE_FATIGUE_EVENT;
		CConfigData::get().Output.DO_FATIGUE_RAINFLOW = DO_FATIGUE_RAINFLOW;
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
		this->do_simulation(pVC, vBridges, vLanes, this->StartTime, this->EndTime, this->rainflow_out_count);

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
		this->do_simulation(pVC, vBridges, vLanes, this->StartTime, this->EndTime, this->rainflow_out_count);

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

	int do_simulation (CVehicleClassification_sp pVC, vector<CBridge_sp> vBridges, vector<CLane_sp> vLanes, double StartTime, double EndTime, vector<vector<map<double, double>>>& rainflow_out_count) {
		for (auto& it : vBridges) {
			it->InitializeDataMgr(StartTime);
			}
		clock_t start = clock();
		doSimulation(pVC, vBridges, vLanes, StartTime, EndTime, rainflow_out_count);
		cout << endl << "Simulation complete" << endl;
		clock_t end = clock();
		cout << endl << "Duration of analysis: " << std::fixed << std::setprecision(3) << ((double)(end) - (double)(start))/((double)CLOCKS_PER_SEC) << " s" << endl;
		return 1;
	};

	vector<vector<map<double, double>>> get_rainflow_out_count() {
		return rainflow_out_count;
	};

	double StartTime;
	double EndTime;

private:
	vector<vector<map<double, double>>> rainflow_out_count;
};

PYBIND11_MODULE(_core, m) {
	m.doc() = "BtlsPy is for short-to-mid span bridge traffic loading simulation.";
	m.def("test_c_rainflow", [](vector<double>& series, int ndigits = -1, int nbins = -1, double binsize = -1.0){Rainflow rainflow_alg = Rainflow(); return rainflow_alg.count_cycles(series,ndigits,nbins,binsize);}, py::arg("series"), py::arg("ndigits")=-1, py::arg("nbins")=-1, py::arg("binsize")=-1.0);
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
			.def("set_time_config", &BTLS::set_time_config)
			.def("set_program_mode", &BTLS::set_program_mode)
			.def("run", py::overload_cast<string>(&BTLS::run), "To run BTLS by BTLSin.txt.", py::arg("file_BTLSin"))
			.def("run", py::overload_cast<>(&BTLS::run), "To run BTLS.")
			.def("get_vehicle_classification", &BTLS::get_vehicle_classification, py::return_value_policy::reference)
			.def("get_bridges", &BTLS::get_bridges, py::return_value_policy::reference)
			.def("get_lanes", &BTLS::get_lanes, py::return_value_policy::reference)
			.def("do_simulation", &BTLS::do_simulation)
			.def("get_rainflow_result", &BTLS::get_rainflow_out_count)
			.def_readwrite("StartTime", &BTLS::StartTime)
			.def_readwrite("EndTime", &BTLS::EndTime);
	py::class_<CConfigData> cconfigdata(m, "CConfigData");
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
			read_config.def(py::init<bool, string, string, string, unsigned int, bool, bool, double>())
				.def_readwrite("READ_FILE", &CConfigData::Read_Config::READ_FILE)
				.def_readwrite("TRAFFIC_FILE", &CConfigData::Read_Config::TRAFFIC_FILE)
				.def_readwrite("GARAGE_FILE", &CConfigData::Read_Config::GARAGE_FILE)
				.def_readwrite("KERNEL_FILE", &CConfigData::Read_Config::KERNEL_FILE)
				.def_readwrite("FILE_FORMAT", &CConfigData::Read_Config::FILE_FORMAT)
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
			output_config.def(py::init<bool, bool, int, bool, bool, CConfigData::Output_Config::VehicleFile_Config, CConfigData::Output_Config::BlockMax_Config, CConfigData::Output_Config::POT_Config, CConfigData::Output_Config::Stats_Config>())
				.def_readwrite("WRITE_TIME_HISTORY", &CConfigData::Output_Config::WRITE_TIME_HISTORY)
				.def_readwrite("WRITE_EACH_EVENT", &CConfigData::Output_Config::WRITE_EACH_EVENT)
				.def_readwrite("WRITE_EVENT_BUFFER_SIZE", &CConfigData::Output_Config::WRITE_EVENT_BUFFER_SIZE)
				.def_readwrite("WRITE_FATIGUE_EVENT", &CConfigData::Output_Config::WRITE_FATIGUE_EVENT)
				.def_readwrite("DO_FATIGUE_RAINFLOW", &CConfigData::Output_Config::DO_FATIGUE_RAINFLOW)
				.def_readwrite("VehicleFile", &CConfigData::Output_Config::VehicleFile)
				.def_readwrite("BlockMax", &CConfigData::Output_Config::BlockMax)
				.def_readwrite("POT", &CConfigData::Output_Config::POT)
				.def_readwrite("Stats", &CConfigData::Output_Config::Stats);
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
		py::class_<CConfigData::Time_Config> time_config(cconfigdata, "Time_Config");
			time_config.def(py::init<int, int, int, int, int, int>())
				.def_readwrite("DAYS_PER_MT", &CConfigData::Time_Config::DAYS_PER_MT)
				.def_readwrite("MTS_PER_YR", &CConfigData::Time_Config::MTS_PER_YR)
				.def_readwrite("HOURS_PER_DAY", &CConfigData::Time_Config::HOURS_PER_DAY)
				.def_readwrite("SECS_PER_HOUR", &CConfigData::Time_Config::SECS_PER_HOUR)
				.def_readwrite("MINS_PER_HOUR", &CConfigData::Time_Config::MINS_PER_HOUR)
				.def_readwrite("SECS_PER_MIN", &CConfigData::Time_Config::SECS_PER_MIN);
	py::class_<CVehicle, CVehicle_sp> cvehicle(m, "CVehicle");
	py::class_<Classification> classification(m, "Classification");
	py::class_<CVehicleClassification, CVehicleClassification_sp> cvehicleclassification(m, "CVehicleClassification");
	py::class_<CVehClassAxle, CVehicleClassification, CVehClassAxle_sp> cvehclassaxle(m, "CVehClassAxle");
	py::class_<CVehClassPattern, CVehicleClassification, CVehClassPattern_sp> cvehclasspattern(m, "CVehClassPattern");
	py::class_<CBridge, CBridge_sp> cbridge(m, "CBridge");
	py::class_<CBridgeLane> cbridgelane(m, "CBridgeLane");
	py::class_<CInfluenceLine> cinfluenceline(m, "CInfluenceLine");
	py::class_<CInfluenceSurface> cinfluencesurface(m, "CInfluenceSurface");
	py::class_<CAxle> caxle(m, "CAxle");
	py::class_<CLane, CLane_sp> clane(m, "CLane");
	py::class_<CLaneFileTraffic, CLane, CLaneFileTraffic_sp> clanefiletraffic(m, "CLaneFileTraffic");
	py::class_<CLaneGenTraffic, CLane, CLaneGenTraffic_sp> clanegentraffic(m, "CLaneGenTraffic");
	py::class_<CVehicleBuffer> cvehiclebuffer(m, "CVehicleBuffer");
	py::class_<CReadILFile> creadilfile(m, "CReadILFile");
	py::class_<CCSVParse> ccsvparse(m, "CCSVParse");
	py::class_<CBridgeFile> cbridgefile(m, "CBridgeFile");
	py::class_<CLaneFlowComposition> claneflowcomposition(m, "CLaneFlowComposition");
	py::class_<CBlockMaxEvent> cblockmaxevent(m, "CBlockMaxEvent");
	py::class_<COutputManagerBase> coutputmanagerbase(m, "COutputManagerBase");
	py::class_<CBlockMaxManager, COutputManagerBase> cblockmaxmanager(m, "CBlockMaxManager");
	py::class_<CPOTManager, COutputManagerBase> cpotmanager(m, "CPOTManager");
	py::class_<CCalcEffect> ccalceffect(m, "CCalcEffect");
	py::class_<CRNGWrapper> crngwrapper(m, "CRNGWrapper");
	py::class_<CDistribution> cdistribution(m, "CDistribution");
	py::class_<CEffect> ceffect(m, "CEffect");
	py::class_<CEvent> cevent(m, "CEvent");
	py::class_<CEventBuffer> ceventbuffer(m, "CEventBuffer");
	py::class_<CEventManager> ceventmanager(m, "CEventManager");
	py::class_<CEventStatistics> ceventstatistics(m, "CEventStatistics");
	py::class_<CGenerator, CGenerator_sp> cgenerator(m, "CGenerator");
	py::class_<CFlowGenerator, CGenerator, CFlowGenerator_sp> cflowgenerator(m, "CFlowGenerator");
	py::class_<CFlowGenNHM, CFlowGenerator, CFlowGenNHM_sp> cflowgennhm(m, "CFlowGenNHM");
	py::class_<CFlowGenCongested, CFlowGenerator, CFlowGenCongested_sp> cflowgencongested(m, "CFlowGenCongested");
	py::class_<CFlowGenPoisson, CFlowGenerator, CFlowGenPoisson_sp> cflowgenpoisson(m, "CFlowGenPoisson");
	py::class_<CFlowGenConstant, CFlowGenerator, CFlowGenConstant_sp> cflowgenconstant(m, "CFlowGenConstant");
	py::class_<CModelData, CModelData_sp> cmodeldata(m, "CModelData");
	py::class_<CLaneFlowData, CModelData, CLaneFlowData_sp> claneflowdata(m, "CLaneFlowData");
	py::class_<CFlowModelData, CModelData, CFlowModelData_sp> cflowmodeldata(m, "CFlowModelData");
	py::class_<CFlowModelDataNHM, CFlowModelData, CFlowModelDataNHM_sp> cflowmodeldatanhm(m, "CFlowModelDataNHM");
	py::class_<CFlowModelDataCongested, CFlowModelData, CFlowModelDataCongested_sp> cflowmodeldatacongested(m, "CFlowModelDataCongested");
	py::class_<CFlowModelDataPoisson, CFlowModelData, CFlowModelDataPoisson_sp> cflowmodeldatapoisson(m, "CFlowModelDataPoisson");
	py::class_<CFlowModelDataConstant, CFlowModelData, CFlowModelDataConstant_sp> cflowmodeldataconstant(m, "CFlowModelDataConstant");
	py::class_<CStatsManager> cstatsmanager(m, "CStatsManager");
	py::class_<CTrafficData> ctrafficdata(m, "CTrafficData");
	py::class_<CTrafficFiles> ctrafficfiles(m, "CTrafficFiles");
	py::class_<CTriModalNormal> ctrimodalnormal(m, "CTriModalNormal");
	py::class_<CFlowRateData> cflowratedata(m, "CFlowRateData");
	py::class_<CVehicleGenerator, CGenerator, CVehicleGenerator_sp> cvehiclegenerator(m, "CVehicleGenerator");
	py::class_<CVehicleGenConstant, CVehicleGenerator, CVehicleGenConstant_sp> cvehiclegenconstant(m, "CVehicleGenConstant");
	py::class_<CVehicleGenGarage, CVehicleGenerator, CVehicleGenGarage_sp> cvehiclegengarage(m, "CVehicleGenGarage");
	py::class_<CVehicleGenGrave, CVehicleGenerator, CVehicleGenGrave_sp> cvehiclegengrave(m, "CVehicleGenGrave");
	py::class_<CVehicleModelData, CModelData, CVehicleModelData_sp> cvehiclemodeldata(m, "CVehicleModelData");
	py::class_<CVehicleTrafficFile> cvehicletrafficfile(m, "CVehicleTrafficFile");
	py::class_<CVehModelDataGarage, CVehicleModelData, CVehModelDataGarage_sp> cvehmodeldatagarage(m, "CVehModelDataGarage");
	py::class_<CVehModelDataGrave, CVehicleModelData, CVehModelDataGrave_sp> cvehmodeldatagrave(m, "CVehModelDataGrave");
	// py::class_<Normal> normal(m, "Normal");
	// py::enum_<EFlowModel>(m, "EFlowModel").export_values();
	// py::enum_<EVehicleModel>(m, "EVehicleModel").export_values();
};
