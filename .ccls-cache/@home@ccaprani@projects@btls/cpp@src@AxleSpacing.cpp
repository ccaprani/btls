// AxleSpacing.cpp: implementation of the CAxleSpacing class.
//
//////////////////////////////////////////////////////////////////////

#include "AxleSpacing.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAxleSpacing::CAxleSpacing()
{

}

CAxleSpacing::~CAxleSpacing()
{

}

CMultiModalNormal CAxleSpacing::GetSpacingDist(int iTruck, int iSpace)
{
	// this function accepts the integers as being 1-based
	switch( iTruck )
	{
		case 2:
			return m_v2AxleData[iSpace-1];
		case 3:
			return m_v3AxleData[iSpace-1];
		case 4:
			return m_v4AxleData[iSpace-1];
		default: // case 5
			return m_v5AxleData[iSpace-1];
	}
}

void CAxleSpacing::Add2AxleData(std::vector<CMultiModalNormal> vSpace)
{
	m_v2AxleData = vSpace;
}

void CAxleSpacing::Add3AxleData(std::vector<CMultiModalNormal> vSpace)
{
	m_v3AxleData = vSpace;
}

void CAxleSpacing::Add4AxleData(std::vector<CMultiModalNormal> vSpace)
{
	m_v4AxleData = vSpace;
}

void CAxleSpacing::Add5AxleData(std::vector<CMultiModalNormal> vSpace)
{
	m_v5AxleData = vSpace;
}