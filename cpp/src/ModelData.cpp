#include "ModelData.h"
#include "ConfigData.h"

CModelData::CModelData(CConfigDataCore& config) : m_Config(config)
{
	m_Path = m_Config.Gen.TRAFFIC_FOLDER;
}

CModelData::~CModelData()
{

}
