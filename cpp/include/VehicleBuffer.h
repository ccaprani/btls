// VehicleBuffer.h: interface for the CVehicleBuffer class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <math.h>
#include "Vehicle.h"
#include "ConfigData.h"

struct CFlowRateData
{
	size_t m_ID;
	size_t m_NoVehicles;
	size_t m_NoTrucks;
	size_t m_NoCars;
	std::vector<size_t> m_vClassCount;

	CFlowRateData(size_t n)
	{
		m_ID = 0;
		m_NoVehicles = 0;
		m_NoTrucks = 0;
		m_NoCars = 0;
		m_vClassCount.assign(n, 0);
	};

	void addByClass(size_t iClass)
	{
		if (iClass > m_vClassCount.size())
			m_vClassCount.at(0)++; // count as default
		else
			m_vClassCount.at(iClass)++;
	}
};

class CVehicleBuffer  
{
public:
	CVehicleBuffer(CConfigDataCore& config, CVehicleClassification_sp pVC, double starttime);
	virtual ~CVehicleBuffer();
	
	void AddVehicle(const CVehicle_sp& pVeh);
	void FlushBuffer();

private:

//	void SetBufferSize(int size);
	void writeFlowData();
	void updateFlowData(const CVehicle_sp& pVeh);
	void flushFlowData();

	std::ofstream m_OutFile;
	std::vector<CVehicle_up> m_vVehicles;
	size_t m_NoVehicles;
	//int m_BufferSize;

	size_t m_FirstHour;
	size_t m_CurHour;
	std::vector< std::vector<CFlowRateData> > m_vFlowData;

	size_t FILE_FORMAT;
	bool WRITE_VEHICLE_FILE;
	std::string VEHICLE_FILENAME;
	size_t	 WRITE_VEHICLE_BUFFER_SIZE;
	bool WRITE_FLOW_STATS;

	size_t NO_LANES_DIR1;
	size_t NO_LANES_DIR2;
	size_t NO_LANES;

	CVehicleClassification_sp m_pVehClassification;

	template <typename T> std::string to_string(T const& value);
};
