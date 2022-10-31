#include "ModelData.h"
#include "ConfigData.h"

//extern CConfigData g_ConfigData;

CModelData::CModelData()
{
	m_Path = CConfigData::get().Gen.TRAFFIC_FOLDER;
}

CModelData::CModelData(CConfigDataCore& config)
{
	m_Path = config.Gen.TRAFFIC_FOLDER;
}

CModelData::~CModelData()
{

}
