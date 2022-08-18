// TrafficData.h: interface for the CTrafficData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TRAFFICDATA_H__30B186A2_C2CB_47A5_BB16_7566527FC87B__INCLUDED_)
#define AFX_TRAFFICDATA_H__30B186A2_C2CB_47A5_BB16_7566527FC87B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TriModalNormal.h"

class CTrafficData  
{
public:
	CTrafficData();
	virtual ~CTrafficData();

	std::vector<double>		GetGVWRange(std::size_t iTruck, std::size_t iRange);
	CTriModalNormal			GetSpacingDist(std::size_t iTruck, std::size_t iSpace);
	CTriModalNormal			GetAxleWeightDist(std::size_t iTruck, std::size_t iAxle);
	CTriModalNormal			GetTrackWidthDist(std::size_t iTruck, std::size_t iAxle);
	CTriModalNormal			GetGVW(std::size_t dir, std::size_t iTruck);

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
	void Add45AxleWeight(std::vector<double> data, std::size_t iTruck, std::size_t iRange);
	
	void AddGVW(int dir, std::vector<CTriModalNormal> vGVW);
	
	// Speed is kept here as part of the class modelling
	void AddSpeed(std::vector<CTriModalNormal> vSpeed);	
	CTriModalNormal			GetSpeed(std::size_t dir);
	
private:
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
	
	std::vector<GVWRange> m_v4AxleWeight;
	std::vector<GVWRange> m_v5AxleWeight;
};

#endif // !defined(AFX_TRAFFICDATA_H__30B186A2_C2CB_47A5_BB16_7566527FC87B__INCLUDED_)