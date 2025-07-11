#include "FlowGenerator.h"

CFlowGenerator::CFlowGenerator(CFlowModelData_sp pFMD, EFlowModel fm)
	: m_pFlowModelData(pFMD), m_FlowModel(fm), m_TotalFlow(0.0), m_TruckFlow(0.0)
	, m_CurBlock(0), m_BlockSize(3600), m_BlockCount(24)
{
	m_pPrevVeh = nullptr;
	m_pNextVeh = nullptr;

	m_MinGap = 0.1; // 0.1 s min gap for first vehicle

	if (m_pFlowModelData != nullptr) // some models may not use data
	{
		m_pFlowModelData->getGapLimits(m_MaxBridgeLength, m_BufferGapSpace, m_BufferGapTime);
		m_pFlowModelData->getBlockInfo(m_BlockSize, m_BlockCount);
	}

	updateProperties(); // for first hour
}


CFlowGenerator::~CFlowGenerator()
{
}

void CFlowGenerator::prepareNextGen(double time, CVehicle_sp pPrevVeh, CVehicle_sp pNextVeh)
{
	m_pPrevVeh = pPrevVeh;
	m_pNextVeh = pNextVeh;

	updateBlock(time);	
}

double CFlowGenerator::Generate()
{
	// Assign speed based on flow model, then check min gap
	m_pNextVeh->setVelocity(GenerateSpeed());
	setMinGap();

	double gap = 0.0;
	size_t gen_count = 0;
	while (gap < m_MinGap)
	{
		gap = GenerateGap(); // instanced in derived classes. Notice! This could be infinate loop for constant headway gen if vehicle length is too long
		gen_count++;
		if (gen_count > 10000)
		{
			std::cout << "***Warning: overlap may not be prevented due to long vehicle, use MinGap (s) " << m_MinGap << std::endl;
			gap = m_MinGap;
			break;
		}
	}

	return gap;
}

void CFlowGenerator::setMaxBridgeLength(double length) 
{
	m_MaxBridgeLength = length;
}

void CFlowGenerator::updateBlock(double time)
{
	// This is now made more generic: 
	// a block is usually an hour
	// blockCount is usually 24

	size_t iBlock = (int)(time / static_cast<double>(m_BlockSize) );
	iBlock = iBlock % m_BlockCount;	// reduce to one period (i.e. day)

	if (iBlock != m_CurBlock) // if it's not the same hour/block
	{
		m_CurBlock = iBlock;
		m_CurBlock = m_CurBlock % m_BlockCount;	// make sure under e.g. 24
		updateProperties();	
	}
}

void CFlowGenerator::updateProperties()
{
	// Update flow properties
	if (m_pFlowModelData != nullptr)
		m_pFlowModelData->getFlow(m_CurBlock, m_TotalFlow, m_TruckFlow);

	updateExponential();
}

void CFlowGenerator::updateExponential()
{
	// update anything relevant to gap generation as hours changes

	double mean = 3600.0 / m_TotalFlow;

	m_RNG.setScale(mean);
	m_RNG.setLocation(0.0);	// for exponential deviates
}

double CFlowGenerator::genExponential()
{
	return m_RNG.GenerateExponential();
}

void CFlowGenerator::setMinGap()
{
	m_MinGap = 0.1; // default 0.1 s min gap

	if (m_pPrevVeh == nullptr)
		return;

	double Va = m_pPrevVeh->getVelocity();
	double Vb = m_pNextVeh->getVelocity();
	double La = m_pPrevVeh->getLength();

	// required time gap at the end of the bridge
	double Tg = (La + m_BufferGapSpace) / Vb + m_BufferGapTime;
	if (Vb <= Va)
		m_MinGap = Tg;
	else
	{
		m_MinGap = (Tg * Vb + m_MaxBridgeLength) / Va - m_MaxBridgeLength / Vb;
		if (m_MinGap < m_BufferGapTime)
		{
			m_MinGap = m_BufferGapTime;
			std::cout << std::endl << "*** Warning: MinGap - unusual vehicle velocities or buffers" << std::endl;
		}
	}

	//if (Vi <= Vii)
	//	m_MinGap = (Lii + m_BufferGapSpace) / Vii;	// so no overlap of vehicles
	//else
	//{
	//	double deltaV = Vi - Vii;
	//	double Tno = m_BufferGapTime + (m_BufferGapSpace + Lii) / Vii;	// time of no overlap allowed
	//	double DistClose = deltaV * Tno;
	//	double DistStart = DistClose + Lii + m_BufferGapSpace;			// no overlap distance gap

	//	m_MinGap = DistStart / Vi;	// no overlap time gap
	//}
}

//////////// CFlowGenHeDS ///////////////

CFlowGenHeDS::CFlowGenHeDS(CFlowModelDataHeDS_sp pFMD) : CFlowGenerator(pFMD, eFM_HeDS)
{
	m_pFMD = std::dynamic_pointer_cast<CFlowModelDataHeDS>(m_pFlowModelData);

	m_vHeDS = m_pFMD->GetHeDS();
	m_pFMD->getSpeedParams(m_CurBlock, m_Speed);
}


CFlowGenHeDS::~CFlowGenHeDS()
{
}

double CFlowGenHeDS::GenerateGap()
{
	double headwayType = m_RNG.GenerateUniform();
	double Q = m_TruckFlow;
	double gap = 0.0;

	// vector - vHeDS: ie New Headway Model - as per IStructE paper
	// first line gives no of intervals
	// second line gives the u1s params of the quadratic curve
	// third gives the u1.5s params
	// the rest give the flowrate interval and the flowrate and it's params

	int curInterval = 0;	int noIntervals = (int)m_vHeDS[0][0];

	int i = 3;
	while (i < 3 + noIntervals && Q > m_vHeDS[i][0])
		i++;
	curInterval = i; // not i+1 because it's a zero-based array

	double u1s = quad(m_vHeDS[1][1], m_vHeDS[1][2], m_vHeDS[1][3], 1.0);
	double u1pt5s = quad(m_vHeDS[2][1], m_vHeDS[2][2], m_vHeDS[2][3], 1.5);
	double u4s = quad(m_vHeDS[curInterval][1], m_vHeDS[curInterval][2], m_vHeDS[curInterval][3], 4.0);

	if (headwayType > u4s)
	{
		do{
			gap = genExponential(); // from base class
		} while (gap < 4.0);
	}
	else if (headwayType <= u1s)
	{
		gap = inv_quad(m_vHeDS[1][1], m_vHeDS[1][2], m_vHeDS[1][3], headwayType);
	}
	else if (headwayType > u1s && headwayType <= u1pt5s)
	{
		gap = inv_quad(m_vHeDS[2][1], m_vHeDS[2][2], m_vHeDS[2][3], headwayType);
	}
	else if (headwayType > u1pt5s && headwayType <= u4s)
	{
		gap = inv_quad(m_vHeDS[curInterval][1], m_vHeDS[curInterval][2], m_vHeDS[curInterval][3], headwayType);
	}
	return gap;
}

double CFlowGenHeDS::GenerateSpeed()
{
	return m_RNG.GenerateNormal(m_Speed.Mean, m_Speed.StdDev);
}

void CFlowGenHeDS::updateProperties()
{
	// update anything relevant to gap generation as hours changes
	CFlowGenerator::updateProperties();

	m_pFMD->getSpeedParams(m_CurBlock, m_Speed);
}

//////////// CFlowGenCongested ///////////////

CFlowGenCongested::CFlowGenCongested(CFlowModelDataCongested_sp pFMD) : CFlowGenerator(pFMD, eFM_Congested)
{
	m_pFMD = std::dynamic_pointer_cast<CFlowModelDataCongested>(m_pFlowModelData);

	m_Speed = m_pFMD->getSpeed();
	m_pFMD->getGapParams(m_Gap.Mean, m_Gap.StdDev);
}

CFlowGenCongested::~CFlowGenCongested()
{
}

double CFlowGenCongested::GenerateGap()
{
	// randomize congestion: note this is time gap from front to front of truck
	double timeForTruckToPass = 0.0;

	if (m_pPrevVeh != nullptr)
	{
		double Vii = m_pPrevVeh->getVelocity();
		double Lii = m_pPrevVeh->getLength();
		timeForTruckToPass = Lii / Vii; // ignores the m_BufferGap on purpose
	}
	else if (m_pNextVeh != nullptr)
		// there is no truck in front so set to own time to pass
		timeForTruckToPass = m_pNextVeh->getLength() / m_pNextVeh->getVelocity();

	// generate congested time gap and add prev vehicle passage time
	double congGap = m_RNG.GenerateNormal(m_Gap.Mean, m_Gap.StdDev);
	double gap = timeForTruckToPass + congGap;
	return gap;
}

double CFlowGenCongested::GenerateSpeed()
{
	return m_Speed;
}

void CFlowGenCongested::updateProperties()
{
	CFlowGenerator::updateProperties();
	// update anything relevant to gap generation as hours changes

}


///////////// CFlowGenPoisson ///////////////

CFlowGenPoisson::CFlowGenPoisson(CFlowModelDataPoisson_sp pFDM) : CFlowGenerator(pFDM, eFM_Poisson)
{
	m_pFMD = std::dynamic_pointer_cast<CFlowModelDataPoisson>(m_pFlowModelData);

	m_pFMD->getSpeedParams(m_CurBlock, m_Speed);
}

CFlowGenPoisson::~CFlowGenPoisson()
{
}

double CFlowGenPoisson::GenerateGap()
{
	return genExponential(); // from base class
}

double CFlowGenPoisson::GenerateSpeed()
{
	return m_RNG.GenerateNormal(m_Speed.Mean, m_Speed.StdDev);
}

void CFlowGenPoisson::updateProperties()
{
	CFlowGenerator::updateProperties();
	// update anything relevant to gap generation as hours changes

	m_pFMD->getSpeedParams(m_CurBlock, m_Speed);
}


///////////// CFlowGenConstant ///////////////

CFlowGenConstant::CFlowGenConstant(CFlowModelDataConstant_sp pFDM) : CFlowGenerator(pFDM, eFM_Constant) 
{
	m_pFMD = std::dynamic_pointer_cast<CFlowModelDataConstant>(m_pFlowModelData);
}

CFlowGenConstant::~CFlowGenConstant() 
{
}

double CFlowGenConstant::GenerateGap() 
{
	return m_pFMD->getConstGap();
}

double CFlowGenConstant::GenerateSpeed() 
{
	return m_pFMD->getConstSpeed();
}
