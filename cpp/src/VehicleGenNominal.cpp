#include "VehicleGenNominal.h"


CVehicleGenNominal::CVehicleGenNominal(CVehModelDataNominal_sp pVMD)	
	: CVehicleGenerator(eVM_Constant, pVMD), m_NominalVehicle(nullptr), m_MinimumCOV(1e-3)
{
	m_pVMD = std::dynamic_pointer_cast<CVehModelDataNominal>(m_pVehModelData);

	m_pVehClassification = m_pVMD->getVehClassification();
	m_NominalVehicle = m_pVMD->getNominalVehicle();
}


CVehicleGenNominal::~CVehicleGenNominal()
{
}

void CVehicleGenNominal::GenerateVehicle(CVehicle_sp pVeh)
{
	CVehicleGenerator::GenerateVehicle(pVeh);

	// determine type of vehicle
	if (NextVehicleIsCar())
		GenerateCar(pVeh);
	else
	{
		// Copy properties over
		*pVeh = *m_NominalVehicle; 

		// Now randomize the vehicle
		randomize(pVeh);
	}
	pVeh->setClass(Classification(1, "Constant"));
}

void CVehicleGenNominal::randomize(CVehicle_sp pVeh)
{
	double val = 0.0;
	KernelParams kAW, kAS;
	m_pVMD->getKernels(kAW, kAS);
	size_t nAxles = pVeh->getNoAxles();

	// Axle Spacings
	if(kAS.Scale > m_MinimumCOV) 
	{		
		vec vAS(nAxles, 0.0);
		for (size_t i = 0; i < nAxles; ++i)	// ignore last-axle spacing
		{
			val = pVeh->getAS(i);
			val = (m_RNG.*m_pKernelGenerator)(val*kAS.Location, val*kAS.Scale);
			vAS.at(i) = val;
			pVeh->setAS(i, val);
		}
		pVeh->setLength(SumVector(vAS));
	}
	
	// Axle Weights
	if(kAW.Scale > m_MinimumCOV) 
	{	
		// Generate and temporarily store new AWs
		vec vAW(nAxles, 0.0);
		for (size_t i = 0; i < nAxles; ++i)
		{
			val = pVeh->getAW(i);
			val = (m_RNG.*m_pKernelGenerator)(val*kAW.Location, val*kAW.Scale);
			vAW.at(i) = val;
			pVeh->setAW(i, val);
		}
		pVeh->setGVW(SumVector(vAW));
	}
}