#pragma once

#include "Lane.h"
#include "VehicleGenerator.h"
#include "Distribution.h"
#include "LaneFlow.h"

class CLaneGenTraffic :
	public CLane
{
public:
	CLaneGenTraffic(void);
	~CLaneGenTraffic(void);

	virtual CVehicle* GetNextVehicle();
	
	void SetParams(CVehicleGenerator* pGen, std::vector<double> vQ, double CarPerc, int iLane, int iDir);
	void setLaneData(CVehicleGenerator* pGen, CLaneFlow lane_flow, double starttime);

private:
	void	GenNextArrival();
	double	GetMinGap();
	void	GenNextVehicle();
	double	inv_quad(double a, double b, double c, double y);
	double	quad(double a, double b, double c, double x);
	double	GenGap_Poisson();
	double	GenGap_Cong();
	double	GenGap_NHM();
	void	SetGapGeneration();
	void	GenNextTime();
	void	UpdateHour();
	void	UpdateFlowProperties(void);
	
	CVehicle* m_pPrevVeh;
	int		m_CurHour;
	CDistribution m_RNG;
	CVehicleGenerator* m_pGenerator;
	CLaneFlow m_LaneFlow;
	std::vector< std::vector<double> > m_vNHM;
	
	// std::vector<double> m_vFlowRate;
	double m_vFlowRate;
	double m_CarPerc;	

	double m_BufferGap;
	
	double CONGESTED_GAP;
	double CONGESTED_GAP_COEF_VAR;
	double CONGESTED_SPEED;
	int HEADWAY_MODEL;
	double NO_OVERLAP_LENGTH;
};

