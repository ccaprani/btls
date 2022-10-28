#include "PyConfigData.h"


using namespace std;

CPyConfigData::CPyConfigData() :m_CommentString("//") {}

CPyConfigData::~CPyConfigData() {}


bool CPyConfigData::ReadData(string in_file)
{
    if (m_CSV.OpenFile(in_file, ","))
    {
        ExtractData();
        return true;
    }
    return false;
}

void CPyConfigData::ExtractData()
{
    string str;
    str = GetNextDataLine();    Mode_PROGRAM_MODE = m_CSV.stringToInt(str);

    str = GetNextDataLine();    Gen_NO_DAYS = m_CSV.stringToInt(str);
    str = GetNextDataLine();    Gen_TRAFFIC_FOLDER = str;
    str = GetNextDataLine();    Gen_TRUCK_TRACK_WIDTH = m_CSV.stringToDouble(str);
    str = GetNextDataLine();    Gen_LANE_ECCENTRICITY_STD = m_CSV.stringToDouble(str);
    str = GetNextDataLine();    Traffic_VEHICLE_MODEL = m_CSV.stringToInt(str);
    str = GetNextDataLine();    Traffic_HEADWAY_MODEL = m_CSV.stringToInt(str);
    str = GetNextDataLine();    Traffic_CLASSIFICATION = m_CSV.stringToInt(str);

    str = GetNextDataLine();    Road_LANES_FILE = str;
    str = GetNextDataLine();    Traffic_CONGESTED_SPACING = m_CSV.stringToDouble(str);
    str = GetNextDataLine();    Traffic_CONGESTED_SPEED = m_CSV.stringToDouble(str);
    str = GetNextDataLine();    Traffic_CONGESTED_GAP_COEF_VAR = m_CSV.stringToDouble(str);
    str = GetNextDataLine();    Traffic_CONSTANT_SPEED = m_CSV.stringToDouble(str);
    str = GetNextDataLine();    Traffic_CONSTANT_GAP = m_CSV.stringToDouble(str);

    str = GetNextDataLine();    Read_GARAGE_FILE = str;
    str = GetNextDataLine();    Read_KERNEL_FILE = str;
    str = GetNextDataLine();    Read_CONSTANT_FILE = str;
    str = GetNextDataLine();    Read_TRAFFIC_FILE = str;
    str = GetNextDataLine();    Read_FILE_FORMAT = m_CSV.stringToInt(str);
    str = GetNextDataLine();    Read_USE_CONSTANT_SPEED = m_CSV.stringToBool(str);
    str = GetNextDataLine();    Read_USE_AVE_SPEED = m_CSV.stringToBool(str);
    str = GetNextDataLine();    Read_CONST_SPEED = m_CSV.stringToDouble(str);

    str = GetNextDataLine();    Sim_BRIDGE_FILE = str;
    str = GetNextDataLine();    Sim_INFLINE_FILE = str;
    str = GetNextDataLine();    Sim_INFSURF_FILE = str;
    str = GetNextDataLine();    Sim_CALC_TIME_STEP = m_CSV.stringToDouble(str);
    str = GetNextDataLine();    Sim_MIN_GVW = m_CSV.stringToInt(str);

    str = GetNextDataLine();    Output_WRITE_TIME_HISTORY = m_CSV.stringToBool(str);
    str = GetNextDataLine();    Output_WRITE_EACH_EVENT = m_CSV.stringToBool(str);
    str = GetNextDataLine();    Output_WRITE_EVENT_BUFFER_SIZE = m_CSV.stringToInt(str);
    str = GetNextDataLine();    Output_WRITE_FATIGUE_EVENT = m_CSV.stringToBool(str);

    str = GetNextDataLine();    Output_Fatigue_DO_FATIGUE_RAINFLOW = m_CSV.stringToBool(str);
    str = GetNextDataLine();    Output_Fatigue_RAINFLOW_DECIMAL = m_CSV.stringToInt(str);
    str = GetNextDataLine();    Output_Fatigue_RAINFLOW_CUTOFF = m_CSV.stringToDouble(str);
    str = GetNextDataLine();    Output_Fatigue_WRITE_RAINFLOW_BUFFER_SIZE = m_CSV.stringToDouble(str);

    str = GetNextDataLine();    Output_VehicleFile_WRITE_VEHICLE_FILE = m_CSV.stringToBool(str);
    str = GetNextDataLine();    Output_VehicleFile_FILE_FORMAT = m_CSV.stringToInt(str);
    str = GetNextDataLine();    Output_VehicleFile_VEHICLE_FILENAME = str;
    str = GetNextDataLine();    Output_VehicleFile_WRITE_VEHICLE_BUFFER_SIZE = m_CSV.stringToInt(str);
    str = GetNextDataLine();    Output_VehicleFile_WRITE_FLOW_STATS = m_CSV.stringToBool(str);

    str = GetNextDataLine();    Output_BlockMax_WRITE_BM = m_CSV.stringToBool(str);
    str = GetNextDataLine();    Output_BlockMax_BLOCK_SIZE_DAYS = m_CSV.stringToInt(str);
    str = GetNextDataLine();    Output_BlockMax_BLOCK_SIZE_SECS = m_CSV.stringToInt(str);
    str = GetNextDataLine();    Output_BlockMax_WRITE_BM_VEHICLES = m_CSV.stringToBool(str);
    str = GetNextDataLine();    Output_BlockMax_WRITE_BM_SUMMARY = m_CSV.stringToBool(str);
    str = GetNextDataLine();    Output_BlockMax_WRITE_BM_MIXED = m_CSV.stringToBool(str);
    str = GetNextDataLine();    Output_BlockMax_WRITE_BM_BUFFER_SIZE = m_CSV.stringToInt(str);

    str = GetNextDataLine();    Output_POT_WRITE_POT = m_CSV.stringToBool(str);
    str = GetNextDataLine();    Output_POT_WRITE_POT_VEHICLES = m_CSV.stringToBool(str);
    str = GetNextDataLine();    Output_POT_WRITE_POT_SUMMARY = m_CSV.stringToBool(str);
    str = GetNextDataLine();    Output_POT_WRITE_POT_COUNTER = m_CSV.stringToBool(str);
    str = GetNextDataLine();    Output_POT_POT_COUNT_SIZE_DAYS = m_CSV.stringToInt(str);
    str = GetNextDataLine();    Output_POT_POT_COUNT_SIZE_SECS = m_CSV.stringToInt(str);
    str = GetNextDataLine();    Output_POT_WRITE_POT_BUFFER_SIZE = m_CSV.stringToInt(str);

    str = GetNextDataLine();    Output_Stats_WRITE_STATS = m_CSV.stringToBool(str);
    str = GetNextDataLine();    Output_Stats_WRITE_SS_CUMULATIVE = m_CSV.stringToBool(str);
    str = GetNextDataLine();    Output_Stats_WRITE_SS_INTERVALS = m_CSV.stringToBool(str);
    str = GetNextDataLine();    Output_Stats_WRITE_SS_INTERVAL_SIZE = m_CSV.stringToInt(str);
    str = GetNextDataLine();    Output_Stats_WRITE_SS_BUFFER_SIZE = m_CSV.stringToInt(str);

    m_CSV.CloseFile();

    doDerivedConstants();
}

void CPyConfigData::doDerivedConstants()
{
    switch (Mode_PROGRAM_MODE)
    {
    case 1:
        Gen_GEN_TRAFFIC = true;
        Read_READ_FILE = false;
        Sim_CALC_LOAD_EFFECTS = true;
        break;
    case 2:
        Gen_GEN_TRAFFIC = true;
        Sim_CALC_LOAD_EFFECTS = false;
        Read_READ_FILE = false;
        break;
    case 3:
        Gen_GEN_TRAFFIC = false;
        Sim_CALC_LOAD_EFFECTS = true;
        Read_READ_FILE = true;
        break;
    }

    if (Mode_PROGRAM_MODE == 2)
        Output_VehicleFile_WRITE_VEHICLE_FILE = true; // regardless of user input

    Traffic_CONGESTED_SPEED = Traffic_CONGESTED_SPEED / 3.6; // km/h to m/s
    Traffic_CONGESTED_GAP = Traffic_CONGESTED_SPACING / Traffic_CONGESTED_SPEED;

    Traffic_CONSTANT_SPEED = Traffic_CONSTANT_SPEED / 3.6; // km/h to m/s
}

string CPyConfigData::GetNextDataLine()
{
    string line;
    m_CSV.getline(line);
    while (line.substr(0, 2) == m_CommentString)
        m_CSV.getline(line);

    return line;
}


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

void CPyConfigData::FlowGenNHM(int classification, string traffic_folder, bool gen_car) {
    Gen_GEN_TRAFFIC = true;
    Read_READ_FILE = false;
    Traffic_HEADWAY_MODEL = 0;
    Traffic_CLASSIFICATION = classification;  // 0 no_ of axle, 1 axle pattern
    Gen_TRAFFIC_FOLDER = traffic_folder;
    Gen_GEN_CAR = gen_car;
}

void CPyConfigData::FlowGenConstant(int classification, double constant_speed, double constant_gap, bool gen_car) {
    Gen_GEN_TRAFFIC = true;
    Read_READ_FILE = false;
    Traffic_HEADWAY_MODEL = 1;
    Traffic_CLASSIFICATION = classification;  // 0 no_ of axle, 1 axle pattern
    Traffic_CONSTANT_SPEED = constant_speed/3.6;  // km/h to m/s
    Traffic_CONSTANT_GAP = constant_gap;
    Gen_GEN_CAR = gen_car;
}

void CPyConfigData::FlowGenCongestion(int classification, double congested_spacing, double congested_speed, double congested_gap_coef_var, bool gen_car) {
    Gen_GEN_TRAFFIC = true;
    Read_READ_FILE = false;
    Traffic_HEADWAY_MODEL = 5;
    Traffic_CLASSIFICATION = classification;  // 0 no_ of axle, 1 axle pattern
    Traffic_CONGESTED_SPACING = congested_spacing;
    Traffic_CONGESTED_SPEED = congested_speed/3.6;  // km/h to m/s
	Traffic_CONGESTED_GAP = Traffic_CONGESTED_SPACING/Traffic_CONGESTED_SPEED;
    Traffic_CONGESTED_GAP_COEF_VAR = congested_gap_coef_var;
    Gen_GEN_CAR = gen_car;
}

void CPyConfigData::FlowGenFreeFlow(int classification, bool gen_car) {
    Gen_GEN_TRAFFIC = true;
    Read_READ_FILE = false;
    Traffic_HEADWAY_MODEL = 6;
    Traffic_CLASSIFICATION = classification;  // 0 no_ of axle, 1 axle pattern
    Gen_GEN_CAR = gen_car;
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

