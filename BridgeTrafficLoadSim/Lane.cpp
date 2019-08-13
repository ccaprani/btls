// Lane.cpp: implementation of the CLane class.
//
//////////////////////////////////////////////////////////////////////

#include "Lane.h"
#include "ConfigData.h"

extern CConfigData g_ConfigData;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLane::CLane()
{
	m_NextArrivalTime = 0.0;
	m_pNextVeh = NULL;
}

CLane::~CLane()
{

}

double CLane::GetNextArrivalTime() const
{
	return m_NextArrivalTime;
}

int CLane::GetLaneID()
{
	return m_LaneIndex;
}

int CLane::GetDirection()
{
	return m_Direction;
}