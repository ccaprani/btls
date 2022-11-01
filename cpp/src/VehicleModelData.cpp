#include "VehicleModelData.h"
#include "ConfigData.h"

CVehicleModelData::CVehicleModelData(CConfigDataCore& config, EVehicleModel vm, CVehicleClassification_sp pVC, CLaneFlowComposition lfc, const size_t nClass)
	: CModelData(config), m_pVehClassification(pVC), m_LaneEccStd(0.0), m_TruckClassCount(nClass)
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
