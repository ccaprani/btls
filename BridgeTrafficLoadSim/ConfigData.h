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
public:
	CConfigData();
	virtual ~CConfigData();
	
	void SetDefaults();	
	bool ReadData(std::string inFile);

	struct Mode_Config
	{
		size_t PROGRAM_MODE;
	} Mode;

	struct Road_Config
	{
		string LANES_FILE;
		size_t NO_LANES_DIR1;
		size_t NO_LANES_DIR2;
		size_t NO_LANES;
		size_t NO_DIRS;
	} Road;

	struct Gen_Config
	{
		string TRAFFIC_FOLDER;
		bool GEN_TRAFFIC;
		int NO_DAYS;
		//int SITE_WEIGHT;
		//int SITE_FLOW;
		double TRUCK_TRACK_WIDTH;
		double LANE_ECCENTRICITY_STD;
		double NO_OVERLAP_LENGTH;
	} Gen;

	struct Read_Config
	{
		bool READ_FILE;
		string TRAFFIC_FILE;
		unsigned int FILE_FORMAT;
		bool USE_CONSTANT_SPEED;
		bool USE_AVE_SPEED;
		double CONST_SPEED;
	} Read;

	struct Traffic_Config
	{
		int HEADWAY_MODEL;
		//double PROPORTION_OF_CARS;
		double CONGESTED_SPACING;
		double CONGESTED_SPEED;
		double CONGESTED_GAP;
		double CONGESTED_GAP_COEF_VAR;
	} Traffic;

	struct Sim_Config
	{
		bool CALC_LOAD_EFFECTS;
		string BRIDGE_FILE;
		string INFLINE_FILE;
		string INFSURF_FILE;
		double CALC_TIME_STEP;
		int MIN_GVW;
	} Sim;

	struct Output_Config
	{
		bool WRITE_TIME_HISTORY;
		bool WRITE_EACH_EVENT;
		int  WRITE_EVENT_BUFFER_SIZE;
		bool WRITE_FATIGUE_EVENT;		
		
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

	} Output;

	struct Time_Config
	{
		int		DAYS_PER_MT;
		int		MTS_PER_YR;
		int		HOURS_PER_DAY;
		int		SECS_PER_HOUR;
		int		MINS_PER_HOUR;
		int		SECS_PER_MIN;
	} Time;

private:
	
	CCSVParse m_CSV;
	string m_CommentString;
	
	void doDerivedConstants();
	void ExtractData();
	string GetNextDataLine();
	string returnInt(int i);
	vector<double> GetVectorFromNextLine();

};

#endif // !defined(AFX_CONFIGDATA_H__066A62BB_724D_469B_B540_6710541CE832__INCLUDED_)
