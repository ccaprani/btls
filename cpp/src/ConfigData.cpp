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
	std::string str;
	str = GetNextDataLine();	Mode.PROGRAM_MODE				= m_CSV.stringToInt(	str);

	str = GetNextDataLine();	Gen.NO_DAYS						= m_CSV.stringToInt(	str);
	str = GetNextDataLine();	Gen.TRAFFIC_FOLDER				= str;
	str = GetNextDataLine();	Gen.TRUCK_TRACK_WIDTH			= m_CSV.stringToDouble( str);
	str = GetNextDataLine();	Gen.LANE_ECCENTRICITY_STD		= m_CSV.stringToDouble( str);
	str = GetNextDataLine();	Gen.KERNEL_TYPE					= m_CSV.stringToInt(str);
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
	str = GetNextDataLine();	Read.NOMINAL_FILE				= str;
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

std::string CConfigDataCore::GetNextDataLine()
{
	std::string line;
	m_CSV.getline(line);
	while(line.substr(0,2) == m_CommentString)
		m_CSV.getline(line);

	return line;
}
/*
vector<double> CConfigDataCore::GetVectorFromNextLine()
{
	vector<double> vData;
	std::string line;
	m_CSV.getline(line);
	while(line.substr(0,2) == m_CommentString)
		m_CSV.getline(line);

	size_t nData = m_CSV.getnfield();	// minus 1 if comma to be at the end is counted
	for (size_t j = 0; j < nData; j++)
	{
		std::string str = m_CSV.getfield(j);
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
std::string CConfigDataCore::returnInt(int i) 
{
	std::string a;
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

void CConfigDataCore::VehGenGrave(std::string lanesFile, std::string trafficFolder, size_t noDays, double truckTrackWidth, double laneEccentricityStd) 
{
    Gen.GEN_TRAFFIC = true;
    Read.READ_FILE = false;
    Road.LANES_FILE = lanesFile;
    Gen.TRAFFIC_FOLDER = trafficFolder;
    Gen.NO_DAYS = noDays;
    Gen.TRUCK_TRACK_WIDTH = truckTrackWidth;
    Gen.LANE_ECCENTRICITY_STD = laneEccentricityStd;
    Traffic.VEHICLE_MODEL = 0;
}

void CConfigDataCore::VehGenGarage(std::string lanesFile, std::string garageFile, size_t fileFormat, std::string kernelFile, size_t noDays, double laneEccentricityStd, int kernelType) 
{
    Gen.GEN_TRAFFIC = true;
    Read.READ_FILE = false;
    Road.LANES_FILE = lanesFile;
    Gen.NO_DAYS = noDays;
    Gen.LANE_ECCENTRICITY_STD = laneEccentricityStd;
	Gen.KERNEL_TYPE = kernelType;
    Read.GARAGE_FILE = garageFile;
    Read.KERNEL_FILE = kernelFile;
    Read.FILE_FORMAT = fileFormat;
    Traffic.VEHICLE_MODEL = 2;
}

void CConfigDataCore::VehGenNominal(std::string lanesFile, std::string constantFile, size_t noDays, double laneEccentricityStd, int kernelType) 
{
    Gen.GEN_TRAFFIC = true;
    Read.READ_FILE = false;
    Road.LANES_FILE = lanesFile;
    Gen.NO_DAYS = noDays;
    Gen.LANE_ECCENTRICITY_STD = laneEccentricityStd;
	Gen.KERNEL_TYPE = kernelType;
    Read.NOMINAL_FILE = constantFile;
    Traffic.VEHICLE_MODEL = 1;
}

void CConfigDataCore::FlowGenNHM(int classification, std::string trafficFolder) 
{
    Gen.GEN_TRAFFIC = true;
    Read.READ_FILE = false;
    Traffic.HEADWAY_MODEL = 0;
    Traffic.CLASSIFICATION = classification;  // 0 no_ of axle, 1 axle pattern
    Gen.TRAFFIC_FOLDER = trafficFolder;
}

void CConfigDataCore::FlowGenConstant(int classification, double constantSpeed, double constantGap) 
{
    Gen.GEN_TRAFFIC = true;
    Read.READ_FILE = false;
    Traffic.HEADWAY_MODEL = 1;
    Traffic.CLASSIFICATION = classification;  // 0 no_ of axle, 1 axle pattern
    Traffic.CONSTANT_SPEED = constantSpeed/3.6;  // km/h to m/s
    Traffic.CONSTANT_GAP = constantGap;
}

void CConfigDataCore::FlowGenCongestion(int classification, double congestedSpacing, double congestedSpeed, double congestedGapCOV) 
{
    Gen.GEN_TRAFFIC = true;
    Read.READ_FILE = false;
    Traffic.HEADWAY_MODEL = 5;
    Traffic.CLASSIFICATION = classification;  // 0 no. of axle, 1 axle pattern
    Traffic.CONGESTED_SPACING = congestedSpacing;
    Traffic.CONGESTED_SPEED = congestedSpeed/3.6;  // km/h to m/s
	Traffic.CONGESTED_GAP = Traffic.CONGESTED_SPACING/Traffic.CONGESTED_SPEED;
    Traffic.CONGESTED_GAP_COEF_VAR = congestedGapCOV;
}

void CConfigDataCore::FlowGenFreeFlow(int classification) 
{
    Gen.GEN_TRAFFIC = true;
    Read.READ_FILE = false;
    Traffic.HEADWAY_MODEL = 6;
    Traffic.CLASSIFICATION = classification;  // 0 no. of axle, 1 axle pattern
}


void CConfigDataCore::TrafficRead(std::string trafficFile, size_t fileFormat, bool useConstantSpeed, bool useAveSpeed, double constSpeed) 
{
    Gen.GEN_TRAFFIC = false;
    Read.READ_FILE = true;
    Read.TRAFFIC_FILE = trafficFile;
    Read.FILE_FORMAT = fileFormat;
    Read.USE_CONSTANT_SPEED = useConstantSpeed;
    Read.USE_AVE_SPEED = useAveSpeed;
    Read.CONST_SPEED = constSpeed;
}


void CConfigDataCore::Simulation(std::string bridgeFile, std::string infLineFile, std::string infSurfFile, double calcTimeStep, size_t minGVW) 
{
    Sim.CALC_LOAD_EFFECTS = true;
    Sim.BRIDGE_FILE = bridgeFile;
    Sim.INFLINE_FILE = infLineFile;
    Sim.INFSURF_FILE = infSurfFile;
    Sim.CALC_TIME_STEP = calcTimeStep;
    Sim.MIN_GVW = minGVW;  // tonne*10
}


void CConfigDataCore::OutputGeneral(bool writeTimeHistory, bool writeEachEvent, bool writeFatigueEvent, size_t writeBufferSize) 
{
    Output.WRITE_TIME_HISTORY = writeTimeHistory;
    Output.WRITE_EACH_EVENT = writeEachEvent;
    Output.WRITE_EVENT_BUFFER_SIZE = writeFatigueEvent;
    Output.WRITE_FATIGUE_EVENT = writeBufferSize;
}

void CConfigDataCore::OutputVehicleFile(bool writeVehicleFile, size_t vehicleFileFormat, std::string vehicleFileName, size_t bufferSize, bool writeFlowStats) 
{
    Output.VehicleFile.WRITE_VEHICLE_FILE = writeVehicleFile;
    Output.VehicleFile.FILE_FORMAT = vehicleFileFormat;
    Output.VehicleFile.VEHICLE_FILENAME = vehicleFileName;
    Output.VehicleFile.WRITE_VEHICLE_BUFFER_SIZE = bufferSize;
    Output.VehicleFile.WRITE_FLOW_STATS = writeFlowStats;
}

void CConfigDataCore::OutputBlockMax(bool writeBM, bool writeVehicle, bool writeSummary, bool write_mixed, size_t blockSizeDays, size_t blockSizeSecs, size_t bufferSize) 
{
    Output.BlockMax.WRITE_BM = writeBM;
    Output.BlockMax.WRITE_BM_VEHICLES = writeVehicle;
    Output.BlockMax.WRITE_BM_SUMMARY = writeSummary;
    Output.BlockMax.WRITE_BM_MIXED = write_mixed;
    Output.BlockMax.BLOCK_SIZE_DAYS = blockSizeDays;
    Output.BlockMax.BLOCK_SIZE_SECS = blockSizeSecs;
    Output.BlockMax.WRITE_BM_BUFFER_SIZE = bufferSize;
}

void CConfigDataCore::OutputPOT(bool writePOT, bool writeVehicle, bool writeSummary, bool writeCounter, size_t POTSizeDays, size_t POTSizeSecs, size_t bufferSize) 
{
    Output.POT.WRITE_POT = writePOT;
    Output.POT.WRITE_POT_VEHICLES = writeVehicle;
    Output.POT.WRITE_POT_SUMMARY = writeSummary;
    Output.POT.WRITE_POT_COUNTER = writeCounter;
    Output.POT.POT_COUNT_SIZE_DAYS = POTSizeDays;
    Output.POT.POT_COUNT_SIZE_SECS = POTSizeSecs;
    Output.POT.WRITE_POT_BUFFER_SIZE = bufferSize;
}

void CConfigDataCore::OutputStats(bool writeStats, bool writeCumulative, bool writeIntervals, size_t intervalSize, size_t bufferSize) 
{
    Output.Stats.WRITE_STATS = writeStats;
    Output.Stats.WRITE_SS_CUMULATIVE = writeCumulative;
    Output.Stats.WRITE_SS_INTERVALS = writeIntervals;
    Output.Stats.WRITE_SS_INTERVAL_SIZE = intervalSize;
    Output.Stats.WRITE_SS_BUFFER_SIZE = bufferSize;
}

void CConfigDataCore::OutputFatigue(bool doRainflow, int decimal, double cutOffValue, size_t bufferSize) 
{
    Output.Fatigue.DO_FATIGUE_RAINFLOW = doRainflow;
    Output.Fatigue.RAINFLOW_DECIMAL = decimal;
    Output.Fatigue.RAINFLOW_CUTOFF = cutOffValue;
    Output.Fatigue.WRITE_RAINFLOW_BUFFER_SIZE = bufferSize;
}



