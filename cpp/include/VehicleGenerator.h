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

class CVehicleGenerator : public CGenerator  
{
public:
	CVehicleGenerator(EVehicleModel vm, CVehicleModelData_sp pVMD);
	virtual ~CVehicleGenerator();
	
	CVehicle_sp Generate(size_t iHour);
	virtual void update(CFlowModelData_sp pFMD);
	void setModelHasCars(bool modelHasCars);
	void setCarPercent(vec carPercent);

protected:
	virtual void GenerateVehicle(CVehicle_sp pVeh);
	virtual size_t GenVehClass() = 0;
	
	void GenerateCar(CVehicle_sp pVeh);
	bool NextVehicleIsCar();
	
	EVehicleModel m_VehModel;
	CVehicleModelData_sp m_pVehModelData;
	bool m_bModelHasCars;

	vec m_vCarPercent;

	size_t m_CurLane;
	size_t m_CurDirection;
	double m_Time;
	size_t m_CurHour;
	
	CVehicleClassification_sp m_pVehClassification;

	double (CDistribution::*m_pKernelGenerator)(double, double);

private:
	void SetKernelGenerator();

};
typedef std::shared_ptr<CVehicleGenerator> CVehicleGenerator_sp;
