// TrafficData.h: interface for the CTrafficData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TRAFFICDATA_H__30B186A2_C2CB_47A5_BB16_7566527FC87B__INCLUDED_)
#define AFX_TRAFFICDATA_H__30B186A2_C2CB_47A5_BB16_7566527FC87B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TriModalNormal.h"
#include "LaneFlow.h"

class CTrafficData  
{
public:

	CTrafficData();
	virtual ~CTrafficData();

	CTriModalNormal			GetSpacingDist(int iTruck, int iSpace);
	CTriModalNormal			GetAxleWeightDist(int iTruck, int iAxle);
	CTriModalNormal			GetTrackWidthDist(int iTruck, int iAxle);
	//CTriModalNormal			GetSpeed(int dir);
	CTriModalNormal			GetGVW(int dir, int iTruck);

	std::vector< std::vector<double> > GetNHM();
	//std::vector< std::vector<double> > GetFlowRate();
	
	std::vector<double>		GetGVWRange(int iTruck, int iRange);
	std::vector<double>		GetClassPercent(int iLane);	
	std::vector<double>		GetFlowRate(int iLane);
	
	//int						GetNoLanes();
	//double					GetClassPercent(int iLane, int iClass);	
	//double					GetFlowRate(int iLane, int iHour);
	//double					GetPercent_OtherVehs();
	//double					GetPercent_Cars();	
	
	void Add2AxleSpacings(std::vector<CTriModalNormal> vSpace);
	void Add3AxleSpacings(std::vector<CTriModalNormal> vSpace);
	void Add4AxleSpacings(std::vector<CTriModalNormal> vSpace);
	void Add5AxleSpacings(std::vector<CTriModalNormal> vSpace);

	void Add2AxleTrackWidth(std::vector<CTriModalNormal> vSpace);
	void Add3AxleTrackWidth(std::vector<CTriModalNormal> vSpace);
	void Add4AxleTrackWidth(std::vector<CTriModalNormal> vSpace);
	void Add5AxleTrackWidth(std::vector<CTriModalNormal> vSpace);

	void Add2AxleWeight(std::vector<CTriModalNormal> vAxle);
	void Add3AxleWeight(std::vector<CTriModalNormal> vAxle);
	void Add45AxleWeight(std::vector<double> data, int iTruck, int iRange);
	
	void AddGVW(int dir, std::vector<CTriModalNormal> vGVW);
	void AddSpeed(std::vector<CTriModalNormal> vSpeed);	
	//void AddClassPercent(int iLane, int iClass, double val);
	
	//void SetCarPercent(double perc);
	void SetNHM( std::vector< std::vector<double> > NHM );
	//void SetFlowRate( std::vector< std::vector<double> > vFlowRate );
	void SetLaneFlow(std::vector<CLaneFlow> lane_flow);
	size_t getNoDirn(void);
	size_t getNoLanes(void);
	size_t getNoLanesDir1(void);
	size_t getNoLanesDir2(void);
	
private:
	double m_CarPercent;
	std::vector<CTriModalNormal> m_v2AxleSpacings;
	std::vector<CTriModalNormal> m_v3AxleSpacings;
	std::vector<CTriModalNormal> m_v4AxleSpacings;
	std::vector<CTriModalNormal> m_v5AxleSpacings;

	std::vector<CTriModalNormal> m_v2AxleTrackWidth;
	std::vector<CTriModalNormal> m_v3AxleTrackWidth;
	std::vector<CTriModalNormal> m_v4AxleTrackWidth;
	std::vector<CTriModalNormal> m_v5AxleTrackWidth;

	std::vector<CTriModalNormal> m_v2AxleWeight;
	std::vector<CTriModalNormal> m_v3AxleWeight;

	std::vector<CTriModalNormal> m_vDir1GVW;
	std::vector<CTriModalNormal> m_vDir2GVW;
	std::vector<CTriModalNormal> m_vSpeed;

	struct Dist
	{
		double Mean;
		double StdDev;
	};

	struct GVWRange
	{
		Dist W1;
		Dist W2;
		Dist WT;
	};
	
	int m_NoDirn;
	size_t m_NoLanes;
	int m_NoLanesDir1;
	int m_NoLanesDir2;

	std::vector<GVWRange> m_v4AxleWeight;
	std::vector<GVWRange> m_v5AxleWeight;

	//std::vector< std::vector<double> > m_vCP;
	std::vector< std::vector<double> > m_vNHM;
	//std::vector< std::vector<double> > m_vFlowRate;
	std::vector<CLaneFlow> m_vLaneFlow;
};

#endif // !defined(AFX_TRAFFICDATA_H__30B186A2_C2CB_47A5_BB16_7566527FC87B__INCLUDED_)
