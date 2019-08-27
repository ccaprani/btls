#pragma once
#include "VehicleModelData.h"
#include "VehicleTrafficFile.h"

class CVehModelDataGarage :	public CVehicleModelData
{
public:
	CVehModelDataGarage(CVehicleClassification* pVC, CLaneFlowComposition lfc);
	virtual ~CVehModelDataGarage();

	virtual void ReadDataIn();

	size_t getGarageCount() const { return m_NoVehicles; };
	CVehicle* getVehicle(size_t i);
	void getKernals(Normal& GVW, Normal& AW, Normal& AS);

private:
	void readGarage();
	void readKernels();

	std::vector<CVehicle*> m_vVehicles;
	size_t m_NoVehicles;

	Normal m_KernalGVW;
	Normal m_KernalAW;
	Normal m_KernalAS;
};

