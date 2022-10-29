#pragma once
#include "VehicleModelData.h"
#include "VehicleTrafficFile.h"

class CVehModelDataGarage :	public CVehicleModelData
{
public:
	CVehModelDataGarage(CVehicleClassification_sp pVC, CLaneFlowComposition lfc);
	CVehModelDataGarage(CVehicleClassification_sp pVC, CLaneFlowComposition lfc, CConfigDataCore& config);
	virtual ~CVehModelDataGarage();

	virtual void ReadDataIn();

	void assignGarage(std::vector<CVehicle_sp> vVehicles);

	size_t getGarageCount() const { return m_NoVehicles; };
	CVehicle_sp getVehicle(size_t i);
	void getKernals(Normal& GVW, Normal& AW, Normal& AS);

private:
	void readGarage();
	void readKernels();

	std::vector<CVehicle_sp> m_vVehicles;
	size_t m_NoVehicles;

	Normal m_KernalGVW;
	Normal m_KernalAW;
	Normal m_KernalAS;
};
typedef std::shared_ptr<CVehModelDataGarage> CVehModelDataGarage_sp;
