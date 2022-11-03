// AxleWeight23.cpp: implementation of the CAxleWeight23 class.
//
//////////////////////////////////////////////////////////////////////

#include "AxleWeight23.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAxleWeight23::CAxleWeight23()
{

}

CAxleWeight23::~CAxleWeight23()
{

}

void CAxleWeight23::Add2AxleData(std::vector<CTriModalNormal> vAxle)
{
	m_v2AxleData = vAxle;
}

void CAxleWeight23::Add3AxleData(std::vector<CTriModalNormal> vAxle)
{
	m_v3AxleData = vAxle;
}


CTriModalNormal CAxleWeight23::GetAxleDist(int iTruck, int iAxle)
{
	// this function accepts the integers as being 1-based
	if( iTruck == 2 )
		return m_v2AxleData[iAxle-1];
	else
		return m_v3AxleData[iAxle-1];
}
