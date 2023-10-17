#include "LaneFileTraffic.h"


CLaneFileTraffic::CLaneFileTraffic(void)
{
}


CLaneFileTraffic::~CLaneFileTraffic(void)
{
}


CVehicle_sp CLaneFileTraffic::GetNextVehicle()
{
	if(m_vVehicles.empty())	
		return nullptr;

	// store pointer to vehicle
	CVehicle_sp pVeh = std::make_shared<CVehicle>();
	*pVeh = *m_vVehicles.front();  // This has to be this, otherwise Python freezes when multiprocessing 

	// delete entry in vector
	m_vVehicles.erase(m_vVehicles.begin());
	// check now if no vehicles left
	if(m_vVehicles.empty())
		m_NextArrivalTime = 1e300;	// MAGIC NUMBER a really big number
	else
	{
		// set next arrival time and vehicle pointer
		m_pNextVeh = m_vVehicles.front();
		m_NextArrivalTime = m_pNextVeh->getTime();
	}
	
	// return pointer
	return pVeh;
}

void CLaneFileTraffic::setLaneData(int dirn, int laneNo)
{
	m_Direction = dirn;
	m_LaneIndex = laneNo;
}

void CLaneFileTraffic::addVehicle(CVehicle_sp pVeh)
{
	m_vVehicles.push_back(pVeh);
}

void CLaneFileTraffic::setFirstArrivalTime()
{
	m_NextArrivalTime = m_vVehicles.front()->getTime();
}