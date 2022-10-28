// Load config from Python to assign to C++.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "CSVParse.h"


class CPyConfigData
{

public:
    CPyConfigData();
    ~CPyConfigData();

    bool ReadData(std::string in_file);

    // Program Mode
    size_t Mode_PROGRAM_MODE;

    // Road
    std::string Road_LANES_FILE = "lane_flow.txt";
    size_t Road_NO_LANES_DIR1;  // assigned in __init__.py
    size_t Road_NO_LANES_DIR2;  // assigned in __init__.py
    size_t Road_NO_LANES;  // assigned in __init__.py
    size_t Road_NO_DIRS;  // assigned in __init__.py

    // Gen
    std::string Gen_TRAFFIC_FOLDER = "Traffic";
    bool Gen_GEN_TRAFFIC = false;
    size_t Gen_NO_DAYS = 1;
    double Gen_TRUCK_TRACK_WIDTH = 190;  // grave gen only
    double Gen_LANE_ECCENTRICITY_STD = 0.0;
    double Gen_NO_OVERLAP_LENGTH;  // assigned in __init__.py
    bool Gen_GEN_CAR = true;

    // Read
    bool Read_READ_FILE = false;
    std::string Read_TRAFFIC_FILE = "traffic.txt";
    std::string Read_GARAGE_FILE = "garage.txt";
    std::string Read_KERNEL_FILE = "kernel.csv";
    std::string Read_CONSTANT_FILE = "constant_vehicle.csv";
    unsigned int Read_FILE_FORMAT = 0.0;
    bool Read_USE_CONSTANT_SPEED = false;
    bool Read_USE_AVE_SPEED = false;
    double Read_CONST_SPEED = 0.0;

    // Traffic
    int Traffic_VEHICLE_MODEL = 4;
    int Traffic_HEADWAY_MODEL = 6;
    int Traffic_CLASSIFICATION = 1;
    double Traffic_CONGESTED_SPACING = 26.1;
    double Traffic_CONGESTED_SPEED = 49.7;
    double Traffic_CONGESTED_GAP = 0.0;
    double Traffic_CONGESTED_GAP_COEF_VAR = 0.05;
    double Traffic_CONSTANT_SPEED = 36.0;
    double Traffic_CONSTANT_GAP = 10.0;

    // Sim
    bool Sim_CALC_LOAD_EFFECTS = false;
    std::string Sim_BRIDGE_FILE = "Bridge.txt";
    std::string Sim_INFLINE_FILE = "IL.txt";
    std::string Sim_INFSURF_FILE = "IS.txt";
    double Sim_CALC_TIME_STEP = 0.1;
    int Sim_MIN_GVW = 35;

    // Output
    bool Output_WRITE_TIME_HISTORY = false;
    bool Output_WRITE_EACH_EVENT = false;
    int Output_WRITE_EVENT_BUFFER_SIZE = 10000;
    bool Output_WRITE_FATIGUE_EVENT = false;

    // Output VehicleFile
    bool Output_VehicleFile_WRITE_VEHICLE_FILE = false;
    unsigned int Output_VehicleFile_FILE_FORMAT = 4;
    std::string Output_VehicleFile_VEHICLE_FILENAME = "Vehicle.txt";
    int Output_VehicleFile_WRITE_VEHICLE_BUFFER_SIZE = 10000;
    bool Output_VehicleFile_WRITE_FLOW_STATS = false;

    // Output BlockMax
    bool Output_BlockMax_WRITE_BM = false;
    bool Output_BlockMax_WRITE_BM_VEHICLES = false;
    bool Output_BlockMax_WRITE_BM_SUMMARY = false;
    bool Output_BlockMax_WRITE_BM_MIXED = false;
    int Output_BlockMax_BLOCK_SIZE_DAYS = 1;
    int Output_BlockMax_BLOCK_SIZE_SECS = 0;
    int Output_BlockMax_WRITE_BM_BUFFER_SIZE = 10000;

    // Output POT
    bool Output_POT_WRITE_POT = false;
    bool Output_POT_WRITE_POT_VEHICLES = false;
    bool Output_POT_WRITE_POT_SUMMARY = false;
    bool Output_POT_WRITE_POT_COUNTER = false;
    int Output_POT_POT_COUNT_SIZE_DAYS = 1;
    int Output_POT_POT_COUNT_SIZE_SECS = 0;
    int Output_POT_WRITE_POT_BUFFER_SIZE = 10000;

    // Output Stats
    bool Output_Stats_WRITE_STATS = false;
    bool Output_Stats_WRITE_SS_CUMULATIVE = false;
    bool Output_Stats_WRITE_SS_INTERVALS = false;
    int Output_Stats_WRITE_SS_INTERVAL_SIZE = 3600;
    int Output_Stats_WRITE_SS_BUFFER_SIZE = 10000;

    // Output Fatigue
    bool Output_Fatigue_DO_FATIGUE_RAINFLOW = false;
    int Output_Fatigue_RAINFLOW_DECIMAL = 3;
    double Output_Fatigue_RAINFLOW_CUTOFF = 0.0;
    int Output_Fatigue_WRITE_RAINFLOW_BUFFER_SIZE = 10000;


    // functions
    void VehGenGrave(std::string lanes_file, std::string traffic_folder, size_t no_days, double truck_track_width, double lane_eccentricity_std);
    void VehGenGarage(std::string lanes_file, std::string garage_file, unsigned int file_format, std::string kernel_file, size_t no_days, double lane_eccentricity_std);
    void VehGenConstant(std::string lanes_file, std::string constant_file, size_t no_days, double lane_eccentricity_std);

    void FlowGenNHM(int classification, std::string traffic_folder, bool gen_car=false);
    void FlowGenConstant(int classification, double constant_speed, double constant_gap, bool gen_car=true);
    void FlowGenCongestion(int classification, double congested_spacing, double congested_speed, double congested_gap_coef_var, bool gen_car=true);
    void FlowGenFreeFlow(int classification, bool gen_car=true);

    void TrafficRead(std::string traffic_file, unsigned int file_format, bool use_constant_speed, bool use_ave_speed, double const_speed);

    void Simulation(std::string bridge_file, std::string infline_file, std::string infsurf_file, double calc_time_step, int min_gvw);

    void OutputGeneral(bool write_time_history=false, bool write_each_event=false, bool write_fatigue_event=false, int write_buffer_size=10000);
    void OutputVehicleFile(bool write_vehicle_file=false, unsigned int vehicle_file_format=4, std::string vehicle_file_name="Vehicles.txt", int buffer_size=10000, bool write_flow_stats=false);
    void OutputBlockMax(bool write_blockmax=false, bool write_vehicle=false, bool write_summary=false, bool write_mixed=false, int block_size_days=1, int block_size_secs=0, int buffer_size=10000);
    void OutputPOT(bool write_pot=false, bool write_vehicle=false, bool write_summary=false, bool write_counter=false, int pot_size_days=1, int pot_size_secs=0, int buffer_size=10000);
    void OutputStats(bool write_stats=false, bool write_cumulative=false, bool write_intervals=false, int interval_size=3600, int buffer_size=10000);
    void OutputFatigue(bool do_rainflow=false, int decimal=3, double cut_off_value=0.0, int buffer_size=10000);


private:
    CCSVParse m_CSV;
    std::string m_CommentString;

    void doDerivedConstants();
    void ExtractData();
    std::string GetNextDataLine();

};
