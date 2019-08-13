// VehicleBuffer.h: interface for the CVehicleBuffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VEHICLEBUFFER_H__B75906EE_9833_4A7C_A839_79BAB74EE23B__INCLUDED_)
#define AFX_VEHICLEBUFFER_H__B75906EE_9833_4A7C_A839_79BAB74EE23B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include <string>
#include <fstream>
#include "Vehicle.h"

struct CFlowRateData
{
	unsigned int m_ID;
	unsigned int m_NoVehicles;
	unsigned int m_NoTrucks;
	unsigned int m_NoCars;
	std::vector<unsigned int> m_vNoTruckAxles;

	CFlowRateData()
	{
		m_ID = 0;
		m_NoVehicles = 0;
		m_NoTrucks = 0;
		m_NoCars = 0;
		m_vNoTruckAxles.assign(20,0); // MAGIC NUMBER - max number of axles
	};
};

class CVehicleBuffer  
{
public:
	CVehicleBuffer();
	//CVehicleBuffer(bool WriteVehicleFile, std::string OutFile, int size);
	virtual ~CVehicleBuffer();
	
	//void init(bool WriteVehicleFile, std::string OutFile, int size);
	void AddVehicle(CVehicle* pVeh);
	void FlushBuffer();

private:
//	void SetBufferSize(int size);
	void writeFlowData();
	void updateFlowData();
	void flushFlowData();

	std::ofstream m_OutFile;
	std::vector<CVehicle*> m_vVehicles;
	int m_NoVehicles;
	//int m_BufferSize;

	unsigned int m_CurHour;
	std::vector< std::vector<CFlowRateData> > m_vFlowData;

	unsigned int FILE_FORMAT;
	bool WRITE_VEHICLE_FILE;
	std::string VEHICLE_FILENAME;
	int	 WRITE_VEHICLE_BUFFER_SIZE;
	bool WRITE_FLOW_STATS;

	unsigned int NO_LANES_DIR1;
	unsigned int NO_LANES_DIR2;
	unsigned int NO_LANES;

	template <typename T> std::string to_string(T const& value);
};

#endif // !defined(AFX_VEHICLEBUFFER_H__B75906EE_9833_4A7C_A839_79BAB74EE23B__INCLUDED_)
