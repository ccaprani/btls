// ConfigData.cpp: implementation of the CConfigData class.
//
//////////////////////////////////////////////////////////////////////

#include "ConfigData.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConfigData::CConfigData()
{
	m_CommentString = "//";
	SetDefaults();
}

CConfigData::~CConfigData()
{

}

// the global config data object

CConfigData g_ConfigData;

bool CConfigData::ReadData(std::string inFile)
{
	if( m_CSV.OpenFile(inFile, ",") )
	{
		ExtractData();
		return true;
	}
	return false;
}

void CConfigData::ExtractData()
{
	string str;
	str = GetNextDataLine();	Mode.PROGRAM_MODE				= m_CSV.stringToInt(	str);

	str = GetNextDataLine();	Gen.NO_DAYS						= m_CSV.stringToInt(	str);
	str = GetNextDataLine();	Gen.TRAFFIC_FOLDER				= str;
	//str = GetNextDataLine();	Gen.SITE_WEIGHT					= m_CSV.stringToInt(	str);
	str = GetNextDataLine();	Gen.TRUCK_TRACK_WIDTH			= m_CSV.stringToDouble( str);
	str = GetNextDataLine();	Gen.LANE_ECCENTRICITY_STD		= m_CSV.stringToDouble( str);
	str = GetNextDataLine();	Traffic.HEADWAY_MODEL			= m_CSV.stringToInt(	str);
	str = GetNextDataLine();	Road.LANES_FILE					= str;
	str = GetNextDataLine();	Traffic.CONGESTED_SPACING		= m_CSV.stringToDouble( str);
	str = GetNextDataLine();	Traffic.CONGESTED_SPEED			= m_CSV.stringToDouble( str);
	str = GetNextDataLine();	Traffic.CONGESTED_GAP_COEF_VAR	= m_CSV.stringToDouble( str);

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

string CConfigData::GetNextDataLine()
{
	string line;
	m_CSV.getline(line);
	while(line.substr(0,2) == m_CommentString)
		m_CSV.getline(line);

	return line;
}

vector<double> CConfigData::GetVectorFromNextLine()
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

void CConfigData::SetDefaults()
{
	Mode.PROGRAM_MODE = 1;

	Gen.NO_DAYS = 2;
	Gen.TRAFFIC_FOLDER = "C:\\Traffic\\Auxerre\\";
	//Gen.SITE_WEIGHT = 2;
	Traffic.HEADWAY_MODEL = 6;
	Road.LANES_FILE = "laneflow.csv";
	Traffic.CONGESTED_SPACING = 5;
	Traffic.CONGESTED_SPEED = 30;	// km/h
	Traffic.CONGESTED_GAP_COEF_VAR = 0.05;
	Gen.NO_OVERLAP_LENGTH = 100.0;
	Gen.TRUCK_TRACK_WIDTH = 190.0; // cm

	Read.TRAFFIC_FILE = "input.txt";
	Read.FILE_FORMAT = 1;
	Read.USE_CONSTANT_SPEED = false;
	Read.USE_AVE_SPEED = true;
	Read.CONST_SPEED = 20;	// km/h

	Sim.BRIDGE_FILE = "bridge.txt";
	Sim.INFLINE_FILE = "";
	Sim.INFSURF_FILE = "";
	Sim.CALC_TIME_STEP = 0.1;
	Sim.MIN_GVW = 35;
	
	Output.WRITE_TIME_HISTORY = 0;
	Output.WRITE_EACH_EVENT = 0;
	Output.WRITE_EVENT_BUFFER_SIZE = 10000;
	Output.WRITE_FATIGUE_EVENT = 0;

	Output.VehicleFile.WRITE_VEHICLE_FILE = 0;
	Output.VehicleFile.FILE_FORMAT = 1;
	Output.VehicleFile.VEHICLE_FILENAME = "BTLSvehicles.txt";
	Output.VehicleFile.WRITE_VEHICLE_BUFFER_SIZE = 10000;
	
	Output.BlockMax.WRITE_BM = 1;
	Output.BlockMax.WRITE_BM_VEHICLES = 0;
	Output.BlockMax.WRITE_BM_SUMMARY = 1;
	Output.BlockMax.WRITE_BM_MIXED = 1;
	Output.BlockMax.BLOCK_SIZE_DAYS = 1;
	Output.BlockMax.BLOCK_SIZE_SECS = 0;
	Output.BlockMax.WRITE_BM_BUFFER_SIZE = 10000;

	Output.POT.WRITE_POT = 1;
	Output.POT.WRITE_POT_VEHICLES = 0;
	Output.POT.WRITE_POT_SUMMARY = 1;
	Output.POT.WRITE_POT_COUNTER = 1;
	Output.POT.POT_COUNT_SIZE_DAYS = 1;
	Output.POT.POT_COUNT_SIZE_SECS = 0;
	Output.POT.WRITE_POT_BUFFER_SIZE = 10000;

	Output.Stats.WRITE_STATS = 1;
	Output.Stats.WRITE_SS_CUMULATIVE = 1;
	Output.Stats.WRITE_SS_INTERVALS = 0;
	Output.Stats.WRITE_SS_INTERVAL_SIZE = 3600;
	Output.Stats.WRITE_SS_BUFFER_SIZE = 10000;

	Time.DAYS_PER_MT	= 25;
	Time.MTS_PER_YR		= 10;
	Time.HOURS_PER_DAY	= 24;
	Time.SECS_PER_HOUR	= 3600;
	Time.MINS_PER_HOUR	= 60;
	Time.SECS_PER_MIN	= 60;

	doDerivedConstants();
}

void CConfigData::doDerivedConstants()
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

string CConfigData::returnInt(int i) 
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


