// Lane.h: interface for the CLane class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LANE_H__7ED6AACE_9B6B_4619_98A7_E7EA4F38FFD1__INCLUDED_)
#define AFX_LANE_H__7ED6AACE_9B6B_4619_98A7_E7EA4F38FFD1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <vector>
#include "Vehicle.h"

class CLane  
{
public:
	CLane();
	virtual ~CLane();
	
	int GetLaneID();
	int GetDirection();

	virtual CVehicle_ptr GetNextVehicle() = 0;
	double GetNextArrivalTime() const;
	void WriteVehicle();

protected:
	int		m_LaneIndex;
	int		m_Direction;
	double	m_NextArrivalTime;
	
	CVehicle_ptr m_pNextVeh;
};
typedef std::shared_ptr<CLane> CLane_ptr;

#endif // !defined(AFX_LANE_H__7ED6AACE_9B6B_4619_98A7_E7EA4F38FFD1__INCLUDED_)
