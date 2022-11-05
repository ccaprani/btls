// MultiModalNormal.cpp: implementation of the CMultiModalNormal class.
//
//////////////////////////////////////////////////////////////////////

#include "MultiModalNormal.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMultiModalNormal::CMultiModalNormal()
{

}

CMultiModalNormal::~CMultiModalNormal()
{

}

void CMultiModalNormal::AddMode(double w, double m, double s)
{
	Mode temp;
	temp.Weight = w;
	temp.Mean = m;
	temp.StdDev = s;

	m_vModes.push_back(temp);
}
