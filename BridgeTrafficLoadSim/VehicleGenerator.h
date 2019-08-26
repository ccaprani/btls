// VehicleGenerator.h: interface for the CVehicleGenerator class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include "Generator.h"
#include "Vehicle.h"
#include "VehicleModelData.h"
#include "TrafficData.h"
#include "LaneFlow.h"

class CVehicleGenerator : public CGenerator  
{
public:
	CVehicleGenerator(CVehicleModelData* pVMD, EVehicleModel vm);
	virtual ~CVehicleGenerator();
	
	CVehicle* Generate(int iHour);

protected:
	virtual void GenerateVehicle(CVehicle* pVeh) = 0;
	virtual size_t GenVehClass() = 0;
	
	void GenerateCar(CVehicle* pVeh);
	bool NextVehicleIsCar();	
	
	CVehicleModelData* m_pVehModelData;
	EVehicleModel m_VehModel;

	size_t m_CurLane;
	size_t m_CurDirection;
	double m_Time;
	int m_CurHour;
	
	CVehicleClassification* m_pVehClassification;
	CLaneFlow m_LaneFlow;	

private:
	int		HEADWAY_MODEL;
	int		VEHICLE_MODEL;
	double	LANE_ECCENTRICITY_STD;
	size_t	NO_LANES;
};
