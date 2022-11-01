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
	CLaneGenTraffic(CConfigDataCore& config);
	~CLaneGenTraffic(void);

	virtual CVehicle_sp GetNextVehicle();
	
	void setLaneData(CVehicleClassification_sp pVC, CLaneFlowComposition lfc, const double starttime);
	void setLaneData(CVehicleClassification_sp pVC, CLaneFlowComposition lfc, const double starttime, CConfigDataCore& config);

private:
	void GenNextArrival();
	void GenNextTime();
	void GenNextVehicle();

	CConfigDataCore& m_Config;
	
	CVehicleGenerator_sp m_pVehicleGen;
	CVehicleModelData_sp m_pVehModelData;
	CFlowGenerator_sp m_pFlowGen;
	CFlowModelData_sp m_pFlowModelData;

	CVehicle_sp m_pPrevVeh;
	CVehicle_sp m_pNextVeh;
	
	int		HEADWAY_MODEL;
	int		VEHICLE_MODEL;
	size_t	NO_LANES;
};
typedef std::shared_ptr<CLaneGenTraffic> CLaneGenTraffic_sp;

