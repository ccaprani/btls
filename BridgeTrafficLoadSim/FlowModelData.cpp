#include "FlowModelData.h"
#include "ConfigData.h"

extern CConfigData g_ConfigData;

CFlowModelData::CFlowModelData(CLaneFlow lf, EFlowModel fm) 
	: m_LaneFlow(lf), m_Model(fm)
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

CFlowModelDataNHM::CFlowModelDataNHM(CLaneFlow lf) 
	: CFlowModelData(lf, eNHM)
{
	ReadDataIn();
}

CFlowModelDataNHM::~CFlowModelDataNHM()
{

}

matrix CFlowModelDataNHM::GetNHM()
{
	return m_vNHM;
}

void CFlowModelDataNHM::ReadDataIn()
{
	ReadFile_NHM();
}

void CFlowModelDataNHM::ReadFile_NHM()
{
	string file = m_Path + "NHM.csv";
	m_CSV.OpenFile(file, ",");

	int noRows = 0;
	string line;

	// get no of rows
	m_CSV.getline(line);
	noRows = m_CSV.stringToInt(m_CSV.getfield(0)) + 3;

	std::vector<double> temp2(noRows, 0.0);
	m_vNHM.assign(noRows, temp2);

	m_vNHM[0][0] = noRows - 3; // ie the no of flow intervals

	for (int i = 1; i < noRows; i++) // first row already done
	{
		m_CSV.getline(line);
		m_vNHM[i] = m_CSV.GetVectorFromCurrentLine();
	};
	m_CSV.CloseFile();
}

//////////// CFlowModelDataCongested //////////////

CFlowModelDataCongested::CFlowModelDataCongested(CLaneFlow lf)
	: CFlowModelData(lf, eCongested)
{
	m_GapMean = g_ConfigData.Traffic.CONGESTED_GAP;
	m_GapStd = g_ConfigData.Traffic.CONGESTED_GAP_COEF_VAR;
	m_Speed = g_ConfigData.Traffic.CONGESTED_SPEED;
}

CFlowModelDataCongested::~CFlowModelDataCongested()
{

}

void CFlowModelDataCongested::ReadDataIn()
{
	// Read in related files for this model
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

void CFlowModelDataPoisson::ReadDataIn()
{
	// Read in related files for this model
}

