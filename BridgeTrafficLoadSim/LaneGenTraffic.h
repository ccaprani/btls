#pragma once

#include "Lane.h"
#include "VehicleGenerator.h"
#include "Distribution.h"
#include "LaneFlow.h"
#include "FlowGenerator.h"
#include "VehicleGenGrave.h"

class CLaneGenTraffic : public CLane
{
public:
	CLaneGenTraffic(void);
	~CLaneGenTraffic(void);

	virtual CVehicle* GetNextVehicle();
	
	void setLaneData(CVehicleClassification* pVC, CTrafficData TD, CLaneFlow lane_flow, double starttime);

private:
	void	GenNextArrival();
	void	GenNextTime();
	void	GenNextVehicle();
	
	CVehicleGenerator* m_pVehicleGen;
	CVehicleModelData* m_pVehModelData;
	CFlowGenerator* m_pFlowGen;
	CFlowModelData* m_pFlowModelData;

	CVehicle* m_pPrevVeh;
	CVehicle* m_pNextVeh;
	
	int		HEADWAY_MODEL;
	int		VEHICLE_MODEL;
	size_t	NO_LANES;
};

