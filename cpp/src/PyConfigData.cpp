#include "PyConfigData.h"


using namespace std;

CPyConfigData::CPyConfigData() {}

CPyConfigData::~CPyConfigData() {}


void CPyConfigData::VehGenGrave(string lanes_file, string traffic_folder, size_t no_days, double truck_track_width, double lane_eccentricity_std) {
    Gen_GEN_TRAFFIC = true;
    Read_READ_FILE = false;
    Road_LANES_FILE = lanes_file;
    Gen_TRAFFIC_FOLDER = traffic_folder;
    Gen_NO_DAYS = no_days;
    Gen_TRUCK_TRACK_WIDTH = truck_track_width;
    Gen_LANE_ECCENTRICITY_STD = lane_eccentricity_std;
    Traffic_VEHICLE_MODEL = 0;
}

void CPyConfigData::VehGenGarage(string lanes_file, string garage_file, unsigned int file_format, string kernel_file, size_t no_days, double lane_eccentricity_std) {
    Gen_GEN_TRAFFIC = true;
    Read_READ_FILE = false;
    Road_LANES_FILE = lanes_file;
    Gen_NO_DAYS = no_days;
    Gen_LANE_ECCENTRICITY_STD = lane_eccentricity_std;
    Read_GARAGE_FILE = garage_file;
    Read_KERNEL_FILE = kernel_file;
    Read_FILE_FORMAT = file_format;
    Traffic_VEHICLE_MODEL = 2;
}

void CPyConfigData::VehGenConstant(string lanes_file, string constant_file, size_t no_days, double lane_eccentricity_std) {
    Gen_GEN_TRAFFIC = true;
    Read_READ_FILE = false;
    Road_LANES_FILE = lanes_file;
    Gen_NO_DAYS = no_days;
    Gen_LANE_ECCENTRICITY_STD = lane_eccentricity_std;
    Read_CONSTANT_FILE = constant_file;
    Traffic_VEHICLE_MODEL = 1;
}

// no car
void CPyConfigData::FlowGenNHM(int classification) {
    Gen_GEN_TRAFFIC = true;
    Read_READ_FILE = false;
    Traffic_HEADWAY_MODEL = 0;
    Traffic_CLASSIFICATION = classification;  // 0 no. of axle, 1 axle pattern
}

// has car
void CPyConfigData::FlowGenConstant(int classification) {
    Gen_GEN_TRAFFIC = true;
    Read_READ_FILE = false;
    Traffic_HEADWAY_MODEL = 1;
    Traffic_CLASSIFICATION = classification;  // 0 no. of axle, 1 axle pattern
}

// has car
void CPyConfigData::FlowGenCongestion(int classification, double congested_spacing, double congested_speed, double congested_gap_coef_var) {
    Gen_GEN_TRAFFIC = true;
    Read_READ_FILE = false;
    Traffic_HEADWAY_MODEL = 5;
    Traffic_CLASSIFICATION = classification;  // 0 no. of axle, 1 axle pattern
    Traffic_CONGESTED_SPACING = congested_spacing;
    Traffic_CONGESTED_SPEED = congested_speed/3.6;  // km/h to m/s
	Traffic_CONGESTED_GAP = Traffic_CONGESTED_SPACING/Traffic_CONGESTED_SPEED;
    Traffic_CONGESTED_GAP_COEF_VAR = congested_gap_coef_var;
}

// has car
void CPyConfigData::FlowGenFreeFlow(int classification) {
    Gen_GEN_TRAFFIC = true;
    Read_READ_FILE = false;
    Traffic_HEADWAY_MODEL = 6;
    Traffic_CLASSIFICATION = classification;  // 0 no. of axle, 1 axle pattern
}

void CPyConfigData::TrafficRead(string traffic_file, unsigned int file_format, bool use_constant_speed, bool use_ave_speed, double const_speed) {
    Gen_GEN_TRAFFIC = false;
    Read_READ_FILE = true;
    Read_TRAFFIC_FILE = traffic_file;
    Read_FILE_FORMAT = file_format;
    Read_USE_CONSTANT_SPEED = use_constant_speed;
    Read_USE_AVE_SPEED = use_ave_speed;
    Read_CONST_SPEED = const_speed;
}

void CPyConfigData::Simulation(string bridge_file, string infline_file, string infsurf_file, double calc_time_step, int min_gvw) {
    Sim_CALC_LOAD_EFFECTS = true;
    Sim_BRIDGE_FILE = bridge_file;
    Sim_INFLINE_FILE = infline_file;
    Sim_INFSURF_FILE = infsurf_file;
    Sim_CALC_TIME_STEP = calc_time_step;
    Sim_MIN_GVW = min_gvw;  // tonne*10
}

void CPyConfigData::OutputGeneral(bool write_time_history, bool write_each_event, bool write_fatigue_event, int write_buffer_size) {
    Output_WRITE_TIME_HISTORY = write_time_history;
    Output_WRITE_EACH_EVENT = write_each_event;
    Output_WRITE_EVENT_BUFFER_SIZE = write_fatigue_event;
    Output_WRITE_FATIGUE_EVENT = write_buffer_size;
}

void CPyConfigData::OutputVehicleFile(bool write_vehicle_file, unsigned int vehicle_file_format, string vehicle_file_name, int buffer_size, bool write_flow_stats) {
    Output_VehicleFile_WRITE_VEHICLE_FILE = write_vehicle_file;
    Output_VehicleFile_FILE_FORMAT = vehicle_file_format;
    Output_VehicleFile_VEHICLE_FILENAME = vehicle_file_name;
    Output_VehicleFile_WRITE_VEHICLE_BUFFER_SIZE = buffer_size;
    Output_VehicleFile_WRITE_FLOW_STATS = write_flow_stats;
}

void CPyConfigData::OutputBlockMax(bool write_blockmax, bool write_vehicle, bool write_summary, bool write_mixed, int block_size_days, int block_size_secs, int buffer_size) {
    Output_BlockMax_WRITE_BM = write_blockmax;
    Output_BlockMax_WRITE_BM_VEHICLES = write_vehicle;
    Output_BlockMax_WRITE_BM_SUMMARY = write_summary;
    Output_BlockMax_WRITE_BM_MIXED = write_mixed;
    Output_BlockMax_BLOCK_SIZE_DAYS = block_size_days;
    Output_BlockMax_BLOCK_SIZE_SECS = block_size_secs;
    Output_BlockMax_WRITE_BM_BUFFER_SIZE = buffer_size;
}

void CPyConfigData::OutputPOT(bool write_pot, bool write_vehicle, bool write_summary, bool write_counter, int pot_size_days, int pot_size_secs, int buffer_size) {
    Output_POT_WRITE_POT = write_pot;
    Output_POT_WRITE_POT_VEHICLES = write_vehicle;
    Output_POT_WRITE_POT_SUMMARY = write_summary;
    Output_POT_WRITE_POT_COUNTER = write_counter;
    Output_POT_POT_COUNT_SIZE_DAYS = pot_size_days;
    Output_POT_POT_COUNT_SIZE_SECS = pot_size_secs;
    Output_POT_WRITE_POT_BUFFER_SIZE = buffer_size;
}

void CPyConfigData::OutputStats(bool write_stats, bool write_cumulative, bool write_intervals, int interval_size, int buffer_size) {
    Output_Stats_WRITE_STATS = write_stats;
    Output_Stats_WRITE_SS_CUMULATIVE = write_cumulative;
    Output_Stats_WRITE_SS_INTERVALS = write_intervals;
    Output_Stats_WRITE_SS_INTERVAL_SIZE = interval_size;
    Output_Stats_WRITE_SS_BUFFER_SIZE = buffer_size;
}

void CPyConfigData::OutputFatigue(bool do_rainflow, int decimal, double cut_off_value, int buffer_size) {
    Output_Fatigue_DO_FATIGUE_RAINFLOW = do_rainflow;
    Output_Fatigue_RAINFLOW_DECIMAL = decimal;
    Output_Fatigue_RAINFLOW_CUTOFF = cut_off_value;
    Output_Fatigue_WRITE_RAINFLOW_BUFFER_SIZE = buffer_size;
}

