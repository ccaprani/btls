#include "LaneFileTraffic.h"


CLaneFileTraffic::CLaneFileTraffic(void)
{
	m_iCurVehicle = 0;
}


CLaneFileTraffic::~CLaneFileTraffic(void)
{
}


CVehicle_sp CLaneFileTraffic::GetNextVehicle()
{
	if(m_iCurVehicle >= m_vVehicles.size())
		return nullptr;

	// store pointer to vehicle, moving it out of the vector so the consumed
	// vehicle is released as the replay progresses
	CVehicle_sp pVeh = std::move(m_vVehicles.at(m_iCurVehicle));

	// advance the cursor: erasing the front entry shifted every remaining
	// element, making a recorded replay quadratic in the number of vehicles
	m_iCurVehicle++;
	// check now if no vehicles left
	if(m_iCurVehicle >= m_vVehicles.size())
		m_NextArrivalTime = 1e300;	// MAGIC NUMBER a really big number
	else
	{
		// set next arrival time and vehicle pointer
		m_pNextVeh = m_vVehicles.at(m_iCurVehicle);
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
	m_NextArrivalTime = m_vVehicles.at(m_iCurVehicle)->getTime();
}