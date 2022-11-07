#pragma once
#include "VehicleModelData.h"
#include "VehicleTrafficFile.h"

class CVehModelDataNominal : public CVehicleModelData
{
public:
    CVehModelDataNominal(CConfigDataCore& config, CVehicleClassification_sp pVC, CLaneFlowComposition lfc);
	virtual ~CVehModelDataNominal();

	virtual void ReadDataIn();

	void getKernels(KernelParams& AW, KernelParams& AS);

    CVehicle_sp getNominalVehicle() {return m_pNominalVehicle;};

private:
	void readNominal();
    void makeNominalVehicle();

    CVehicle_sp m_pNominalVehicle;

    EKernelType m_KernelType;

    KernelParams m_KernelAW;
    KernelParams m_KernelAS;
};
typedef std::shared_ptr<CVehModelDataNominal> CVehModelDataNominal_sp;