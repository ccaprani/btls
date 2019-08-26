#include "VehicleModelData.h"
#include "ConfigData.h"

extern CConfigData g_ConfigData;

CVehicleModelData::CVehicleModelData(CVehicleClassification* pVC, CLaneFlow lf, EVehicleModel vm)
	: m_pVehClassification(pVC), m_LaneFlow(lf), m_Model(vm), m_LaneEccStd(0.0)
{
	m_LaneEccStd = g_ConfigData.Gen.LANE_ECCENTRICITY_STD / 100; // cm to m
}


CVehicleModelData::~CVehicleModelData()
{

}
