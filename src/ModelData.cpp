#include "ModelData.h"
#include "ConfigData.h"

//extern CConfigData g_ConfigData;

CModelData::CModelData()
{
	m_Path = CConfigData::get().Gen.TRAFFIC_FOLDER;
	// check if the path ends in a \ and if not add it
	if (*m_Path.rbegin() != '\\')
		m_Path += "\\";
}


CModelData::~CModelData()
{

}
