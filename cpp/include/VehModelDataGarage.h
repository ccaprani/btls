#pragma once
#include "VehicleModelData.h"
#include "VehicleTrafficFile.h"

class CVehModelDataGarage :	public CVehicleModelData
{
public:
	CVehModelDataGarage(CVehicleClassification_sp pVC, CLaneFlowComposition lfc);
	virtual ~CVehModelDataGarage();

	virtual void ReadDataIn();

	void readGarage(std::string garage_file);
	void assignGarage(std::vector<CVehicle_sp> vVehicles);
	void readKernels(std::string kernel_file);

	size_t getGarageCount() const { return m_NoVehicles; };
	CVehicle_sp getVehicle(size_t i);
	void getKernals(Normal& GVW, Normal& AW, Normal& AS);

private:
	void readGarage();
	void extractGarage(filesystem::path file);
	void readKernels();
	void extractKernels(filesystem::path kernel_file);

	std::vector<CVehicle_sp> m_vVehicles;
	size_t m_NoVehicles;

	Normal m_KernalGVW;
	Normal m_KernalAW;
	Normal m_KernalAS;
};
typedef std::shared_ptr<CVehModelDataGarage> CVehModelDataGarage_sp;
