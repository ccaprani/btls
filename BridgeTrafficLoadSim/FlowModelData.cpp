#include "FlowModelData.h"


CFlowModelData::CFlowModelData()
{
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

double CFlowModelData::getSpeedMean(int iHour)
{
	return m_LaneFlow.getSpeedMean((int)iHour);
}

double CFlowModelData::getSpeedStd(int iHour)
{
	return m_LaneFlow.getSpeedStDev((int)iHour);
}

//////////// CFlowModelDataNHM //////////////

CFlowModelDataNHM::CFlowModelDataNHM()
{

}

CFlowModelDataNHM::~CFlowModelDataNHM()
{

}

void CFlowModelDataNHM::setNHM(matrix NHM)
{
	m_vNHM = NHM;
}

matrix CFlowModelDataNHM::GetNHM()
{
	return m_vNHM;
}

//////////// CFlowModelDataCongested //////////////

CFlowModelDataCongested::CFlowModelDataCongested(double mean, double std) 
	: m_GapMean(mean), m_GapStd(std)
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

CFlowModelDataPoisson::CFlowModelDataPoisson()
{

}

CFlowModelDataPoisson::~CFlowModelDataPoisson()
{

}

