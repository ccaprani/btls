// Effect.cpp: implementation of the CEffect class.
//
//////////////////////////////////////////////////////////////////////

#include "Effect.h"

// extern CGlobals myGlobals;

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEffect::CEffect(double value, double time, double distance)
{
	m_Val			= value;		
	m_Time			= time;	
	m_Dist			= distance;
	m_NoVehicles	= 0;
}

CEffect::CEffect()
{
	m_Val			= 0;		
	m_Time			= 0;	
	m_Dist			= 0;
	m_NoVehicles	= 0;
}

CEffect::~CEffect()
{

}

bool CEffect::operator<(const CEffect& x)
{
	return m_Val < x.m_Val;
}

void CEffect::setTime(double time)
{
	m_Time = time;
}

void CEffect::setValue(double val)
{
	m_Val = val;
}

void CEffect::setPosition(double pos)
{
	m_Dist = pos;
}

double CEffect::getPosition()
{
	return m_Dist;
}

double CEffect::getTime()
{
	return m_Time;
}

double CEffect::getValue()
const
{
	return m_Val;
}

void CEffect::AddVehicles(std::vector<CVehicle> vVeh)
{
	m_vVehicles = vVeh;
	m_NoVehicles = vVeh.size();
}

void CEffect::AddVehicle(CVehicle& Vehicle)
{
	m_NoVehicles++;
	m_vVehicles.push_back(Vehicle);
}

CVehicle CEffect::giveVehicle(size_t i) const
{
	//i = i - 1;
	if(i <= m_vVehicles.size())
		return m_vVehicles[i];
	else
	{
		std::cerr << "Vehicle not available: " << i << endl;
		return m_vVehicles[0];
	}
}

void CEffect::sortVehicles()
{
	std::sort(m_vVehicles.begin(), m_vVehicles.end());
}
