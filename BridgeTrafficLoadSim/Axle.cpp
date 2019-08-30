// Axle.cpp: implementation of the CAxle class.
//
//////////////////////////////////////////////////////////////////////

#include "Axle.h"

#include "Vehicle.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAxle::CAxle(size_t i, double t, double v, double x, double w, double tw, int dirn)
{
	m_Index = i;
	m_TimeAtDatum = t;
	m_Speed = v;
	m_Position = x;
	m_AxleWeight = w;
	m_TrackWidth = tw;
	m_Dirn = dirn;

	m_Sign = m_Dirn == 1 ? 1 : -1;
}

CAxle::CAxle(size_t i, size_t iAxle, double t, double x, CVehicle_ptr pVeh)
{
	m_Index = i;
	m_TimeAtDatum = t;
	m_Position = x;
	
	m_AxleWeight = pVeh->getAW(iAxle);
	m_TrackWidth = pVeh->getAT(iAxle);	
	
	//m_pVeh = pVeh;
	m_Speed = pVeh->getVelocity();
	m_Dirn = pVeh->getDirection();
	m_TransPos = pVeh->getTrans();
	m_Eccentricity = pVeh->getLaneEccentricity();
	m_Lane = pVeh->getBridgeLaneNo();
	
	m_Sign = m_Dirn == 1 ? 1 : -1;
}

CAxle::CAxle()
{

}

CAxle::~CAxle()
{

}

void CAxle::UpdatePosition(double time)
{
	double timeFromDatum = time - m_TimeAtDatum;
	m_Position = m_Sign*m_Speed*timeFromDatum;
}
