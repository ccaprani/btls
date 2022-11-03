#include "VehicleModelData.h"
#include "ConfigData.h"

CVehicleModelData::CVehicleModelData(CConfigDataCore& config, EVehicleModel vm, CVehicleClassification_sp pVC, CLaneFlowComposition lfc, const size_t nClass)
	: CModelData(config)
	, m_Model(vm)
	, m_pVehClassification(pVC)
	, m_TruckClassCount(nClass)
	, m_LaneEccStd(0.0)
	, m_KernelType(eKT_Normal)
{
	m_LaneEccStd = m_Config.Gen.LANE_ECCENTRICITY_STD / 100; // cm to m
	m_KernelType = static_cast<EKernelType>(m_Config.Gen.KERNEL_TYPE);

	m_mComposition = lfc.getComposition();
}

CVehicleModelData::~CVehicleModelData()
{

}

vec CVehicleModelData::getComposition(size_t i) const
{
	return m_mComposition.at(i);
}
