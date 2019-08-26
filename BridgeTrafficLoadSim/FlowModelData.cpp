#include "FlowModelData.h"


CFlowModelData::CFlowModelData(CLaneFlow lf, EFlowModel fm) 
	: m_LaneFlow(lf), m_FlowModel(fm)
{
	m_BufferGapSpace = 1.0; // MAGIC NUMBER - internal gap buffer (e.g. tyre diameter)
	m_BufferGapTime = 0.1;
}


CFlowModelData::~CFlowModelData()
{
}

double CFlowModelData::getFlow(size_t iHour)
{
	return m_LaneFlow.getFlow((int)iHour);
}

double CFlowModelData::getCP_cars(size_t iHour)
{
	return m_LaneFlow.getCP_cars((int)iHour);
}

void CFlowModelData::getSpeedParams(size_t iHour, double& mean, double& std)
{
	mean = m_LaneFlow.getSpeedMean((int)iHour);
	std = m_LaneFlow.getSpeedStDev((int)iHour);
}

void CFlowModelData::getGapBuffers(double& space, double& time)
{
	space = m_BufferGapSpace;
	time = m_BufferGapTime;
}

//////////// CFlowModelDataNHM //////////////

CFlowModelDataNHM::CFlowModelDataNHM(CLaneFlow lf, matrix vNHM) 
	: CFlowModelData(lf, eNHM), m_vNHM(vNHM)
{

}

CFlowModelDataNHM::~CFlowModelDataNHM()
{

}

matrix CFlowModelDataNHM::GetNHM()
{
	return m_vNHM;
}

//////////// CFlowModelDataCongested //////////////

CFlowModelDataCongested::CFlowModelDataCongested(CLaneFlow lf, double mean, double std)
	: CFlowModelData(lf, eCongested), m_GapMean(mean), m_GapStd(std)
{

}

CFlowModelDataCongested::~CFlowModelDataCongested()
{

}

void CFlowModelDataCongested::getGapParams(double& mean, double& std)
{
	mean = m_GapMean;
	std = m_GapStd;
}

//////////// CFlowModelDataPoisson //////////////

CFlowModelDataPoisson::CFlowModelDataPoisson(CLaneFlow lf) 
	: CFlowModelData(lf, ePoisson)
{

}

CFlowModelDataPoisson::~CFlowModelDataPoisson()
{

}

