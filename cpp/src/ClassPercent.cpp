// ClassPercent.cpp: implementation of the CClassPercent class.
//
//////////////////////////////////////////////////////////////////////

#include "ClassPercent.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CClassPercent::CClassPercent()
{
	m_NoLanes = 0;
}

CClassPercent::~CClassPercent()
{

}

void CClassPercent::AddClassPercent(int iLane, int iClass, double val)
{
	if( iLane >= m_NoLanes )
	{
		CP tempCP;
		m_NoLanes++;
		m_vCP.push_back(tempCP);
	}
	
	switch(iClass)
	{
		case 2:	m_vCP[iLane].CP_2Axle = val; break;
		case 3:	m_vCP[iLane].CP_3Axle = val; break;
		case 4:	m_vCP[iLane].CP_4Axle = val; break;
		case 5:	m_vCP[iLane].CP_5Axle = val; break;
	}

}

double CClassPercent::GetClassPercent(int iLane, int iClass)
{
	double val = 0.0;

	if(iLane < m_NoLanes)
	{
		switch(iClass)
		{
			case 2:	val = m_vCP[iLane].CP_2Axle; break;
			case 3:	val = m_vCP[iLane].CP_3Axle; break;
			case 4:	val = m_vCP[iLane].CP_4Axle; break;
			case 5:	val = m_vCP[iLane].CP_5Axle; break;
		}
	}
	return val;
}

int CClassPercent::GetNoLanes()
{
	return m_NoLanes;
}
