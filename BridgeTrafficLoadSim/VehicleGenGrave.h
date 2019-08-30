#pragma once
#include "VehicleGenerator.h"
#include "VehModelDataGrave.h"

// Implements model of Sam Grave for French traffic linked to Eurocode development
class CVehicleGenGrave : public CVehicleGenerator
{
public:
	CVehicleGenGrave(CVehModelDataGrave_ptr pVMD);
	~CVehicleGenGrave();

protected:
	virtual void GenerateVehicle(CVehicle_ptr pVeh);
	virtual size_t GenVehClass();

private:
	void GenerateTruck23(CVehicle_ptr pVeh, int nAxles);
	void GenerateTruck45(CVehicle_ptr pVeh, int nAxles);
	void GenerateCommonProps(CVehicle_ptr pVeh, int nAxles);

	CVehModelDataGrave_ptr m_pVMD;
};

