// VehicleGenerator.cpp: implementation of the CVehicleGenerator class.
//
//////////////////////////////////////////////////////////////////////

#include "VehicleGenerator.h"
#include "ConfigData.h"

extern CConfigData g_ConfigData;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVehicleGenerator::CVehicleGenerator(EVehicleModel vm, CVehicleModelData* pVMD)
	: m_VehModel(vm), m_pVehModelData(pVMD), m_pVehClassification(NULL)
{
	if (m_pVehModelData != NULL) // not all models have data
		m_pVehClassification = m_pVehModelData->getVehClassification();
}

CVehicleGenerator::~CVehicleGenerator()
{

}

void CVehicleGenerator::update(CFlowModelData* pFMD)
{
	m_bModelHasCars = pFMD->getModelHasCars();
	m_vCarPercent = pFMD->getCarPercent();
}

CVehicle* CVehicleGenerator::Generate(int iHour)
{
	// Must generate a new CVehicle because once off the bridge
	// the object is deleted
	CVehicle* pVeh = new CVehicle;
	m_CurHour = iHour;

	GenerateVehicle(pVeh);

	return pVeh;
}

bool CVehicleGenerator::NextVehicleIsCar()
{
	if (m_bModelHasCars)
	{
		double prop = m_RNG.GenerateUniform();
		if (prop > m_vCarPercent.at(m_CurHour))
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