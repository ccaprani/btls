#include "VehicleModelData.h"
#include "ConfigData.h"

//extern CConfigData g_ConfigData;

CVehicleModelData::CVehicleModelData(EVehicleModel vm, CVehicleClassification_sp pVC, CLaneFlowComposition lfc, const size_t nClass)
	: m_Model(vm), m_pVehClassification(pVC), m_LaneEccStd(0.0), m_TruckClassCount(nClass)
{
	m_LaneEccStd = CConfigData::get().Gen.LANE_ECCENTRICITY_STD / 100; // cm to m

	m_mComposition = lfc.getComposition();
}

CVehicleModelData::CVehicleModelData(EVehicleModel vm, CVehicleClassification_sp pVC, CLaneFlowComposition lfc, const size_t nClass, CConfigDataCore& config)
	: m_Model(vm), m_pVehClassification(pVC), m_LaneEccStd(0.0), m_TruckClassCount(nClass), CModelData(config)
{
	m_LaneEccStd = config.Gen.LANE_ECCENTRICITY_STD / 100; // cm to m

	m_mComposition = lfc.getComposition();
}

CVehicleModelData::~CVehicleModelData()
{

}

vec CVehicleModelData::getComposition(size_t i) const
{
	return m_mComposition.at(i);
}
