// TrafficData.cpp: implementation of the CTrafficData class.
//
//////////////////////////////////////////////////////////////////////

#include "TrafficData.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTrafficData::CTrafficData()
{
	GVWRange range;
	m_v4AxleWeight.assign(12, range);
	m_v5AxleWeight.assign(12, range);
}

CTrafficData::~CTrafficData()
{

}

////////// THE GETS ////////////////

CTriModalNormal CTrafficData::GetSpacingDist(size_t iTruck, size_t iSpace)
{
	switch( iTruck )
	{
		case 2:
			return m_v2AxleSpacings[iSpace];
		case 3:
			return m_v3AxleSpacings[iSpace];
		case 4:
			return m_v4AxleSpacings[iSpace];
		default:
			return m_v5AxleSpacings[iSpace];
	}
}

CTriModalNormal CTrafficData::GetAxleWeightDist(size_t iTruck, size_t iAxle)
{
	if( iTruck == 2 )
		return m_v2AxleWeight[iAxle];
	else
		return m_v3AxleWeight[iAxle];
}

CTriModalNormal	CTrafficData::GetTrackWidthDist(size_t iTruck, size_t iAxle)
{
	switch( iTruck )
	{
		case 2:
			return m_v2AxleTrackWidth[iAxle];
		case 3:
			return m_v3AxleTrackWidth[iAxle];
		case 4:
			return m_v4AxleTrackWidth[iAxle];
		default:
			return m_v5AxleTrackWidth[iAxle];
	}
}

std::vector<double> CTrafficData::GetGVWRange(size_t iTruck, size_t iRange)
{
	std::vector<double> data;
	GVWRange range;
	if(iRange > 11)
		iRange = 11;
	if( iTruck == 4)
		range = m_v4AxleWeight[iRange];
	else
		range = m_v5AxleWeight[iRange];

	data.push_back(	range.W1.Mean );
	data.push_back(	range.W2.Mean );
	data.push_back(	range.WT.Mean );
	data.push_back(	range.W1.StdDev );
	data.push_back(	range.W2.StdDev );
	data.push_back(	range.WT.StdDev );

	return data;
}

CTriModalNormal CTrafficData::GetGVW(size_t dir, size_t iTruck)
{
	if(dir == 1)
		return m_vDir1GVW[iTruck - 2];
	else
		return m_vDir2GVW[iTruck - 2];
}	

CTriModalNormal CTrafficData::GetSpeed(size_t dir)
{
	if (dir == 1)
		return m_vSpeed[0];
	else
		return m_vSpeed[1];
}

/////////////// THE SETS //////////////////

void CTrafficData::Add2AxleSpacings(std::vector<CTriModalNormal> vSpace)
{
	m_v2AxleSpacings = vSpace;
}

void CTrafficData::Add3AxleSpacings(std::vector<CTriModalNormal> vSpace)
{
	m_v3AxleSpacings = vSpace;
}

void CTrafficData::Add4AxleSpacings(std::vector<CTriModalNormal> vSpace)
{
	m_v4AxleSpacings = vSpace;
}

void CTrafficData::Add5AxleSpacings(std::vector<CTriModalNormal> vSpace)
{
	m_v5AxleSpacings = vSpace;
}

void CTrafficData::Add2AxleTrackWidth(std::vector<CTriModalNormal> vTrack)
{
	m_v2AxleTrackWidth = vTrack;
}

void CTrafficData::Add3AxleTrackWidth(std::vector<CTriModalNormal> vTrack)
{
	m_v3AxleTrackWidth = vTrack;
}

void CTrafficData::Add4AxleTrackWidth(std::vector<CTriModalNormal> vTrack)
{
	m_v4AxleTrackWidth = vTrack;
}

void CTrafficData::Add5AxleTrackWidth(std::vector<CTriModalNormal> vTrack)
{
	m_v5AxleTrackWidth = vTrack;
}

void CTrafficData::Add2AxleWeight(std::vector<CTriModalNormal> vAxle)
{
	m_v2AxleWeight = vAxle;
}

void CTrafficData::Add3AxleWeight(std::vector<CTriModalNormal> vAxle)
{
	m_v3AxleWeight = vAxle;
}

void CTrafficData::Add45AxleWeight(std::vector<double> data, size_t iTruck, size_t iRange)
{
	GVWRange range;
	range.W1.Mean = data[0];
	range.W2.Mean = data[1];
	range.WT.Mean = data[2];
	range.W1.StdDev = data[3];
	range.W2.StdDev = data[4];
	range.WT.StdDev = data[5];

	if( iTruck == 4)
		m_v4AxleWeight[iRange] = range;
	else
		m_v5AxleWeight[iRange] = range;
}

void CTrafficData::AddGVW(int dir, std::vector<CTriModalNormal> vGVW)
{
	if(dir == 1)
		m_vDir1GVW = vGVW;
	else
		m_vDir2GVW = vGVW;
}

void CTrafficData::AddSpeed(std::vector<CTriModalNormal> vSpeed)
{
	m_vSpeed = vSpeed;
}