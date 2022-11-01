#include "VehicleModelData.h"
#include "ConfigData.h"

//extern CConfigData g_ConfigData;

CVehicleModelData::CVehicleModelData(CConfigDataCore& config, EVehicleModel vm, CVehicleClassification_sp pVC, CLaneFlowComposition lfc, const size_t nClass)
	: m_Config(config), m_Model(vm), m_pVehClassification(pVC), m_LaneEccStd(0.0), m_TruckClassCount(nClass)
{
	m_LaneEccStd = m_Config.Gen.LANE_ECCENTRICITY_STD / 100; // cm to m

	m_mComposition = lfc.getComposition();
}

CVehicleModelData::~CVehicleModelData()
{

}

vec CVehicleModelData::getComposition(size_t i) const
{
	return m_mComposition.at(i);
}
