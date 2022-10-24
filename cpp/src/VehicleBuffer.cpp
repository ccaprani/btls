// VehicleBuffer.cpp: implementation of the CVehicleBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include <iostream>
#include "VehicleBuffer.h"
#include "ConfigData.h"

//extern CConfigData g_ConfigData;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVehicleBuffer::CVehicleBuffer(CVehicleClassification_sp pVC, double starttime)
{
	//init(false, "", 0);
	WRITE_VEHICLE_FILE			= CConfigData::get().Output.VehicleFile.WRITE_VEHICLE_FILE;
	FILE_FORMAT					= CConfigData::get().Output.VehicleFile.FILE_FORMAT;	
	VEHICLE_FILENAME			= CConfigData::get().Output.VehicleFile.VEHICLE_FILENAME;
	WRITE_VEHICLE_BUFFER_SIZE	= CConfigData::get().Output.VehicleFile.WRITE_VEHICLE_BUFFER_SIZE;
	WRITE_FLOW_STATS			= CConfigData::get().Output.VehicleFile.WRITE_FLOW_STATS;

	NO_LANES_DIR1	= CConfigData::get().Road.NO_LANES_DIR1;
	NO_LANES_DIR2	= CConfigData::get().Road.NO_LANES_DIR2;
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

void CVehicleBuffer::AddVehicle(const CVehicle_sp& pVeh)
{ 
	if(pVeh == nullptr)
		return;
	if( !(WRITE_VEHICLE_FILE || WRITE_FLOW_STATS) )
		return;

	if (WRITE_FLOW_STATS)
	{
		updateFlowData(pVeh);
	}

	if (WRITE_VEHICLE_FILE)
	{
		// copy it & store locally using move semantics
		CVehicle_up pV = std::make_unique<CVehicle>(*pVeh);
		m_vVehicles.push_back(std::move(pV));
		m_NoVehicles++;
	}

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
		CVehicle_up& pVeh = m_vVehicles.at(nVehs-1);
		std::cout << std::endl  << "Flushing buffer of " 
			<< nVehs << " vehicles at " << pVeh->getTimeStr() <<  std::endl;
		
		for (size_t i = 0; i < nVehs; i++)
			m_OutFile << m_vVehicles.at(i)->Write(FILE_FORMAT) << '\n';
		
		// clear the memory
		for (unsigned int i = 0; i < nVehs; i++)
			m_vVehicles.at(i) = nullptr;
	}

	m_NoVehicles = 0;
	m_vVehicles.clear();
	m_vVehicles.reserve(WRITE_VEHICLE_BUFFER_SIZE);
}

void CVehicleBuffer::updateFlowData(const CVehicle_sp& pV)
{
	if(!WRITE_FLOW_STATS)
		return;

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
		// oStr << std::ends;
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
			// oStr << std::ends;
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