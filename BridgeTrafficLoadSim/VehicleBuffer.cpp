// VehicleBuffer.cpp: implementation of the CVehicleBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include <iostream>
#include "VehicleBuffer.h"
#include "ConfigData.h"

extern CConfigData g_ConfigData;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVehicleBuffer::CVehicleBuffer(CVehicleClassification_ptr pVC, double starttime)
{
	//init(false, "", 0);

	FILE_FORMAT					= g_ConfigData.Output.VehicleFile.FILE_FORMAT;
	WRITE_VEHICLE_FILE			= g_ConfigData.Output.VehicleFile.WRITE_VEHICLE_FILE;
	VEHICLE_FILENAME			= g_ConfigData.Output.VehicleFile.VEHICLE_FILENAME;
	WRITE_VEHICLE_BUFFER_SIZE	= g_ConfigData.Output.VehicleFile.WRITE_VEHICLE_BUFFER_SIZE;
	WRITE_FLOW_STATS			= g_ConfigData.Output.VehicleFile.WRITE_FLOW_STATS;

	NO_LANES_DIR1	= g_ConfigData.Road.NO_LANES_DIR1;
	NO_LANES_DIR2	= g_ConfigData.Road.NO_LANES_DIR2;
	NO_LANES = NO_LANES_DIR1 + NO_LANES_DIR2;

	m_pVehClassification = pVC;
	m_NoVehicles = 0;

	if(WRITE_FLOW_STATS)
	{
		m_FirstHour = (size_t)floor(starttime / 3600.0);
		m_CurHour = 0;
		flushFlowData(); // will set cur hour to 1
	}
	
	if(WRITE_VEHICLE_FILE)
	{
		m_OutFile.open(VEHICLE_FILENAME.c_str(), std::ios::out);
		m_vVehicles.reserve(WRITE_VEHICLE_BUFFER_SIZE);	
	}
}

CVehicleBuffer::~CVehicleBuffer()
{
	if(m_OutFile.is_open())
		m_OutFile.close();
	
	writeFlowData();
}

void CVehicleBuffer::AddVehicle(CVehicle_ptr pVeh)
{ 
	if(pVeh == nullptr)
		return;
	if( !(WRITE_VEHICLE_FILE || WRITE_FLOW_STATS) )
		return;

	// copy it locally
	CVehicle_ptr pV = std::make_shared<CVehicle>(*pVeh); //new CVehicle;
	//*pV = *pVeh;

	m_NoVehicles++;
	m_vVehicles.push_back(pV);
	
	updateFlowData();

	if(m_NoVehicles >= WRITE_VEHICLE_BUFFER_SIZE)
		FlushBuffer();
}

void CVehicleBuffer::FlushBuffer()
{
	if(!WRITE_VEHICLE_FILE)
		return;

	size_t nVehs = m_vVehicles.size();
	if(nVehs > 0)
	{
		CVehicle_ptr pVeh = m_vVehicles[nVehs-1];
		std::cout << std::endl  << "Flushing buffer of " << nVehs << " vehicles at " << pVeh->getTimeStr() <<  '\t';
		
		for (size_t i = 0; i < nVehs; i++)
			m_OutFile << m_vVehicles[i]->Write(FILE_FORMAT) << '\n';
		
		// clear the memory
		//for(unsigned int i = 0; i < nVehs; i++)
		//	delete m_vVehicles[i];
	}

	m_NoVehicles = 0;
	m_vVehicles.clear();
	m_vVehicles.reserve(WRITE_VEHICLE_BUFFER_SIZE);
}

void CVehicleBuffer::updateFlowData()
{
	if(!WRITE_FLOW_STATS)
		return;

	// get vehicle and see if in next hour
	CVehicle_ptr pV = m_vVehicles.back();
	double curRelTime = pV->getTime() - m_FirstHour*3600.0;
	if (curRelTime > m_CurHour*3600.0)
		flushFlowData();
	
	// get ref to data
	size_t lane = pV->getGlobalLane(NO_LANES);
	CFlowRateData& data = m_vFlowData.at(m_CurHour-1).at(lane-1);
	
	// assemble data
	data.m_NoVehicles++;
	if( pV->IsCar() ) 
		data.m_NoCars++;
	else
		data.m_NoTrucks++;
	data.addByClass( m_pVehClassification->getClassID(pV->getClass()) );
}

void CVehicleBuffer::flushFlowData()
{
	m_CurHour++;
	CFlowRateData data(m_pVehClassification->getNoClasses());
	data.m_ID = m_CurHour;
	
	std::vector<CFlowRateData> vFD(NO_LANES,data);
	m_vFlowData.push_back(vFD);
}

void CVehicleBuffer::writeFlowData()
{
	if(!WRITE_FLOW_STATS)
		return;

	for(unsigned int iLane = 0; iLane < NO_LANES; iLane++)
	{
		int dir = iLane < NO_LANES_DIR1 ? 1 : 2;
		std::string file = "FlowData_" + to_string(dir) + "_" + to_string(iLane+1) + ".txt";
		std::ofstream outFile(file.c_str(), std::ios::out ); 

		std::ostringstream oStr;
		oStr.width(12);		oStr << std::right << "Hour";
		oStr.width(12);		oStr << std::right << "#Vehicles";
		oStr.width(12);		oStr << std::right << "#Trucks";
		oStr.width(12);		oStr << std::right << "#Cars";
		for (size_t j = 0; j < m_pVehClassification->getNoClasses(); j++)
		{
			std::string str;
			str = to_string(j) + ": " + m_pVehClassification->getClass(j).m_Desc;
			oStr.width(20);
			oStr << std::right << str;
		}
		oStr << std::ends;
		outFile << oStr.str() << '\n';
		oStr.str(""); //clears the stringstream

		for(size_t i = 0; i < m_vFlowData.size(); i++)
		{
			CFlowRateData& data = m_vFlowData.at(i).at(iLane);
			
			oStr.width(12); oStr << std::right << data.m_ID;
			oStr.width(12); oStr << std::right << data.m_NoVehicles;
			oStr.width(12); oStr << std::right << data.m_NoTrucks;
			oStr.width(12); oStr << std::right << data.m_NoCars;
			for (size_t j = 0; j < data.m_vClassCount.size(); j++)
			{
				oStr.width(20); oStr << std::right << data.m_vClassCount.at(j);
			}
			oStr << std::ends;
			outFile << oStr.str() << '\n';
			oStr.str("");
		}
	
		outFile.close();
	}
}

template <typename T>
std::string CVehicleBuffer::to_string(T const& value)
{
    stringstream sstr;
    sstr << value;
    return sstr.str();
}