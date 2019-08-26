// VehicleGenerator.cpp: implementation of the CVehicleGenerator class.
//
//////////////////////////////////////////////////////////////////////

#include "VehicleGenerator.h"
#include "ConfigData.h"

extern CConfigData g_ConfigData;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVehicleGenerator::CVehicleGenerator(CVehicleModelData* pVMD, EVehicleModel vm)
	: m_pVehModelData(pVMD), m_VehModel(vm)
{
	HEADWAY_MODEL = g_ConfigData.Traffic.HEADWAY_MODEL;
	VEHICLE_MODEL = g_ConfigData.Traffic.VEHICLE_MODEL;

	m_pVehClassification = pVMD->getVehClassification();
	m_LaneFlow = pVMD->getLaneFlow();
}

CVehicleGenerator::~CVehicleGenerator()
{

}

CVehicle* CVehicleGenerator::Generate(int iHour)
{
	CVehicle* pVeh = new CVehicle;
	m_CurHour = iHour;

	GenerateVehicle(pVeh);

	return pVeh;
}

bool CVehicleGenerator::NextVehicleIsCar()
{
	// cars are only possible with HW of 5 (congestion) or 6 (FF w/ cars)
	if(HEADWAY_MODEL == 5 || HEADWAY_MODEL == 6)
	{
		double prop = m_RNG.GenerateUniform();
		// if( prop > m_TD.GetPercent_Cars() )
		if( prop > m_LaneFlow.getCP_cars(m_CurHour) )
			return false;	// is not a car
		
		return true;		// is a car
	}
	else
		return false;
}

void CVehicleGenerator::GenerateCar(CVehicle *pVeh)
{
	pVeh->setNoAxles(2);
	pVeh->setGVW(20.0);
	pVeh->setAW(0,10.0);
	pVeh->setAW(1,10.0);
	pVeh->setLength(4.0);	// in m
	pVeh->setAS(0, 4.0);	// in m
	pVeh->setAS(1, 0.0);
}