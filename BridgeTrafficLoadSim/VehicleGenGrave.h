#pragma once
#include "VehicleGenerator.h"
#include "VehModelDataGrave.h"

// Implements model of Sam Grave for French traffic linked to Eurocode development
class CVehicleGenGrave : public CVehicleGenerator
{
public:
	CVehicleGenGrave(CVehModelDataGrave* pVMD);
	~CVehicleGenGrave();

protected:
	virtual void GenerateVehicle(CVehicle* pVeh);
	virtual size_t GenVehClass();

private:
	void GenerateTruck23(CVehicle* pVeh, int nAxles);
	void GenerateTruck45(CVehicle* pVeh, int nAxles);
	void GenerateCommonProps(CVehicle* pVeh, int nAxles);

	CVehModelDataGrave* m_pVMD;
};

