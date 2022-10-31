#include "FlowModelData.h"
#include "ConfigData.h"

//extern CConfigData g_ConfigData;

CFlowModelData::CFlowModelData(EFlowModel fm, CLaneFlowComposition lfc, const bool bCars)
	: m_Model(fm), m_bModelHasCars(bCars)
	// MAGIC NUMBERs - internal gap buffer e.g. tyre diameter, and a min driving gap
	, m_BufferGapSpace(1.0), m_BufferGapTime(0.1)
{
	Creator(lfc);
}

CFlowModelData::CFlowModelData(EFlowModel fm, CLaneFlowComposition lfc, const bool bCars, CConfigDataCore& config)
	: m_Model(fm), m_bModelHasCars(bCars)
	// MAGIC NUMBERs - internal gap buffer e.g. tyre diameter, and a min driving gap
	, m_BufferGapSpace(1.0), m_BufferGapTime(0.1), CModelData(config)
{
	CConfigData::get().Gen.NO_OVERLAP_LENGTH = config.Gen.NO_OVERLAP_LENGTH;
	Creator(lfc);
}

CFlowModelData::~CFlowModelData()
{
}

void CFlowModelData::Creator(CLaneFlowComposition lfc) {
	m_vTotalFlow = lfc.getTotalFlow();
	m_vTruckFlow = lfc.getTruckFlow();
	m_vCarPercent = lfc.getCarPercent();
	m_vSpeed = lfc.getSpeed();
	m_BlockSize = lfc.getBlockSize();
	m_BlockCount = lfc.getBlockCount();
}

void CFlowModelData::getFlow(size_t i, double& totalFlow, double& truckFlow)
{
	totalFlow = m_vTotalFlow.at(i);
	truckFlow = m_vTruckFlow.at(i);
}

void CFlowModelData::getSpeedParams(size_t i, Normal& speed)
{
	speed = m_vSpeed.at(i);
}

void CFlowModelData::getGapLimits(double& bridge, double& space, double& time)
{
	bridge = CConfigData::get().Gen.NO_OVERLAP_LENGTH;
	space = m_BufferGapSpace;
	time = m_BufferGapTime;
}

void CFlowModelData::getBlockInfo(size_t& sz, size_t& n) const
{
	sz = m_BlockSize;
	n = m_BlockCount;
}

//////////// CFlowModelDataNHM //////////////

CFlowModelDataNHM::CFlowModelDataNHM(CLaneFlowComposition lfc)
	: CFlowModelData(eFM_NHM, lfc, CConfigData::get().Traffic.GEN_CAR)
{
	ReadDataIn();
}

CFlowModelDataNHM::CFlowModelDataNHM(CLaneFlowComposition lfc, CConfigDataCore& config)
	: CFlowModelData(eFM_NHM, lfc, config.Traffic.GEN_CAR, config)
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
	filesystem::path file = m_Path / "NHM.csv";
	if (!m_CSV.OpenFile(file.string(), ","))
	{
		std::cout << "**** ERROR: Cannot read " << std::filesystem::weakly_canonical(file) << std::endl;
		return;
	}
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

CFlowModelDataCongested::CFlowModelDataCongested(CLaneFlowComposition lfc)
	: CFlowModelData(eFM_Congested, lfc, CConfigData::get().Traffic.GEN_CAR)
{
	m_GapMean = CConfigData::get().Traffic.CONGESTED_GAP;
	m_GapStd = CConfigData::get().Traffic.CONGESTED_GAP_COEF_VAR;
	m_Speed = CConfigData::get().Traffic.CONGESTED_SPEED;
}

CFlowModelDataCongested::CFlowModelDataCongested(CLaneFlowComposition lfc, CConfigDataCore& config)
	: CFlowModelData(eFM_Congested, lfc, config.Traffic.GEN_CAR, config)
{
	m_GapMean = config.Traffic.CONGESTED_GAP;
	m_GapStd = config.Traffic.CONGESTED_GAP_COEF_VAR;
	m_Speed = config.Traffic.CONGESTED_SPEED;
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

CFlowModelDataPoisson::CFlowModelDataPoisson(CLaneFlowComposition lfc)
	: CFlowModelData(eFM_Poisson, lfc, CConfigData::get().Traffic.GEN_CAR)
{

}

CFlowModelDataPoisson::CFlowModelDataPoisson(CLaneFlowComposition lfc, CConfigDataCore& config)
	: CFlowModelData(eFM_Poisson, lfc, config.Traffic.GEN_CAR, config)
{

}

CFlowModelDataPoisson::~CFlowModelDataPoisson()
{

}

void CFlowModelDataPoisson::ReadDataIn()
{
	// Read in related files for this model
}

//////////// CFlowModelDataConstant //////////////

CFlowModelDataConstant::CFlowModelDataConstant(CLaneFlowComposition lfc)
	: CFlowModelData(eFM_Constant, lfc, CConfigData::get().Traffic.GEN_CAR)
{
	m_ConstSpeed = CConfigData::get().Traffic.CONSTANT_SPEED;
	m_ConstGap = CConfigData::get().Traffic.CONSTANT_GAP;
}

CFlowModelDataConstant::CFlowModelDataConstant(CLaneFlowComposition lfc, CConfigDataCore& config)
	: CFlowModelData(eFM_Constant, lfc, config.Traffic.GEN_CAR, config)
{
	m_ConstSpeed = config.Traffic.CONSTANT_SPEED;
	m_ConstGap = config.Traffic.CONSTANT_GAP;
}

CFlowModelDataConstant::~CFlowModelDataConstant()
{
}

double CFlowModelDataConstant::getConstSpeed() 
{
	return m_ConstSpeed;
}

double CFlowModelDataConstant::getConstGap() 
{
	return m_ConstGap;
}

void CFlowModelDataConstant::ReadDataIn()
{
	// Read in related files for this model
}

