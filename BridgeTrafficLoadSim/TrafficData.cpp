// TrafficData.cpp: implementation of the CTrafficData class.
//
//////////////////////////////////////////////////////////////////////

#include "TrafficData.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTrafficData::CTrafficData()
{
	m_NoLanes = 0;

	GVWRange range;
	m_v4AxleWeight.assign(12, range);
	m_v5AxleWeight.assign(12, range);
}

CTrafficData::~CTrafficData()
{

}

////////// THE GETS ////////////////

CTriModalNormal CTrafficData::GetSpacingDist(int iTruck, int iSpace)
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

CTriModalNormal CTrafficData::GetAxleWeightDist(int iTruck, int iAxle)
{
	if( iTruck == 2 )
		return m_v2AxleWeight[iAxle];
	else
		return m_v3AxleWeight[iAxle];
}

CTriModalNormal	CTrafficData::GetTrackWidthDist(int iTruck, int iAxle)
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

std::vector<double> CTrafficData::GetGVWRange(int iTruck, int iRange)
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

//double CTrafficData::GetClassPercent(int iLane, int iClass)
//{
//	double val = 0.0;
//
//	if(iLane < m_NoLanes)
//	{
//		val = m_vCP[iLane][iClass-2]; 
//	}
//	return val;
//}

//std::vector<double> CTrafficData::GetClassPercent(int iLane)
//{
//	return m_vCP[iLane];
//}

//double CTrafficData::GetPercent_Cars()
//{
//	return m_CarPercent;
//}

//int CTrafficData::GetNoLanes()
//{
//	return m_NoLanes;
//}

//double CTrafficData::GetPercent_OtherVehs()
//{
//	return 1 - m_CarPercent;
//}

CTriModalNormal CTrafficData::GetGVW(int dir, int iTruck)
{
	if(dir == 1)
		return m_vDir1GVW[iTruck - 2];
	else
		return m_vDir2GVW[iTruck - 2];
}	

//CTriModalNormal CTrafficData::GetSpeed(int dir)
//{
//	if(dir == 1)
//		return m_vSpeed[0];
//	else
//		return m_vSpeed[1];
//}

std::vector< std::vector<double> > CTrafficData::GetNHM()
{
	return m_vNHM;
}

//std::vector< std::vector<double> > CTrafficData::GetFlowRate()
//{
//	return m_vFlowRate;
//}

//double CTrafficData::GetFlowRate(int iLane, int iHour)
//{
//	return m_vFlowRate[iLane][iHour];
//}

//std::vector<double> CTrafficData::GetFlowRate(int iLane)
//{
//	return m_vFlowRate[iLane];
//}

int CTrafficData::getNoDirn(void)
{
	return m_NoDirn;
}


int CTrafficData::getNoLanes(void)
{
	return m_NoLanes;
}


int CTrafficData::getNoLanesDir1(void)
{
	return m_NoLanesDir1;
}


int CTrafficData::getNoLanesDir2(void)
{
	return m_NoLanesDir2;
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

void CTrafficData::Add45AxleWeight(std::vector<double> data, int iTruck, int iRange)
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

//void CTrafficData::AddClassPercent(int iLane, int iClass, double val)
//{
//	if( iLane >= m_NoLanes )
//	{
//		std::vector<double> temp(4, 0.0);
//		m_NoLanes++;
//		m_vCP.push_back(temp);
//	}
//	
//	m_vCP[iLane][iClass-2] = val; 
//}

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

void CTrafficData::SetNHM(	std::vector< std::vector<double> > NHM )
{
	m_vNHM = NHM;
}

//void CTrafficData::SetCarPercent(double perc)
//{
//	m_CarPercent = perc;
//}
//
//void CTrafficData::SetFlowRate(	std::vector< std::vector<double> > vFlowRate )
//{
//	m_vFlowRate = vFlowRate;
//}

void CTrafficData::SetLaneFlow(std::vector<CLaneFlow> lane_flow)
{
	m_vLaneFlow = lane_flow;
	m_NoLanes = m_vLaneFlow.size();
	m_NoDirn = 0;
	m_NoLanesDir1 = 0;
	m_NoLanesDir2 = 0;
	for(int i = 0; i < m_NoLanes; ++i)
	{
		int cur_dir = m_vLaneFlow.at(i).getDirn();
		if(cur_dir > m_NoDirn) m_NoDirn = cur_dir;
		if(cur_dir == 1)
			++m_NoLanesDir1;
		else
			++m_NoLanesDir2;
	}

}
