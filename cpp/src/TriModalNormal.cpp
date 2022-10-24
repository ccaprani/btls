// TriModalNormal.cpp: implementation of the CTriModalNormal class.
//
//////////////////////////////////////////////////////////////////////

#include "TriModalNormal.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTriModalNormal::CTriModalNormal()
{

}

CTriModalNormal::~CTriModalNormal()
{

}

void CTriModalNormal::AddMode(double w, double m, double s)
{
	Mode temp;
	temp.Weight = w;
	temp.Mean = m;
	temp.StdDev = s;

	m_vModes.push_back(temp);
}
