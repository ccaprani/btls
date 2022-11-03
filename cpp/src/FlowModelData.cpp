#include "FlowModelData.h"


CFlowModelData::CFlowModelData(CConfigDataCore& config, EFlowModel fm, CLaneFlowComposition lfc, const bool bCars)
	: CModelData(config), m_Model(fm), m_bModelHasCars(bCars)
	// MAGIC NUMBERs - internal gap buffer e.g. tyre diameter, and a min driving gap
	, m_BufferGapSpace(1.0), m_BufferGapTime(0.1)
{	
	m_vTotalFlow = lfc.getTotalFlow();
	m_vTruckFlow = lfc.getTruckFlow();
	m_vCarPercent = lfc.getCarPercent();
	m_vSpeed = lfc.getSpeed();
	m_BlockSize = lfc.getBlockSize();
	m_BlockCount = lfc.getBlockCount();
}

CFlowModelData::~CFlowModelData()
{

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
	bridge = m_Config.Gen.NO_OVERLAP_LENGTH;
	space = m_BufferGapSpace;
	time = m_BufferGapTime;
}

void CFlowModelData::getBlockInfo(size_t& sz, size_t& n) const
{
	sz = m_BlockSize;
	n = m_BlockCount;
}

//////////// CFlowModelDataNHM //////////////

CFlowModelDataNHM::CFlowModelDataNHM(CConfigDataCore& config, CLaneFlowComposition lfc)
	: CFlowModelData(config, eFM_NHM, lfc, false) // Model does not have cars
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
	std::filesystem::path file = m_Path / "NHM.csv";
	if (!m_CSV.OpenFile(file.string(), ","))
	{
		std::cout << "**** ERROR: Cannot read " << std::filesystem::weakly_canonical(file) << std::endl;
		return;
	}
	int noRows = 0;
	std::string line;

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

CFlowModelDataCongested::CFlowModelDataCongested(CConfigDataCore& config, CLaneFlowComposition lfc)
	: CFlowModelData(config, eFM_Congested, lfc, true) // Model has cars
{
	m_GapMean = m_Config.Traffic.CONGESTED_GAP;
	m_GapStd = m_Config.Traffic.CONGESTED_GAP_COEF_VAR;
	m_Speed = m_Config.Traffic.CONGESTED_SPEED;
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

CFlowModelDataPoisson::CFlowModelDataPoisson(CConfigDataCore& config, CLaneFlowComposition lfc)
	: CFlowModelData(config, eFM_Poisson, lfc, true) // Model has cars
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

CFlowModelDataConstant::CFlowModelDataConstant(CConfigDataCore& config, CLaneFlowComposition lfc)
	: CFlowModelData(config, eFM_Constant, lfc, true) // Model has cars
{
	m_ConstSpeed = m_Config.Traffic.CONSTANT_SPEED;
	m_ConstGap = m_Config.Traffic.CONSTANT_GAP;
}

CFlowModelDataConstant::~CFlowModelDataConstant()
{

}

void CFlowModelDataConstant::ReadDataIn()
{
	// Read in related files for this model
}

double CFlowModelDataConstant::getConstSpeed() 
{
	return m_ConstSpeed;
}

double CFlowModelDataConstant::getConstGap() 
{
	return m_ConstGap;
}

