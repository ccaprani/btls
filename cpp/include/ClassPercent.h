// ClassPercent.h: interface for the CClassPercent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLASSPERCENT_H__8B0D33B8_B1E1_47EB_9BAC_3A2586719A60__INCLUDED_)
#define AFX_CLASSPERCENT_H__8B0D33B8_B1E1_47EB_9BAC_3A2586719A60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>

class CClassPercent  
{
public:
	int GetNoLanes();
	double GetClassPercent(int iLane, int iClass);
	void AddClassPercent(int iLane, int iClass, double val);
	CClassPercent();
	virtual ~CClassPercent();

private:
	int m_NoLanes;
	struct CP
	{
		double CP_2Axle;
		double CP_3Axle;
		double CP_4Axle;
		double CP_5Axle;
	};
		
	std::vector<CP> m_vCP;
};

#endif // !defined(AFX_CLASSPERCENT_H__8B0D33B8_B1E1_47EB_9BAC_3A2586719A60__INCLUDED_)
