// AxleWeight45.cpp: implementation of the CAxleWeight45 class.
//
//////////////////////////////////////////////////////////////////////

#include "AxleWeight45.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAxleWeight45::CAxleWeight45()
{
	GVWRange range;
	m_v4AxleTrucks.assign(12, range);
	m_v5AxleTrucks.assign(12, range);
}

CAxleWeight45::~CAxleWeight45()
{

}

void CAxleWeight45::AddGVWRange(std::vector<double> data, int iTruck, int iRange)
{
	GVWRange range;
	range.W1.Mean = data[0];
	range.W2.Mean = data[1];
	range.WT.Mean = data[2];
	range.W1.StdDev = data[3];
	range.W2.StdDev = data[4];
	range.WT.StdDev = data[5];

	if( iTruck == 4)
		m_v4AxleTrucks[iRange] = range;
	else
		m_v5AxleTrucks[iRange] = range;
}

std::vector<double> CAxleWeight45::GetGVWRange(int iTruck, int iRange)
{
	std::vector<double> data;
	GVWRange range;
	if( iTruck == 4)
		range = m_v4AxleTrucks[iRange];
	else
		range = m_v5AxleTrucks[iRange];

	data.push_back(	range.W1.Mean );
	data.push_back(	range.W2.Mean );
	data.push_back(	range.WT.Mean );
	data.push_back(	range.W1.StdDev );
	data.push_back(	range.W2.StdDev );
	data.push_back(	range.WT.StdDev );

	return data;
}
