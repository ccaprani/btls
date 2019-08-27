#pragma once
#include "VehicleGenerator.h"
#include "VehModelDataGarage.h"

class CVehicleGenGarage : public CVehicleGenerator
{
public:
	CVehicleGenGarage(CVehModelDataGarage* pVMD);
	virtual ~CVehicleGenGarage();

protected:
	virtual void GenerateVehicle(CVehicle* pVeh);
	virtual size_t GenVehClass() { return 0; };

private:
	void randomize(CVehicle* pVeh);

	CVehModelDataGarage* m_pVMD;
	size_t m_GarageCount;
};

