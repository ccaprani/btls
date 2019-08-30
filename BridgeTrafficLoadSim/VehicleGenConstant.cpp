#include "VehicleGenConstant.h"


CVehicleGenConstant::CVehicleGenConstant()	: CVehicleGenerator(eVM_Constant, nullptr)
{
	m_pVehClassification = nullptr;
}


CVehicleGenConstant::~CVehicleGenConstant()
{
}

void CVehicleGenConstant::GenerateVehicle(CVehicle_ptr pVeh)
{
	// assign general properties
	pVeh->setLaneEccentricity(0.0);
	pVeh->setTrns(0.0); // m 0 for generated vehicles
	pVeh->setHead(1001);

	// determine type of vehicle
	if (NextVehicleIsCar())
		GenerateCar(pVeh);
	else
	{
		pVeh->setGVW(460);
		pVeh->setNoAxles(6);
		int i = 0;	pVeh->setAS(i, 3.5); pVeh->setAW(i, 70); pVeh->setAT(i, 2.4);
		i++;		pVeh->setAS(i, 2.0); pVeh->setAW(i, 60); pVeh->setAT(i, 2.4);
		i++;		pVeh->setAS(i, 6.0); pVeh->setAW(i, 60); pVeh->setAT(i, 2.4);
		i++;		pVeh->setAS(i, 1.2); pVeh->setAW(i, 90); pVeh->setAT(i, 2.4);
		i++;		pVeh->setAS(i, 1.2); pVeh->setAW(i, 90); pVeh->setAT(i, 2.4);
		i++;		pVeh->setAS(i, 0.0); pVeh->setAW(i, 90); pVeh->setAT(i, 2.4);
		pVeh->setLength(13.9);
	}
	pVeh->setClass(Classification(1, "Constant"));
}

size_t CVehicleGenConstant::GenVehClass()
{
	// all of the same class
	return 1;
}