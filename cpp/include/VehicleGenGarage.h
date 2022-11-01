#pragma once
#include "VehicleGenerator.h"
#include "VehModelDataGarage.h"

class CVehicleGenGarage : public CVehicleGenerator
{
public:
	CVehicleGenGarage(CVehModelDataGarage_sp pVMD);
	virtual ~CVehicleGenGarage();

protected:
	virtual void GenerateVehicle(CVehicle_sp pVeh);
	virtual size_t GenVehClass() { return 0; };

private:
	void randomize(CVehicle_sp pVeh);

	CVehModelDataGarage_sp m_pVMD;
	size_t m_GarageCount;
};
typedef std::shared_ptr<CVehicleGenGarage> CVehicleGenGarage_sp;

