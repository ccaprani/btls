#pragma once
#include "VehicleGenerator.h"
#include "VehModelDataGarage.h"

class CVehicleGenGarage : public CVehicleGenerator
{
public:
	CVehicleGenGarage(CVehModelDataGarage_ptr pVMD);
	virtual ~CVehicleGenGarage();

protected:
	virtual void GenerateVehicle(CVehicle_ptr pVeh);
	virtual size_t GenVehClass() { return 0; };

private:
	void randomize(CVehicle_ptr pVeh);

	CVehModelDataGarage_ptr m_pVMD;
	size_t m_GarageCount;
};
typedef std::shared_ptr<CVehicleGenGarage> CVehicleGenGarage_ptr;

