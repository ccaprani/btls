// VehicleGenerator.h: interface for the CVehicleGenerator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VEHICLEGENERATOR_H__CD28CD98_79D8_447A_988C_8EB02B19CA73__INCLUDED_)
#define AFX_VEHICLEGENERATOR_H__CD28CD98_79D8_447A_988C_8EB02B19CA73__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include "Vehicle.h"
#include "TrafficData.h"
#include "Distribution.h"
#include "LaneFlow.h"

class CVehicleGenerator  
{
public:
	std::vector< std::vector<double> > GetNHM();
	void GenerateTruck45(CVehicle* pVeh, int nAxles);
	void GenerateTruck23(CVehicle* pVeh, int nAxles);
	//CVehicle* Generate(int iLane, int dir);
	CVehicle* Generate(int iHour);
	CVehicleGenerator(CTrafficData TD, CLaneFlow LF);
	virtual ~CVehicleGenerator();

private:
	double GenerateSpeed();
	void GenerateCar(CVehicle* pVeh);
	bool NextVehicleIsCar();	
	void GenerateCommonProps(CVehicle* pVeh, int nAxles);
	void ScaleVector(std::vector<double>& vec, double scale);
	double SumVector(std::vector<double> vec);
	
	int m_CurLane;
	int m_CurDirection;
	double m_Time;
	int m_CurHour;
	int TruckType();

	CTrafficData m_TD;
	CDistribution m_RNG;
	CLaneFlow m_LaneFlow;

	double CONGESTED_SPEED;
	int HEADWAY_MODEL;
	double LANE_ECCENTRICITY_STD;
};

#endif // !defined(AFX_VEHICLEGENERATOR_H__CD28CD98_79D8_447A_988C_8EB02B19CA73__INCLUDED_)
