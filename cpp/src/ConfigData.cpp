// ConfigData.cpp: implementation of the CConfigDataCore class.
//
//////////////////////////////////////////////////////////////////////

#include "ConfigData.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConfigDataCore::CConfigDataCore(): m_CommentString("//")
{

}

bool CConfigDataCore::ReadData(std::string inFile)
{
	if( m_CSV.OpenFile(inFile, ",") )
	{
		ExtractData();
		return true;
	}
	return false;
}

void CConfigDataCore::ExtractData()
{
	string str;
	str = GetNextDataLine();	Mode.PROGRAM_MODE				= m_CSV.stringToInt(	str);

	str = GetNextDataLine();	Gen.NO_DAYS						= m_CSV.stringToInt(	str);
	str = GetNextDataLine();	Gen.TRAFFIC_FOLDER				= str;
	str = GetNextDataLine();	Gen.TRUCK_TRACK_WIDTH			= m_CSV.stringToDouble( str);
	str = GetNextDataLine();	Gen.LANE_ECCENTRICITY_STD		= m_CSV.stringToDouble( str);
	str = GetNextDataLine();	Traffic.VEHICLE_MODEL			= m_CSV.stringToInt(	str);
	str = GetNextDataLine();	Traffic.HEADWAY_MODEL			= m_CSV.stringToInt(	str);
	str = GetNextDataLine();	Traffic.CLASSIFICATION			= m_CSV.stringToInt(str);
	
	str = GetNextDataLine();	Road.LANES_FILE					= str;
	str = GetNextDataLine();	Traffic.CONGESTED_SPACING		= m_CSV.stringToDouble( str);
	str = GetNextDataLine();	Traffic.CONGESTED_SPEED			= m_CSV.stringToDouble( str);
	str = GetNextDataLine();	Traffic.CONGESTED_GAP_COEF_VAR	= m_CSV.stringToDouble( str);
	str = GetNextDataLine();	Traffic.CONSTANT_SPEED 			= m_CSV.stringToDouble(str);
    str = GetNextDataLine();    Traffic.CONSTANT_GAP 			= m_CSV.stringToDouble(str);

	str = GetNextDataLine();	Read.GARAGE_FILE				= str;
	str = GetNextDataLine();	Read.KERNEL_FILE				= str;
	str = GetNextDataLine();	Read.CONSTANT_FILE				= str;
	str = GetNextDataLine();	Read.TRAFFIC_FILE				= str;	
	str = GetNextDataLine();	Read.FILE_FORMAT				= m_CSV.stringToInt(	str);
	str = GetNextDataLine();	Read.USE_CONSTANT_SPEED			= m_CSV.stringToBool(	str);
	str = GetNextDataLine();	Read.USE_AVE_SPEED				= m_CSV.stringToBool(	str);
	str = GetNextDataLine();	Read.CONST_SPEED				= m_CSV.stringToDouble( str);

	str = GetNextDataLine();	Sim.BRIDGE_FILE					= str;
	str = GetNextDataLine();	Sim.INFLINE_FILE				= str;
	str = GetNextDataLine();	Sim.INFSURF_FILE				= str;
	str = GetNextDataLine();	Sim.CALC_TIME_STEP				= m_CSV.stringToDouble( str);
	str = GetNextDataLine();	Sim.MIN_GVW						= m_CSV.stringToInt(	str);
	
	str = GetNextDataLine();	Output.WRITE_TIME_HISTORY		= m_CSV.stringToBool(	str);
	str = GetNextDataLine();	Output.WRITE_EACH_EVENT			= m_CSV.stringToBool(	str);	
	str = GetNextDataLine();	Output.WRITE_EVENT_BUFFER_SIZE	= m_CSV.stringToInt(	str);
	str = GetNextDataLine();	Output.WRITE_FATIGUE_EVENT		= m_CSV.stringToBool(	str);
	
	str = GetNextDataLine();	Output.Fatigue.DO_FATIGUE_RAINFLOW			= m_CSV.stringToBool(	str);
	str = GetNextDataLine();	Output.Fatigue.RAINFLOW_DECIMAL				= m_CSV.stringToInt(	str);
	str = GetNextDataLine();	Output.Fatigue.RAINFLOW_CUTOFF				= m_CSV.stringToDouble(	str);
	str = GetNextDataLine();	Output.Fatigue.WRITE_RAINFLOW_BUFFER_SIZE	= m_CSV.stringToDouble(	str);
	
	str = GetNextDataLine();	Output.VehicleFile.WRITE_VEHICLE_FILE		= m_CSV.stringToBool(	str);
	str = GetNextDataLine();	Output.VehicleFile.FILE_FORMAT				= m_CSV.stringToInt(	str);
	str = GetNextDataLine();	Output.VehicleFile.VEHICLE_FILENAME			= str;
	str = GetNextDataLine();	Output.VehicleFile.WRITE_VEHICLE_BUFFER_SIZE= m_CSV.stringToInt(	str);
	str = GetNextDataLine();	Output.VehicleFile.WRITE_FLOW_STATS			= m_CSV.stringToBool(	str);

	str = GetNextDataLine();	Output.BlockMax.WRITE_BM			= m_CSV.stringToBool(	str);
	str = GetNextDataLine();	Output.BlockMax.BLOCK_SIZE_DAYS		= m_CSV.stringToInt(	str);
	str = GetNextDataLine();	Output.BlockMax.BLOCK_SIZE_SECS		= m_CSV.stringToInt(	str);	
	str = GetNextDataLine();	Output.BlockMax.WRITE_BM_VEHICLES	= m_CSV.stringToBool(	str);
	str = GetNextDataLine();	Output.BlockMax.WRITE_BM_SUMMARY	= m_CSV.stringToBool(	str);
	str = GetNextDataLine();	Output.BlockMax.WRITE_BM_MIXED		= m_CSV.stringToBool(	str);
	str = GetNextDataLine();	Output.BlockMax.WRITE_BM_BUFFER_SIZE= m_CSV.stringToInt(	str);
	
	str = GetNextDataLine();	Output.POT.WRITE_POT				= m_CSV.stringToBool(	str);
	str = GetNextDataLine();	Output.POT.WRITE_POT_VEHICLES		= m_CSV.stringToBool(	str);
	str = GetNextDataLine();	Output.POT.WRITE_POT_SUMMARY		= m_CSV.stringToBool(	str);
	str = GetNextDataLine();	Output.POT.WRITE_POT_COUNTER		= m_CSV.stringToBool(	str);
	str = GetNextDataLine();	Output.POT.POT_COUNT_SIZE_DAYS		= m_CSV.stringToInt(	str);
	str = GetNextDataLine();	Output.POT.POT_COUNT_SIZE_SECS		= m_CSV.stringToInt(	str);
	str = GetNextDataLine();	Output.POT.WRITE_POT_BUFFER_SIZE	= m_CSV.stringToInt(	str);

	str = GetNextDataLine();	Output.Stats.WRITE_STATS			= m_CSV.stringToBool(	str);
	str = GetNextDataLine();	Output.Stats.WRITE_SS_CUMULATIVE	= m_CSV.stringToBool(	str);
	str = GetNextDataLine();	Output.Stats.WRITE_SS_INTERVALS		= m_CSV.stringToBool(	str);
	str = GetNextDataLine();	Output.Stats.WRITE_SS_INTERVAL_SIZE	= m_CSV.stringToInt(	str);
	str = GetNextDataLine();	Output.Stats.WRITE_SS_BUFFER_SIZE	= m_CSV.stringToInt(	str);

	m_CSV.CloseFile();

	doDerivedConstants();
}

string CConfigDataCore::GetNextDataLine()
{
	string line;
	m_CSV.getline(line);
	while(line.substr(0,2) == m_CommentString)
		m_CSV.getline(line);

	return line;
}
/*
vector<double> CConfigDataCore::GetVectorFromNextLine()
{
	vector<double> vData;
	string line;
	m_CSV.getline(line);
	while(line.substr(0,2) == m_CommentString)
		m_CSV.getline(line);

	size_t nData = m_CSV.getnfield();	// minus 1 if comma to be at the end is counted
	for (size_t j = 0; j < nData; j++)
	{
		string str = m_CSV.getfield(j);
		double val = m_CSV.stringToDouble(str);
		vData.push_back(val);
	}

	return vData;
}
*/

void CConfigDataCore::doDerivedConstants()
{
	switch(Mode.PROGRAM_MODE)
	{
	case 1:
		Gen.GEN_TRAFFIC = true;
		Read.READ_FILE = false;
		Sim.CALC_LOAD_EFFECTS = true;
		break;
	case 2:
		Gen.GEN_TRAFFIC = true;
		Sim.CALC_LOAD_EFFECTS = false;
		Read.READ_FILE = false;
		break;
	case 3:
		Gen.GEN_TRAFFIC = false;
		Sim.CALC_LOAD_EFFECTS = true;
		Read.READ_FILE = true;
		break;
	}

	if(Mode.PROGRAM_MODE == 2)
		Output.VehicleFile.WRITE_VEHICLE_FILE = true;	// regardless of user input

	Traffic.CONGESTED_SPEED = Traffic.CONGESTED_SPEED/3.6;	// km/h to m/s
	Traffic.CONGESTED_GAP = Traffic.CONGESTED_SPACING/Traffic.CONGESTED_SPEED;
}
/*
string CConfigDataCore::returnInt(int i) 
{
	string a;
	switch(i)
	{
	case 1: { a = 'a'; break;};
	case 2: { a = 'b'; break;};
	case 3: { a = 'c'; break;};
	case 4: { a = 'd'; break;};
	case 5: { a = 'e'; break;};
	case 6: { a = 'f'; break;};
	case 7: { a = 'g'; break;};
	case 8: { a = 'h'; break;};
	case 9: { a = 'i'; break;};
	case 10: { a = 'j'; break;};
	case 11: { a = 'k'; break;};
	case 12: { a = 'l'; break;};
	case 13: { a = 'm'; break;};
	}
	return a;
}
*/

void CConfigDataCore::VehGenGrave(string lanes_file, string traffic_folder, size_t no_days, double truck_track_width, double lane_eccentricity_std) 
{
    Gen.GEN_TRAFFIC = true;
    Read.READ_FILE = false;
    Road.LANES_FILE = lanes_file;
    Gen.TRAFFIC_FOLDER = traffic_folder;
    Gen.NO_DAYS = no_days;
    Gen.TRUCK_TRACK_WIDTH = truck_track_width;
    Gen.LANE_ECCENTRICITY_STD = lane_eccentricity_std;
    Traffic.VEHICLE_MODEL = 0;
}

void CConfigDataCore::VehGenGarage(string lanes_file, string garage_file, unsigned int file_format, string kernel_file, size_t no_days, double lane_eccentricity_std) 
{
    Gen.GEN_TRAFFIC = true;
    Read.READ_FILE = false;
    Road.LANES_FILE = lanes_file;
    Gen.NO_DAYS = no_days;
    Gen.LANE_ECCENTRICITY_STD = lane_eccentricity_std;
    Read.GARAGE_FILE = garage_file;
    Read.KERNEL_FILE = kernel_file;
    Read.FILE_FORMAT = file_format;
    Traffic.VEHICLE_MODEL = 2;
}

void CConfigDataCore::VehGenConstant(string lanes_file, string constant_file, size_t no_days, double lane_eccentricity_std) 
{
    Gen.GEN_TRAFFIC = true;
    Read.READ_FILE = false;
    Road.LANES_FILE = lanes_file;
    Gen.NO_DAYS = no_days;
    Gen.LANE_ECCENTRICITY_STD = lane_eccentricity_std;
    Read.CONSTANT_FILE = constant_file;
    Traffic.VEHICLE_MODEL = 1;
}

void CConfigDataCore::FlowGenNHM(int classification, string traffic_folder, bool gen_car) 
{
    Gen.GEN_TRAFFIC = true;
    Read.READ_FILE = false;
    Traffic.HEADWAY_MODEL = 0;
    Traffic.CLASSIFICATION = classification;  // 0 no_ of axle, 1 axle pattern
    Gen.TRAFFIC_FOLDER = traffic_folder;
}

void CConfigDataCore::FlowGenConstant(int classification, double constant_speed, double constant_gap, bool gen_car) 
{
    Gen.GEN_TRAFFIC = true;
    Read.READ_FILE = false;
    Traffic.HEADWAY_MODEL = 1;
    Traffic.CLASSIFICATION = classification;  // 0 no_ of axle, 1 axle pattern
    Traffic.CONSTANT_SPEED = constant_speed/3.6;  // km/h to m/s
    Traffic.CONSTANT_GAP = constant_gap;
    Traffic.GEN_CAR = gen_car;
}

void CConfigDataCore::FlowGenCongestion(int classification, double congested_spacing, double congested_speed, double congested_gap_coef_var, bool gen_car) 
{
    Gen.GEN_TRAFFIC = true;
    Read.READ_FILE = false;
    Traffic.HEADWAY_MODEL = 5;
    Traffic.CLASSIFICATION = classification;  // 0 no. of axle, 1 axle pattern
    Traffic.CONGESTED_SPACING = congested_spacing;
    Traffic.CONGESTED_SPEED = congested_speed/3.6;  // km/h to m/s
	Traffic.CONGESTED_GAP = Traffic.CONGESTED_SPACING/Traffic.CONGESTED_SPEED;
    Traffic.CONGESTED_GAP_COEF_VAR = congested_gap_coef_var;
    Traffic.GEN_CAR = gen_car;
}

void CConfigDataCore::FlowGenFreeFlow(int classification, bool gen_car) 
{
    Gen.GEN_TRAFFIC = true;
    Read.READ_FILE = false;
    Traffic.HEADWAY_MODEL = 6;
    Traffic.CLASSIFICATION = classification;  // 0 no. of axle, 1 axle pattern
    Traffic.GEN_CAR = gen_car;
}


void CConfigDataCore::TrafficRead(string traffic_file, unsigned int file_format, bool use_constant_speed, bool use_ave_speed, double const_speed) 
{
    Gen.GEN_TRAFFIC = false;
    Read.READ_FILE = true;
    Read.TRAFFIC_FILE = traffic_file;
    Read.FILE_FORMAT = file_format;
    Read.USE_CONSTANT_SPEED = use_constant_speed;
    Read.USE_AVE_SPEED = use_ave_speed;
    Read.CONST_SPEED = const_speed;
}


void CConfigDataCore::Simulation(string bridge_file, string infline_file, string infsurf_file, double calc_time_step, int min_gvw) 
{
    Sim.CALC_LOAD_EFFECTS = true;
    Sim.BRIDGE_FILE = bridge_file;
    Sim.INFLINE_FILE = infline_file;
    Sim.INFSURF_FILE = infsurf_file;
    Sim.CALC_TIME_STEP = calc_time_step;
    Sim.MIN_GVW = min_gvw;  // tonne*10
}


void CConfigDataCore::OutputGeneral(bool write_time_history, bool write_each_event, bool write_fatigue_event, int write_buffer_size) 
{
    Output.WRITE_TIME_HISTORY = write_time_history;
    Output.WRITE_EACH_EVENT = write_each_event;
    Output.WRITE_EVENT_BUFFER_SIZE = write_fatigue_event;
    Output.WRITE_FATIGUE_EVENT = write_buffer_size;
}

void CConfigDataCore::OutputVehicleFile(bool write_vehicle_file, unsigned int vehicle_file_format, string vehicle_file_name, int buffer_size, bool write_flow_stats) 
{
    Output.VehicleFile.WRITE_VEHICLE_FILE = write_vehicle_file;
    Output.VehicleFile.FILE_FORMAT = vehicle_file_format;
    Output.VehicleFile.VEHICLE_FILENAME = vehicle_file_name;
    Output.VehicleFile.WRITE_VEHICLE_BUFFER_SIZE = buffer_size;
    Output.VehicleFile.WRITE_FLOW_STATS = write_flow_stats;
}

void CConfigDataCore::OutputBlockMax(bool write_blockmax, bool write_vehicle, bool write_summary, bool write_mixed, int block_size_days, int block_size_secs, int buffer_size) 
{
    Output.BlockMax.WRITE_BM = write_blockmax;
    Output.BlockMax.WRITE_BM_VEHICLES = write_vehicle;
    Output.BlockMax.WRITE_BM_SUMMARY = write_summary;
    Output.BlockMax.WRITE_BM_MIXED = write_mixed;
    Output.BlockMax.BLOCK_SIZE_DAYS = block_size_days;
    Output.BlockMax.BLOCK_SIZE_SECS = block_size_secs;
    Output.BlockMax.WRITE_BM_BUFFER_SIZE = buffer_size;
}

void CConfigDataCore::OutputPOT(bool write_pot, bool write_vehicle, bool write_summary, bool write_counter, int pot_size_days, int pot_size_secs, int buffer_size) 
{
    Output.POT.WRITE_POT = write_pot;
    Output.POT.WRITE_POT_VEHICLES = write_vehicle;
    Output.POT.WRITE_POT_SUMMARY = write_summary;
    Output.POT.WRITE_POT_COUNTER = write_counter;
    Output.POT.POT_COUNT_SIZE_DAYS = pot_size_days;
    Output.POT.POT_COUNT_SIZE_SECS = pot_size_secs;
    Output.POT.WRITE_POT_BUFFER_SIZE = buffer_size;
}

void CConfigDataCore::OutputStats(bool write_stats, bool write_cumulative, bool write_intervals, int interval_size, int buffer_size) 
{
    Output.Stats.WRITE_STATS = write_stats;
    Output.Stats.WRITE_SS_CUMULATIVE = write_cumulative;
    Output.Stats.WRITE_SS_INTERVALS = write_intervals;
    Output.Stats.WRITE_SS_INTERVAL_SIZE = interval_size;
    Output.Stats.WRITE_SS_BUFFER_SIZE = buffer_size;
}

void CConfigDataCore::OutputFatigue(bool do_rainflow, int decimal, double cut_off_value, int buffer_size) 
{
    Output.Fatigue.DO_FATIGUE_RAINFLOW = do_rainflow;
    Output.Fatigue.RAINFLOW_DECIMAL = decimal;
    Output.Fatigue.RAINFLOW_CUTOFF = cut_off_value;
    Output.Fatigue.WRITE_RAINFLOW_BUFFER_SIZE = buffer_size;
}



