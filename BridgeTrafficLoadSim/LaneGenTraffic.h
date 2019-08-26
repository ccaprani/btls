#pragma once

#include "Lane.h"
#include "VehicleGenerator.h"
#include "Distribution.h"
#include "LaneFlow.h"
#include "FlowGenerator.h"

class CLaneGenTraffic : public CLane
{
public:
	CLaneGenTraffic(void);
	~CLaneGenTraffic(void);

	virtual CVehicle* GetNextVehicle();
	
	void setLaneData(CVehicleGenerator* pGen, CLaneFlow lane_flow, double starttime);

private:
	void	GenNextArrival();
	void	GenNextTime();
	void	GenNextVehicle();
	
	CVehicleGenerator* m_pVehicleGen;
	CFlowGenerator* m_pFlowGen;
	CFlowModelData* m_pFlowModelData;

	CVehicle* m_pPrevVeh;
	CVehicle* m_pNextVeh;
	
	double	CONGESTED_GAP;
	double	CONGESTED_GAP_COEF_VAR;
	double	CONGESTED_SPEED;
	int		HEADWAY_MODEL;
	double	NO_OVERLAP_LENGTH;
};

