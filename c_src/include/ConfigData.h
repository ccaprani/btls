// ConfigData.h: interface for the CConfigData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONFIGDATA_H__066A62BB_724D_469B_B540_6710541CE832__INCLUDED_)
#define AFX_CONFIGDATA_H__066A62BB_724D_469B_B540_6710541CE832__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CSVParse.h"
#include <vector>
#include <algorithm>

class CConfigData  
{
	// Using Singleton pattern here:
	// https://stackoverflow.com/questions/1008019/c-singleton-design-pattern/1008289#1008289
public:
	static CConfigData& get()
	{
		static CConfigData instance;	// Guaranteed to be destroyed.
		return instance;				// Instantiated on first use.
	}

private:
	CConfigData();						// Constructor? (the {} brackets) are needed here.

	// C++ 11
	// =======
	// We can use the better technique of deleting the methods
	// we don't want.
public:
	CConfigData(CConfigData const&) = delete;
	void operator=(CConfigData const&) = delete;
	// Note: Scott Meyers mentions in his Effective Modern
	//       C++ book, that deleted functions should generally
	//       be public as it results in better error messages
	//       due to the compilers behavior to check accessibility
	//       before deleted status

public:	
	bool ReadData(std::string inFile);

	struct Mode_Config
	{
		size_t PROGRAM_MODE;
	} Mode = { 1 };

	struct Road_Config
	{
		string LANES_FILE;
		size_t NO_LANES_DIR1;
		size_t NO_LANES_DIR2;
		size_t NO_LANES;
		size_t NO_DIRS;
	} Road = {"laneflow.csv",1,1,2,2};

	struct Gen_Config
	{
		string TRAFFIC_FOLDER;
		bool GEN_TRAFFIC;
		size_t NO_DAYS;
		double TRUCK_TRACK_WIDTH;
		double LANE_ECCENTRICITY_STD;
		double NO_OVERLAP_LENGTH;
	} Gen = { "C:\\Traffic\\Auxerre\\",true,1,190.0,0.0,100.0};

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
	} Read = {false,"traffic.txt","garage.txt","kernel.csv","constant_vehicle.csv",4,true,false,24.0};

	struct Traffic_Config
	{
		int VEHICLE_MODEL;
		int HEADWAY_MODEL;
		int CLASSIFICATION;
		double CONGESTED_SPACING;
		double CONGESTED_SPEED;
		double CONGESTED_GAP;
		double CONGESTED_GAP_COEF_VAR;
	} Traffic = {1,1,1,5.0,30.0,5.0,0.05};

	struct Sim_Config
	{
		bool CALC_LOAD_EFFECTS;
		string BRIDGE_FILE;
		string INFLINE_FILE;
		string INFSURF_FILE;
		double CALC_TIME_STEP;
		int MIN_GVW;
	} Sim = {true,"bridges.txt","IL.txt","IS.csv",0.1,35};

	struct Output_Config
	{
		bool WRITE_TIME_HISTORY;
		bool WRITE_EACH_EVENT;
		int  WRITE_EVENT_BUFFER_SIZE;
		bool WRITE_FATIGUE_EVENT;	
		bool DO_FATIGUE_RAINFLOW;
		
		struct VehicleFile_Config
		{
			bool WRITE_VEHICLE_FILE;
			unsigned int FILE_FORMAT;
			string VEHICLE_FILENAME;
			int	 WRITE_VEHICLE_BUFFER_SIZE;
			bool WRITE_FLOW_STATS;
		} VehicleFile;

		struct BlockMax_Config
		{
			bool WRITE_BM;
			bool WRITE_BM_VEHICLES;
			bool WRITE_BM_SUMMARY;
			bool WRITE_BM_MIXED;
			int  BLOCK_SIZE_DAYS;
			int  BLOCK_SIZE_SECS;
			int  WRITE_BM_BUFFER_SIZE;
		} BlockMax;

		struct POT_Config
		{
			bool WRITE_POT;
			bool WRITE_POT_VEHICLES;
			bool WRITE_POT_SUMMARY;
			bool WRITE_POT_COUNTER;
			int  POT_COUNT_SIZE_DAYS;
			int  POT_COUNT_SIZE_SECS;
			int  WRITE_POT_BUFFER_SIZE;
		} POT;

		struct Stats_Config
		{
			bool WRITE_STATS;
			bool WRITE_SS_CUMULATIVE;
			bool WRITE_SS_INTERVALS;
			int  WRITE_SS_INTERVAL_SIZE;
			int  WRITE_SS_BUFFER_SIZE;
		} Stats;

	} Output = {false, false, 1000, false, false,
				{false,4,"vehicles.txt",1000,true}, // VehicleFile_Config
				{true,false,false,false,1,0,1000},	// BlockMax_Config
				{false,false,false,false,1,0,1000},	// POT_Config
				{true,true,false,3600,1000} };		// Stats_Config

	struct Time_Config
	{
		int		DAYS_PER_MT;
		int		MTS_PER_YR;
		int		HOURS_PER_DAY;
		int		SECS_PER_HOUR;
		int		MINS_PER_HOUR;
		int		SECS_PER_MIN;
	} Time = {25,10,24,3600,60,60};

private:
	
	CCSVParse m_CSV;
	string m_CommentString;
	
	void doDerivedConstants();
	void ExtractData();
	string GetNextDataLine();
	string returnInt(int i);
	vector<double> GetVectorFromNextLine();

};

// the global config data object
//CConfigData g_ConfigData;

#endif // !defined(AFX_CONFIGDATA_H__066A62BB_724D_469B_B540_6710541CE832__INCLUDED_)
