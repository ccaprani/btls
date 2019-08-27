#pragma once
#include "VehicleGenerator.h"

class CVehicleGenConstant :	public CVehicleGenerator
{
public:
	CVehicleGenConstant();
	virtual ~CVehicleGenConstant();

protected:
	virtual void GenerateVehicle(CVehicle* pVeh);
	virtual size_t GenVehClass();
};

