#include "FlowGenerator.h"

CFlowGenerator::CFlowGenerator(CFlowModelData_sp pFMD, EFlowModel fm)
	: m_CurBlock(0), m_FlowModel(fm), m_TotalFlow(0.0), m_TruckFlow(0.0)
	, m_BlockSize(3600), m_BlockCount(24), m_pFlowModelData(pFMD)
{
	m_pPrevVeh = nullptr;
	m_pNextVeh = nullptr;

	m_MinGap = 0.1; // 0.1 s min gap for first vehicle

	if (m_pFlowModelData != nullptr) // some models may not use data
	{
		m_pFlowModelData->getGapBuffers(m_BufferGapSpace, m_BufferGapTime);
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
	while (gap < m_MinGap)
	{
		gap = GenerateGap(); // instanced in derived classes
//		if(gap < minGap)
//			std::cout << "Overlap prevented: " << gap << " s < " << minGap << " s" << endl;
	}

	return gap;
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

	double Vii = m_pPrevVeh->getVelocity();
	double Vi = m_pNextVeh->getVelocity();
	double Lii = m_pPrevVeh->getLength();

	if (Vi <= Vii)
		m_MinGap = (Lii + m_BufferGapSpace) / Vii;	// so no overlap of vehicles
	else
	{
		double deltaV = Vi - Vii;
		double Tno = m_BufferGapTime + (m_BufferGapSpace + Lii) / Vii;	// time of no overlap allowed
		double DistClose = deltaV * Tno;
		double DistStart = DistClose + Lii + m_BufferGapSpace;			// no overlap distance gap

		m_MinGap = DistStart / Vi;	// no overlap time gap
	}
}

//////////// CFlowGenNHM ///////////////

CFlowGenNHM::CFlowGenNHM(CFlowModelDataNHM_sp pFMD) : CFlowGenerator(pFMD, eFM_NHM)
{
	m_pFMD = std::dynamic_pointer_cast<CFlowModelDataNHM>(m_pFlowModelData);

	m_vNHM = m_pFMD->GetNHM();
	m_pFMD->getSpeedParams(m_CurBlock, m_Speed);
}


CFlowGenNHM::~CFlowGenNHM()
{
}

double CFlowGenNHM::GenerateGap()
{
	double headwayType = m_RNG.GenerateUniform();
	double Q = m_TruckFlow;
	double gap = 0.0;

	// vector - vNHM: ie New Headway Model - as per IStructE paper
	// first line gives no of intervals
	// second line gives the u1s params of the quadratic curve
	// third gives the u1.5s params
	// the rest give the flowrate interval and the flowrate and it's params

	int curInterval = 0;	int noIntervals = (int)m_vNHM[0][0];

	int i = 3;
	while (i < 3 + noIntervals && Q > m_vNHM[i][0])
		i++;
	curInterval = i; // not i+1 because it's a zero-based array

	double u1s = quad(m_vNHM[1][1], m_vNHM[1][2], m_vNHM[1][3], 1.0);
	double u1pt5s = quad(m_vNHM[2][1], m_vNHM[2][2], m_vNHM[2][3], 1.5);
	double u4s = quad(m_vNHM[curInterval][1], m_vNHM[curInterval][2], m_vNHM[curInterval][3], 4.0);

	if (headwayType > u4s)
	{
		do{
			gap = genExponential(); // from base class
		} while (gap < 4.0);
	}
	else if (headwayType <= u1s)
	{
		gap = inv_quad(m_vNHM[1][1], m_vNHM[1][2], m_vNHM[1][3], headwayType);
	}
	else if (headwayType > u1s && headwayType <= u1pt5s)
	{
		gap = inv_quad(m_vNHM[2][1], m_vNHM[2][2], m_vNHM[2][3], headwayType);
	}
	else if (headwayType > u1pt5s && headwayType <= u4s)
	{
		gap = inv_quad(m_vNHM[curInterval][1], m_vNHM[curInterval][2], m_vNHM[curInterval][3], headwayType);
	}
	return gap;
}

double CFlowGenNHM::GenerateSpeed()
{
	return m_RNG.GenerateNormal(m_Speed.Mean, m_Speed.StdDev);
}

void CFlowGenNHM::updateProperties()
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