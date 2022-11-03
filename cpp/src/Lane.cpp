// Lane.cpp: implementation of the CLane class.
//
//////////////////////////////////////////////////////////////////////

#include "Lane.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLane::CLane()
{
	m_NextArrivalTime = 0.0;
	m_pNextVeh = nullptr;
}

CLane::~CLane()
{

}

double CLane::GetNextArrivalTime() const
{
	return m_NextArrivalTime;
}

size_t CLane::GetLaneID()
{
	return m_LaneIndex;
}

size_t CLane::GetDirection()
{
	return m_Direction;
}