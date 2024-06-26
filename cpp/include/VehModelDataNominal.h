#pragma once
#include "VehicleModelData.h"
#include "VehicleTrafficFile.h"

class CVehModelDataNominal : public CVehicleModelData
{
public:
    CVehModelDataNominal(CConfigDataCore& config, CVehicleClassification_sp pVC, CLaneFlowComposition lfc);
    CVehModelDataNominal(CConfigDataCore& config, CVehicleClassification_sp pVC, CLaneFlowComposition lfc, CVehicle_sp pVeh, std::vector<double> vCOV);
	virtual ~CVehModelDataNominal();

	virtual void ReadDataIn();

	void getKernels(KernelParams& AW, KernelParams& AS);

    CVehicle_sp getNominalVehicle() {return m_pNominalVehicle;};

private:
	void readNominalVehicle();
    void assignNominalVehicle(CVehicle_sp pVeh);
    void makeNominalVehicle();

    CVehicle_sp m_pNominalVehicle;

    KernelParams m_KernelAW;
    KernelParams m_KernelAS;
};
typedef std::shared_ptr<CVehModelDataNominal> CVehModelDataNominal_sp;