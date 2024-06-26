#pragma once

#include "Lane.h"
#include "VehicleGenerator.h"
#include "Distribution.h"
#include "LaneFlowComposition.h"
#include "FlowGenerator.h"
#include "VehicleGenGrave.h"
#include "VehicleGenNominal.h"
#include "VehicleGenGarage.h"

class CLaneGenTraffic : public CLane
{
public:
	CLaneGenTraffic(CConfigDataCore& config);
	~CLaneGenTraffic(void);

	virtual CVehicle_sp GetNextVehicle();
	
	void setLaneData(CVehicleClassification_sp pVC, CLaneFlowComposition lfc, const double starttime);
	void setLaneData(CLaneFlowComposition lfc, CVehicleGenerator_sp pVehicleGen, CFlowGenerator_sp pFlowGen, const double startTime);

	void initLane(CFlowModelData_sp pFlowModelData);

private:
	void GenNextArrival();
	void GenNextTime();
	void GenNextVehicle();

	inline void setLaneAttributes(CLaneFlowComposition lfc, const double startTime) {
		m_NextArrivalTime = startTime;
		m_Direction = lfc.getDirn();
		m_LaneIndex = lfc.getGlobalLaneNo();	// Map vehicles to global lane using zero based cumulative lane no.
	}

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

