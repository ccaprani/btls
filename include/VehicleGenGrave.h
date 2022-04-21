#pragma once
#include "VehicleGenerator.h"
#include "VehModelDataGrave.h"

// Implements model of Sam Grave for French traffic linked to Eurocode development
class CVehicleGenGrave : public CVehicleGenerator
{
public:
	CVehicleGenGrave(CVehModelDataGrave_sp pVMD);
	~CVehicleGenGrave();

protected:
	virtual void GenerateVehicle(CVehicle_sp pVeh);
	virtual size_t GenVehClass();

private:
	void GenerateTruck23(CVehicle_sp pVeh, size_t nAxles);
	void GenerateTruck45(CVehicle_sp pVeh, size_t nAxles);
	void GenerateCommonProps(CVehicle_sp pVeh, size_t nAxles);

	CVehModelDataGrave_sp m_pVMD;
};

