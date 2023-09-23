#pragma once
#include "VehicleModelData.h"
#include "VehicleTrafficFile.h"

class CVehModelDataGarage :	public CVehicleModelData
{
public:
	CVehModelDataGarage(CConfigDataCore& config, CVehicleClassification_sp pVC, CLaneFlowComposition lfc);
	CVehModelDataGarage(CConfigDataCore& config, CVehicleClassification_sp pVC, CLaneFlowComposition lfc, std::vector<CVehicle_sp> vVehicles, std::vector<std::vector<double>> vKernelParams);
	virtual ~CVehModelDataGarage();

	virtual void ReadDataIn();
	
	size_t getGarageCount() const { return m_NoVehicles; };
	CVehicle_sp getVehicle(size_t i);

	void getKernels(KernelParams& GVW, KernelParams& AW, KernelParams& AS);

private:
	void readGarage();
	void readKernels();

	void assignGarage(std::vector<CVehicle_sp> vVehicles);
	void assignKernels(std::vector<std::vector<double>> vKernelParams);

	std::vector<CVehicle_sp> m_vVehicles;
	size_t m_NoVehicles;
	
	KernelParams m_KernelGVW;
	KernelParams m_KernelAW;
	KernelParams m_KernelAS;
};
typedef std::shared_ptr<CVehModelDataGarage> CVehModelDataGarage_sp;
