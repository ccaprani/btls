#include "ModelData.h"
#include "ConfigData.h"

//extern CConfigData g_ConfigData;

CModelData::CModelData()
{
	m_Path = CConfigData::get().Gen.TRAFFIC_FOLDER;
}

CModelData::CModelData(CPyConfigData& pyConfig)
{
	m_Path = pyConfig.Gen_TRAFFIC_FOLDER;
}

CModelData::~CModelData()
{

}
