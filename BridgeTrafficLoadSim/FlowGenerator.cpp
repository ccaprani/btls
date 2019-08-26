#include "FlowGenerator.h"

CFlowGenerator::CFlowGenerator(CFlowModelData* pFDM, EFlowModel fm) 
	: m_CurHour(0), m_FlowModel(fm)
{
	m_pFlowModelData = pFDM;

	m_pPrevVeh = NULL;
	m_pNextVeh = NULL;

	m_MinGap = 0.1; // 0.1 s min gap for first vehicle

	m_pFlowModelData->getGapBuffers(m_BufferGapSpace, m_BufferGapTime);

	updateProperties(); // for first hour
}


CFlowGenerator::~CFlowGenerator()
{
}

void CFlowGenerator::prepareNextGen(double time, CVehicle* pPrevVeh, CVehicle* pNextVeh)
{
	m_pPrevVeh = pPrevVeh;
	m_pNextVeh = pNextVeh;

	updateHour(time);

	setMinGap();
}

double CFlowGenerator::Generate()
{
	double gap = 0.0;
	while (gap < m_MinGap)
	{
		gap = GenerateGap(); // instanced in derived classes
//		if(gap < minGap)
//			std::cout << "Overlap prevented: " << gap << " s < " << minGap << " s" << endl;
	}

	return gap;
}

void CFlowGenerator::updateHour(double time)
{
	int nHour = (int)(time/3600.0);
	nHour = nHour % 24;	// reduce to one day

	if (nHour - m_CurHour != 0) // if it's not the same hour
	{
		m_CurHour++;
		m_CurHour = m_CurHour % 24;	// make sure under 24
		// Give derived classes chance to update
		//updateFlowProperties();
		updateProperties();	
	}
}

void CFlowGenerator::updateProperties()
{
	// Update flow properties
	m_vFlowRate = m_pFlowModelData->getFlow(m_CurHour);
	m_CarPerc = m_pFlowModelData->getCP_cars(m_CurHour);

	updateExponential();
}

void CFlowGenerator::updateExponential()
{
	// update anything relevant to gap generation as hours changes

	// We must amplify the truck flows because there are cars
	double truckPercent = 1.0 - m_CarPerc;
	double totalFlow = m_vFlowRate / truckPercent;
	double mean = 3600.0 / totalFlow;

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

	if (m_pPrevVeh == NULL)
		return;

	double Vii = m_pPrevVeh->getVelocity();
	double Vi = m_pNextVeh->getVelocity();
	double Lii = m_pPrevVeh->getLength();

	if (Vi <= Vii)
		m_MinGap = (Lii + m_BufferGapSpace) / Vii;	// so no overlap of vehicles
	else
	{
		double deltaV = Vi - Vii;
		double Tno = (m_BufferGapTime + m_BufferGapSpace + Lii) / Vii;	// time of no overlap allowed
		double DistClose = deltaV * Tno;
		double DistStart = DistClose + Lii + m_BufferGapSpace;			// no overlap distance gap

		m_MinGap = DistStart / Vi;	// no overlap time gap
	}
}

//////////// CFlowGenNHM ///////////////

CFlowGenNHM::CFlowGenNHM(CFlowModelDataNHM* pFDM) : CFlowGenerator(pFDM, eNHM)
{
	m_pFDM = dynamic_cast<CFlowModelDataNHM*>(m_pFlowModelData);

	m_vNHM = m_pFDM->GetNHM();
	m_pFDM->getSpeedParams(m_CurHour, m_Speed.Mean, m_Speed.StdDev);
}


CFlowGenNHM::~CFlowGenNHM()
{
}

double CFlowGenNHM::GenerateGap()
{
	double headwayType = m_RNG.GenerateUniform();
	double Q = m_vFlowRate; //[m_CurHour];
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

	m_pFDM->getSpeedParams(m_CurHour, m_Speed.Mean, m_Speed.StdDev);
}

//////////// CFlowGenCongested ///////////////

CFlowGenCongested::CFlowGenCongested(CFlowModelDataCongested* pFDM) : CFlowGenerator(pFDM, eCongested)
{
	m_pFDM = dynamic_cast<CFlowModelDataCongested*>(m_pFlowModelData);

	m_pFDM->getSpeedParams(m_CurHour, m_Speed.Mean, m_Speed.StdDev);
	m_pFDM->getGapParams(m_Gap.Mean, m_Gap.StdDev);
}

CFlowGenCongested::~CFlowGenCongested()
{
}

double CFlowGenCongested::GenerateGap()
{
	// randomize congestion: note this is time gap from front to front of truck
	double timeForTruckToPass = 0.0;

	if (m_pPrevVeh != NULL)
	{
		double Vii = m_pPrevVeh->getVelocity();
		double Lii = m_pPrevVeh->getLength();
		timeForTruckToPass = Lii / Vii; // ignores the m_BufferGap on purpose
	}
	else if (m_pNextVeh != NULL)
		// there is no truck in front so set to own time to pass
		timeForTruckToPass = m_pNextVeh->getLength() / m_pNextVeh->getVelocity();

	// generate congested time gap and add prev vehicle passage time
	double congGap = m_RNG.GenerateNormal(m_Gap.Mean, m_Gap.StdDev);
	double gap = timeForTruckToPass + congGap;
	return gap;
}

double CFlowGenCongested::GenerateSpeed()
{
	return m_RNG.GenerateNormal(m_Speed.Mean, m_Speed.StdDev);
}

void CFlowGenCongested::updateProperties()
{
	CFlowGenerator::updateProperties();
	// update anything relevant to gap generation as hours changes

	m_pFDM->getSpeedParams(m_CurHour, m_Speed.Mean, m_Speed.StdDev);
}


///////////// CFlowGenPoisson ///////////////

CFlowGenPoisson::CFlowGenPoisson(CFlowModelDataPoisson* pFDM) : CFlowGenerator(pFDM, ePoisson)
{
	m_pFDM = dynamic_cast<CFlowModelDataPoisson*>(m_pFlowModelData);

	m_pFDM->getSpeedParams(m_CurHour, m_Speed.Mean, m_Speed.StdDev);
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

	m_pFDM->getSpeedParams(m_CurHour, m_Speed.Mean, m_Speed.StdDev);
}