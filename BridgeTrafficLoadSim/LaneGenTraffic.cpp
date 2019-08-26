#include "LaneGenTraffic.h"

#include "ConfigData.h"

extern CConfigData g_ConfigData;

CLaneGenTraffic::CLaneGenTraffic(void)
{
	m_pVehicleGen = NULL;
	m_pFlowGen = NULL;
	m_pFlowModelData = NULL;
	m_pPrevVeh = NULL;
	m_pNextVeh = NULL;

	CONGESTED_GAP			= g_ConfigData.Traffic.CONGESTED_GAP;
	CONGESTED_GAP_COEF_VAR	= g_ConfigData.Traffic.CONGESTED_GAP_COEF_VAR;
	CONGESTED_SPEED			= g_ConfigData.Traffic.CONGESTED_SPEED;
	HEADWAY_MODEL			= g_ConfigData.Traffic.HEADWAY_MODEL;

	NO_OVERLAP_LENGTH		= g_ConfigData.Gen.NO_OVERLAP_LENGTH;
}


CLaneGenTraffic::~CLaneGenTraffic(void)
{
	delete m_pFlowModelData;
	delete m_pFlowGen;
}


void CLaneGenTraffic::setLaneData(CVehicleGenerator* pGen, CLaneFlow lane_flow, double starttime)
{
	m_pVehicleGen = pGen;
	m_NextArrivalTime = starttime;
	m_LaneIndex = lane_flow.getLaneNo();
	m_Direction = lane_flow.getDirn();

	switch (HEADWAY_MODEL)
	{
	case 0:		// NHM
		m_pFlowModelData = new CFlowModelDataNHM(lane_flow, m_pVehicleGen->GetNHM()); // TEMPORARY
		m_pFlowGen = new CFlowGenNHM(dynamic_cast<CFlowModelDataNHM*>(m_pFlowModelData));
		break;
	case 5:		// Congestion
		m_pFlowModelData = new CFlowModelDataCongested(lane_flow, CONGESTED_GAP, CONGESTED_GAP*CONGESTED_GAP_COEF_VAR);
		m_pFlowGen = new CFlowGenCongested(dynamic_cast<CFlowModelDataCongested*>(m_pFlowModelData));
		break;
	default:	// Poisson arrivals
		m_pFlowModelData = new CFlowModelDataPoisson(lane_flow);
		m_pFlowGen = new CFlowGenPoisson(dynamic_cast<CFlowModelDataPoisson*>(m_pFlowModelData));
		break;
	}

	GenNextArrival();	// the first arrival generation
}

CVehicle* CLaneGenTraffic::GetNextVehicle()
{
	CVehicle* pVeh = m_pNextVeh;	// temp store
	GenNextArrival();				// prepare next vehicle	
	return pVeh;					// send out last vehicle
}

void CLaneGenTraffic::GenNextArrival()
{
	GenNextVehicle();
	GenNextTime();
	m_pNextVeh->setTime(m_NextArrivalTime); // update with new arrival time	
}

void CLaneGenTraffic::GenNextVehicle()
{
	m_pPrevVeh = m_pNextVeh;	// assign as previous vehicle
	m_pNextVeh = m_pVehicleGen->Generate(m_pFlowGen->getHour());
}

void CLaneGenTraffic::GenNextTime()
{
	double gap = 0.0;
	
	m_pFlowGen->prepareNextGen(m_NextArrivalTime, m_pPrevVeh, m_pNextVeh);
	gap = m_pFlowGen->Generate();

	m_NextArrivalTime += gap;	
}