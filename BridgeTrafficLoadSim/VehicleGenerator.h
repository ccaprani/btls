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
	CVehicleGenerator(EVehicleModel vm, CVehicleModelData_ptr pVMD);
	virtual ~CVehicleGenerator();
	
	CVehicle_ptr Generate(int iHour);
	virtual void update(CFlowModelData_ptr pFMD);

protected:
	virtual void GenerateVehicle(CVehicle_ptr pVeh) = 0;
	virtual size_t GenVehClass() = 0;
	
	void GenerateCar(CVehicle_ptr pVeh);
	bool NextVehicleIsCar();	
	
	CVehicleModelData_ptr m_pVehModelData;
	EVehicleModel m_VehModel;
	bool m_bModelHasCars;

	vec m_vCarPercent;

	size_t m_CurLane;
	size_t m_CurDirection;
	double m_Time;
	int m_CurHour;
	
	CVehicleClassification_ptr m_pVehClassification;

private:

};
typedef std::shared_ptr<CVehicleGenerator> CVehicleGenerator_ptr;
