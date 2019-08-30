#pragma once

#include "Lane.h"
#include "VehicleGenerator.h"
#include "Distribution.h"
#include "LaneFlowComposition.h"
#include "FlowGenerator.h"
#include "VehicleGenGrave.h"
#include "VehicleGenConstant.h"
#include "VehicleGenGarage.h"

class CLaneGenTraffic : public CLane
{
public:
	CLaneGenTraffic(void);
	~CLaneGenTraffic(void);

	virtual CVehicle_ptr GetNextVehicle();
	
	void setLaneData(CVehicleClassification_ptr pVC, CLaneFlowComposition lfc, double starttime);

private:
	void	GenNextArrival();
	void	GenNextTime();
	void	GenNextVehicle();
	
	CVehicleGenerator_ptr m_pVehicleGen;
	CVehicleModelData_ptr m_pVehModelData;
	CFlowGenerator_ptr m_pFlowGen;
	CFlowModelData_ptr m_pFlowModelData;

	CVehicle_ptr m_pPrevVeh;
	CVehicle_ptr m_pNextVeh;
	
	int		HEADWAY_MODEL;
	int		VEHICLE_MODEL;
	size_t	NO_LANES;
};
typedef std::shared_ptr<CLaneGenTraffic> CLaneGenTraffic_ptr;

