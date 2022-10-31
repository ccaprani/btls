// Load config from Python to assign to C++.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "CSVParse.h"

///////////// CConfigDataCore ///////////////

class CConfigDataCore
{

public:
	CConfigDataCore();
	~CConfigDataCore();

	bool ReadData(std::string in_file);

	struct Mode_Config
	{
		size_t PROGRAM_MODE;
	} Mode = {1};

	struct Road_Config
	{
		string LANES_FILE;
		size_t NO_LANES_DIR1; // assigned in program
		size_t NO_LANES_DIR2; // assigned in program
		size_t NO_LANES;	  // assigned in program
		size_t NO_DIRS;		  // assigned in program
	} Road = {"laneflow.csv", 1, 1, 2, 2};

	struct Gen_Config
	{
		string TRAFFIC_FOLDER;
		bool GEN_TRAFFIC;
		size_t NO_DAYS;
		double TRUCK_TRACK_WIDTH;
		double LANE_ECCENTRICITY_STD;
		double NO_OVERLAP_LENGTH; // assigned in program
	} Gen = {"../Traffic/Auxerre/", false, 1, 190.0, 0.0, 100.0};

	struct Read_Config
	{
		bool READ_FILE;
		string TRAFFIC_FILE;
		string GARAGE_FILE;
		string KERNEL_FILE;
		string CONSTANT_FILE;
		unsigned int FILE_FORMAT;
		bool USE_CONSTANT_SPEED;
		bool USE_AVE_SPEED;
		double CONST_SPEED;
	} Read = {false, "traffic.txt", "garage.txt", "kernel.csv", "constant_vehicle.csv", 4, false, false, 0.0};

	struct Traffic_Config
	{
		int VEHICLE_MODEL;
		int HEADWAY_MODEL;
		int CLASSIFICATION;
		double CONGESTED_SPACING;
		double CONGESTED_SPEED;
		double CONGESTED_GAP; // assigned automatically (later)
		double CONGESTED_GAP_COEF_VAR;
		double CONSTANT_SPEED;
		double CONSTANT_GAP;
		bool GEN_CAR;
	} Traffic = {0, 0, 1, 5.0, 30.0, 5.0, 0.05, 36.0, 10.0, true};

	struct Sim_Config
	{
		bool CALC_LOAD_EFFECTS;
		string BRIDGE_FILE;
		string INFLINE_FILE;
		string INFSURF_FILE;
		double CALC_TIME_STEP;
		int MIN_GVW;
	} Sim = {false, "bridges.txt", "IL.txt", "IS.csv", 0.1, 35};

	struct Output_Config
	{
		bool WRITE_TIME_HISTORY;
		bool WRITE_EACH_EVENT;
		int WRITE_EVENT_BUFFER_SIZE;
		bool WRITE_FATIGUE_EVENT;

		struct VehicleFile_Config
		{
			bool WRITE_VEHICLE_FILE;
			unsigned int FILE_FORMAT;
			string VEHICLE_FILENAME;
			int WRITE_VEHICLE_BUFFER_SIZE;
			bool WRITE_FLOW_STATS;
		} VehicleFile;

		struct BlockMax_Config
		{
			bool WRITE_BM;
			bool WRITE_BM_VEHICLES;
			bool WRITE_BM_SUMMARY;
			bool WRITE_BM_MIXED;
			int BLOCK_SIZE_DAYS;
			int BLOCK_SIZE_SECS;
			int WRITE_BM_BUFFER_SIZE;
		} BlockMax;

		struct POT_Config
		{
			bool WRITE_POT;
			bool WRITE_POT_VEHICLES;
			bool WRITE_POT_SUMMARY;
			bool WRITE_POT_COUNTER;
			int POT_COUNT_SIZE_DAYS;
			int POT_COUNT_SIZE_SECS;
			int WRITE_POT_BUFFER_SIZE;
		} POT;

		struct Stats_Config
		{
			bool WRITE_STATS;
			bool WRITE_SS_CUMULATIVE;
			bool WRITE_SS_INTERVALS;
			int WRITE_SS_INTERVAL_SIZE;
			int WRITE_SS_BUFFER_SIZE;
		} Stats;

		struct Fatigue_Config
		{
			bool DO_FATIGUE_RAINFLOW;
			int RAINFLOW_DECIMAL;
			double RAINFLOW_CUTOFF;
			int WRITE_RAINFLOW_BUFFER_SIZE;
		} Fatigue;

	} Output = {false, false, 10000, false, {false, 4, "vehicles.txt", 10000, false}, // VehicleFile_Config
				{false, false, false, false, 1, 0, 10000},							  // BlockMax_Config
				{false, false, false, false, 1, 0, 10000},							  // POT_Config
				{false, false, false, 3600, 10000},									  // Stats_Config
				{false, 3, 0.0, 10000}};											  // Fatigue_Config

	struct Time_Config
	{
		int DAYS_PER_MT;
		int MTS_PER_YR;
		int HOURS_PER_DAY;
		int SECS_PER_HOUR;
		int MINS_PER_HOUR;
		int SECS_PER_MIN;
	} Time = {25, 10, 24, 3600, 60, 60};

	// functions
	void VehGenGrave(std::string lanes_file, std::string traffic_folder, size_t no_days, double truck_track_width, double lane_eccentricity_std);
	void VehGenGarage(std::string lanes_file, std::string garage_file, unsigned int file_format, std::string kernel_file, size_t no_days, double lane_eccentricity_std);
	void VehGenConstant(std::string lanes_file, std::string constant_file, size_t no_days, double lane_eccentricity_std);

	void FlowGenNHM(int classification, std::string traffic_folder, bool gen_car = true);
	void FlowGenConstant(int classification, double constant_speed, double constant_gap, bool gen_car = true);
	void FlowGenCongestion(int classification, double congested_spacing, double congested_speed, double congested_gap_coef_var, bool gen_car = true);
	void FlowGenFreeFlow(int classification, bool gen_car = true);

	void TrafficRead(std::string traffic_file, unsigned int file_format, bool use_constant_speed, bool use_ave_speed, double const_speed);

	void Simulation(std::string bridge_file, std::string infline_file, std::string infsurf_file, double calc_time_step, int min_gvw);

	void OutputGeneral(bool write_time_history = false, bool write_each_event = false, bool write_fatigue_event = false, int write_buffer_size = 10000);
	void OutputVehicleFile(bool write_vehicle_file = false, unsigned int vehicle_file_format = 4, std::string vehicle_file_name = "Vehicles.txt", int buffer_size = 10000, bool write_flow_stats = false);
	void OutputBlockMax(bool write_blockmax = false, bool write_vehicle = false, bool write_summary = false, bool write_mixed = false, int block_size_days = 1, int block_size_secs = 0, int buffer_size = 10000);
	void OutputPOT(bool write_pot = false, bool write_vehicle = false, bool write_summary = false, bool write_counter = false, int pot_size_days = 1, int pot_size_secs = 0, int buffer_size = 10000);
	void OutputStats(bool write_stats = false, bool write_cumulative = false, bool write_intervals = false, int interval_size = 3600, int buffer_size = 10000);
	void OutputFatigue(bool do_rainflow = false, int decimal = 3, double cut_off_value = 0.0, int buffer_size = 10000);

private:
	CCSVParse m_CSV;
	std::string m_CommentString;

	void doDerivedConstants();
	void ExtractData();
	std::string GetNextDataLine();
};

///////////// CConfigData ///////////////

class CConfigData : public CConfigDataCore
{
	// Using Singleton pattern here:
	// https://stackoverflow.com/questions/1008019/c-singleton-design-pattern/1008289#1008289
public:
	static CConfigData &get()
	{
		static CConfigData instance; // Guaranteed to be destroyed.
		return instance;			 // Instantiated on first use.
	}

	// C++ 11
	// =======
	// We can use the better technique of deleting the methods
	// we don't want.
	CConfigData(CConfigData const &) = delete;
	void operator=(CConfigData const &) = delete;
	// Note: Scott Meyers mentions in his Effective Modern
	//       C++ book, that deleted functions should generally
	//       be public as it results in better error messages
	//       due to the compilers behavior to check accessibility
	//       before deleted status

private:
	CConfigData(){};
};
