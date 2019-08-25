#include "FlowGenerator.h"

CFlowGenerator::CFlowGenerator(EFlowModel fm) : m_CurHour(0)
{

}


CFlowGenerator::~CFlowGenerator()
{
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
		updateFlowProperties();
		updateProperties();	
	}
}

void CFlowGenerator::updateFlowProperties()
{
	m_vFlowRate = m_pFlowModelData->getFlow(m_CurHour);
	m_CarPerc = m_pFlowModelData->getCP_cars(m_CurHour);
}

//////////// CFlowGenNHM ///////////////

CFlowGenNHM::CFlowGenNHM(CFlowModelDataNHM* pFDM) : CFlowGenerator(eNHM)
{
	m_pFDM = pFDM;
}


CFlowGenNHM::~CFlowGenNHM()
{
}

double CFlowGenNHM::Generate()
{
	return 0.0;
}

double CFlowGenNHM::GenerateSpeed()
{
	double 	speed = m_RNG.GenerateNormal(m_pFDM->getSpeedMean(m_CurHour),
										 m_pFDM->getSpeedStd(m_CurHour));
	return speed;
}

void CFlowGenNHM::updateProperties()
{
	// update anything relevant to gap generation as hours changes

	//double mean =  3600.0 / m_vFlowRate;
	//m_RNG.setScale(mean);
	//m_RNG.setLocation(0.0);	// for exponential deviates
}

//////////// CFlowGenCongested ///////////////

CFlowGenCongested::CFlowGenCongested(CFlowModelDataCongested* pFDM) : CFlowGenerator(eCongested)
{
	m_pFDM = pFDM;

	m_pFDM->getGapParams(m_GapMean, m_GapStd);
}


CFlowGenCongested::~CFlowGenCongested()
{
}

double CFlowGenCongested::Generate()
{
	// Does this need to know about time for the vehicle to pass?

	// generate congested time gap
	return m_RNG.GenerateNormal(m_GapMean, m_GapStd);
}

double CFlowGenCongested::GenerateSpeed()
{
	double 	speed = m_RNG.GenerateNormal(m_pFDM->getSpeedMean(m_CurHour),
		m_pFDM->getSpeedStd(m_CurHour));
	return speed;
}

void CFlowGenCongested::updateProperties()
{
	// update anything relevant to gap generation as hours changes

	//double mean = 3600.0 / m_vFlowRate;
	//m_RNG.setScale(mean);
	//m_RNG.setLocation(0.0);	// for exponential deviates
}


///////////// CFlowGenPoisson ///////////////
CFlowGenPoisson::CFlowGenPoisson(CFlowModelDataPoisson* pFDM) : CFlowGenerator(ePoisson)
{
	m_pFDM = pFDM;
}


CFlowGenPoisson::~CFlowGenPoisson()
{
}

double CFlowGenPoisson::Generate()
{
	double gap = m_RNG.GenerateExponential();
	return gap;
}

double CFlowGenPoisson::GenerateSpeed()
{
	double 	speed = m_RNG.GenerateNormal(m_pFDM->getSpeedMean(m_CurHour),
		m_pFDM->getSpeedStd(m_CurHour));
	return speed;
}

void CFlowGenPoisson::updateProperties()
{
	// update anything relevant to gap generation as hours changes

	// We must amplify the truck flows because there are cars
	double truckPercent = 1.0 - m_CarPerc;
	double totalFlow = m_vFlowRate / truckPercent;
	double mean = 3600.0 / totalFlow;

	m_RNG.setScale(mean);
	m_RNG.setLocation(0.0);	// for exponential deviates
}