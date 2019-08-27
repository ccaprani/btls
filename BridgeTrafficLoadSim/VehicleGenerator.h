// VehicleGenerator.h: interface for the CVehicleGenerator class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include "Generator.h"
#include "Vehicle.h"
#include "VehicleModelData.h"
#include "FlowModelData.h"
#include "TrafficData.h"
#include "LaneFlow.h"

class CVehicleGenerator : public CGenerator  
{
public:
	CVehicleGenerator(EVehicleModel vm, CVehicleModelData* pVMD);
	virtual ~CVehicleGenerator();
	
	CVehicle* Generate(int iHour);
	virtual void update(CFlowModelData* pFMD);

protected:
	virtual void GenerateVehicle(CVehicle* pVeh) = 0;
	virtual size_t GenVehClass() = 0;
	
	void GenerateCar(CVehicle* pVeh);
	bool NextVehicleIsCar();	
	
	CVehicleModelData* m_pVehModelData;
	EVehicleModel m_VehModel;
	bool m_bModelHasCars;

	size_t m_CurLane;
	size_t m_CurDirection;
	double m_Time;
	int m_CurHour;
	
	CVehicleClassification* m_pVehClassification;

private:

};
