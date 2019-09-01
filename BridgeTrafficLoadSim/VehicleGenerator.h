// VehicleGenerator.h: interface for the CVehicleGenerator class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <vector>
#include "Generator.h"
#include "Vehicle.h"
#include "VehicleModelData.h"
#include "FlowModelData.h"
#include "TrafficData.h"

class CVehicleGenerator : public CGenerator  
{
public:
	CVehicleGenerator(EVehicleModel vm, CVehicleModelData_sp pVMD);
	virtual ~CVehicleGenerator();
	
	CVehicle_sp Generate(int iHour);
	virtual void update(CFlowModelData_sp pFMD);

protected:
	virtual void GenerateVehicle(CVehicle_sp pVeh) = 0;
	virtual size_t GenVehClass() = 0;
	
	void GenerateCar(CVehicle_sp pVeh);
	bool NextVehicleIsCar();	
	
	CVehicleModelData_sp m_pVehModelData;
	EVehicleModel m_VehModel;
	bool m_bModelHasCars;

	vec m_vCarPercent;

	size_t m_CurLane;
	size_t m_CurDirection;
	double m_Time;
	int m_CurHour;
	
	CVehicleClassification_sp m_pVehClassification;

private:

};
typedef std::shared_ptr<CVehicleGenerator> CVehicleGenerator_sp;
