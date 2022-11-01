#pragma once
#include "VehicleModelData.h"
#include "VehicleTrafficFile.h"

class CVehModelDataConstant :	public CVehicleModelData
{
public:
    CVehModelDataConstant(CConfigDataCore& config, CVehicleClassification_sp pVC, CLaneFlowComposition lfc);
	virtual ~CVehModelDataConstant();

	virtual void ReadDataIn();

    void getCoVs(double& CoV_AS, double& CoV_AW);
    CVehicle_sp getNominalVehicle() {return m_pNominalVehicle;};

private:
	void readConstant();
    void makeNominalVehicle();

    CVehicle_sp m_pNominalVehicle;
    double m_CoV_AS;
    double m_CoV_AW;
};
typedef std::shared_ptr<CVehModelDataConstant> CVehModelDataConstant_sp;