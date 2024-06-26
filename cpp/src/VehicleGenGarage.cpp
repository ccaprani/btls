#include "VehicleGenGarage.h"


CVehicleGenGarage::CVehicleGenGarage(CVehModelDataGarage_sp pVMD)
	: CVehicleGenerator(eVM_Garage, pVMD)
{
	m_pVMD = std::dynamic_pointer_cast<CVehModelDataGarage>(m_pVehModelData);

	m_pVehClassification = m_pVMD->getVehClassification();

	m_GarageCount = m_pVMD->getGarageCount();
}


CVehicleGenGarage::~CVehicleGenGarage()
{

}

void CVehicleGenGarage::GenerateVehicle(CVehicle_sp pVeh)
{
	CVehicleGenerator::GenerateVehicle(pVeh);
	
	// determine type of vehicle
	if (NextVehicleIsCar())
		GenerateCar(pVeh);
	else
	{
		// Randomly pick a vehicle from the garage
		double u = m_RNG.GenerateUniform();
		size_t i = static_cast<size_t>(static_cast<double>(m_GarageCount) * u);
		CVehicle_sp pGarageVeh = m_pVMD->getVehicle(i);
		*pVeh = *pGarageVeh; // Copy properties over

		// Now randomize the vehicle
		randomize(pVeh);
	}
	m_pVehClassification->setClassification(pVeh);	// It has been done when load garage, but the class info losses when pickle and unpickle in Python. 
}

void CVehicleGenGarage::randomize(CVehicle_sp pVeh)
{
	double u = 1.0;
	KernelParams kGVW, kAW, kAS;
	m_pVMD->getKernels(kGVW, kAW, kAS);

	// GVW
	double gvw = pVeh->getGVW();
	u = (m_RNG.*m_pKernelGenerator)(kGVW.Location, kGVW.Scale);
	gvw *= u;
	pVeh->setGVW(gvw);
	
	// Axle Weights
	double val = 0.0;
	size_t nAxles = pVeh->getNoAxles();
	// Generate and temporarily store new AWs
	vec vAW(nAxles, 0.0);
	for (size_t i = 0; i < nAxles; ++i)
	{
		u = (m_RNG.*m_pKernelGenerator)(kAW.Location, kAW.Scale);
		val = pVeh->getAW(i);
		val *= u;
		vAW.at(i) = val;
	}
	// scale AWs to get correct GVW
	double sumAW = SumVector(vAW);
	ScaleVector(vAW, gvw/sumAW);	
	// And assign to vehicle
	for (size_t i = 0; i < nAxles; ++i)
		pVeh->setAW(i, vAW.at(i));
	
	// Axle Spacings
	vec vAS(nAxles, 0.0);
	for (size_t i = 0; i < nAxles - 1; ++i)	// ignore last-axle spacing
	{
		u = (m_RNG.*m_pKernelGenerator)(kAS.Location, kAS.Scale);
		val = pVeh->getAS(i);
		val *= u;
		vAS.at(i) = val;
		pVeh->setAS(i, val); // set it
	}
	double length = SumVector(vAS);
	pVeh->setLength(length);
}
