// AxleWeight23.h: interface for the CAxleWeight23 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AXLEWEIGHT23_H__C9129144_60EA_46A6_B604_55DE6A7803BB__INCLUDED_)
#define AFX_AXLEWEIGHT23_H__C9129144_60EA_46A6_B604_55DE6A7803BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MultiModalNormal.h"

class CAxleWeight23  
{
public:
	CMultiModalNormal GetAxleDist(int iTruck, int iAxle);
	void Add3AxleData(std::vector<CMultiModalNormal> vAxle);
	void Add2AxleData(std::vector<CMultiModalNormal> vAxle);
	
	CAxleWeight23();
	virtual ~CAxleWeight23();

private:
	std::vector<CMultiModalNormal> m_v2AxleData;
	std::vector<CMultiModalNormal> m_v3AxleData;

};

#endif // !defined(AFX_AXLEWEIGHT23_H__C9129144_60EA_46A6_B604_55DE6A7803BB__INCLUDED_)
