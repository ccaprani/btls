#include "LaneGenTraffic.h"
#include "ConfigData.h"

//extern CConfigData g_ConfigData;

CLaneGenTraffic::CLaneGenTraffic(void)
{
	m_pVehicleGen		= nullptr;
	m_pVehModelData		= nullptr;
	m_pFlowGen			= nullptr;
	m_pFlowModelData	= nullptr;
	m_pPrevVeh			= nullptr;
	m_pNextVeh			= nullptr;
	
	HEADWAY_MODEL			= CConfigData::get().Traffic.HEADWAY_MODEL;
	VEHICLE_MODEL			= CConfigData::get().Traffic.VEHICLE_MODEL;

	NO_LANES				= CConfigData::get().Road.NO_LANES;
}


CLaneGenTraffic::~CLaneGenTraffic(void)
{
	m_pVehModelData = nullptr;
	m_pVehicleGen = nullptr;
	m_pFlowModelData = nullptr;
	m_pFlowGen = nullptr;
}


void CLaneGenTraffic::setLaneData(CVehicleClassification_sp pVC, 
									CLaneFlowComposition lfc, const double starttime)
{
	m_NextArrivalTime = starttime;

	m_Direction = lfc.getDirn();
	// Map vehicles to local lane using zero based cumulative lane no.
	if (m_Direction == 2)
		m_LaneIndex = NO_LANES - lfc.getGlobalLaneNo();
	else
		m_LaneIndex = lfc.getGlobalLaneNo();

	// Vehicle model must come first for NHM temporary
	switch (VEHICLE_MODEL)
	{
	case 1:		// Constant
		m_pVehModelData = std::make_shared<CVehModelDataConstant>(pVC, lfc);
		m_pVehicleGen = std::make_shared<CVehicleGenConstant>(std::dynamic_pointer_cast<CVehModelDataConstant>(m_pVehModelData));
		break;
	case 2:		// Garage
		m_pVehModelData = std::make_shared<CVehModelDataGarage>(pVC, lfc);
		m_pVehicleGen = std::make_shared<CVehicleGenGarage>(std::dynamic_pointer_cast<CVehModelDataGarage>(m_pVehModelData));
		break;
	default:	// Grave
		m_pVehModelData = std::make_shared<CVehModelDataGrave>(pVC, lfc);
		m_pVehicleGen = std::make_shared<CVehicleGenGrave>(std::dynamic_pointer_cast<CVehModelDataGrave>(m_pVehModelData));
		break;
	}

	switch (HEADWAY_MODEL)
	{
	case 0:		// NHM
		m_pFlowModelData = std::make_shared<CFlowModelDataNHM>(lfc);
		m_pFlowGen = std::make_shared<CFlowGenNHM>(std::dynamic_pointer_cast<CFlowModelDataNHM>(m_pFlowModelData));
		break;
	case 1:		// Constant test
		m_pFlowModelData = std::make_shared<CFlowModelDataConstant>(lfc);
		m_pFlowGen = std::make_shared<CFlowGenConstant>(std::dynamic_pointer_cast<CFlowModelDataConstant>(m_pFlowModelData));
		break;
	case 5:		// Congestion
		m_pFlowModelData = std::make_shared<CFlowModelDataCongested>(lfc);
		m_pFlowGen = std::make_shared<CFlowGenCongested>(std::dynamic_pointer_cast<CFlowModelDataCongested>(m_pFlowModelData));
		break;
	default:	// Poisson arrivals
		m_pFlowModelData = std::make_shared<CFlowModelDataPoisson>(lfc);
		m_pFlowGen = std::make_shared<CFlowGenPoisson>(std::dynamic_pointer_cast<CFlowModelDataPoisson>(m_pFlowModelData));
		break;
	}

	// Now update the vehicle generator about the flow model
	m_pVehicleGen->update(m_pFlowModelData);

	GenNextArrival();	// the first arrival generation
}


CVehicle_sp CLaneGenTraffic::GetNextVehicle()
{
	CVehicle_sp pVeh = m_pNextVeh;	// temp store
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
	m_pNextVeh = m_pVehicleGen->Generate(m_pFlowGen->getCurBlock());
	m_pNextVeh->setDirection(m_Direction);
	m_pNextVeh->setGlobalLane(m_LaneIndex + 1, NO_LANES);
}


void CLaneGenTraffic::GenNextTime()
{
	double gap = 0.0;
	
	m_pFlowGen->prepareNextGen(m_NextArrivalTime, m_pPrevVeh, m_pNextVeh);
	gap = m_pFlowGen->Generate();

	m_NextArrivalTime += gap;	
}