#include "LaneGenTraffic.h"


CLaneGenTraffic::CLaneGenTraffic(CConfigDataCore& config): m_Config(config)
{
	m_pVehicleGen		= nullptr;
	m_pVehModelData		= nullptr;
	m_pFlowGen			= nullptr;
	m_pFlowModelData	= nullptr;
	m_pPrevVeh			= nullptr;
	m_pNextVeh			= nullptr;
	
	HEADWAY_MODEL	= m_Config.Traffic.HEADWAY_MODEL;
	VEHICLE_MODEL	= m_Config.Traffic.VEHICLE_MODEL;

	NO_LANES		= m_Config.Road.NO_LANES;
}

CLaneGenTraffic::~CLaneGenTraffic(void)
{
	m_pVehModelData = nullptr;
	m_pVehicleGen = nullptr;
	m_pFlowModelData = nullptr;
	m_pFlowGen = nullptr;
}

void CLaneGenTraffic::setLaneData(CVehicleClassification_sp pVC, 
									CLaneFlowComposition lfc, const double startTime)
{
	setLaneAttributes(lfc, startTime);

	// Vehicle model must come first for NHM temporary
	switch (VEHICLE_MODEL)
	{
	case 1:		// Constant
		m_pVehModelData = std::make_shared<CVehModelDataNominal>(m_Config, pVC, lfc);
		m_pVehicleGen = std::make_shared<CVehicleGenNominal>(std::dynamic_pointer_cast<CVehModelDataNominal>(m_pVehModelData));
		break;
	case 2:		// Garage
		m_pVehModelData = std::make_shared<CVehModelDataGarage>(m_Config, pVC, lfc);
		m_pVehicleGen = std::make_shared<CVehicleGenGarage>(std::dynamic_pointer_cast<CVehModelDataGarage>(m_pVehModelData));
		break;
	default:	// Grave
		m_pVehModelData = std::make_shared<CVehModelDataGrave>(m_Config, pVC, lfc);
		m_pVehicleGen = std::make_shared<CVehicleGenGrave>(std::dynamic_pointer_cast<CVehModelDataGrave>(m_pVehModelData));
		break;
	}

	switch (HEADWAY_MODEL)
	{
	case 0:		// NHM
		m_pFlowModelData = std::make_shared<CFlowModelDataNHM>(m_Config, lfc);
		m_pFlowGen = std::make_shared<CFlowGenNHM>(std::dynamic_pointer_cast<CFlowModelDataNHM>(m_pFlowModelData));
		break;
	case 1:		// Constant
		m_pFlowModelData = std::make_shared<CFlowModelDataConstant>(m_Config, lfc);
		m_pFlowGen = std::make_shared<CFlowGenConstant>(std::dynamic_pointer_cast<CFlowModelDataConstant>(m_pFlowModelData));
		break;
	case 5:		// Congested
		m_pFlowModelData = std::make_shared<CFlowModelDataCongested>(m_Config, lfc);
		m_pFlowGen = std::make_shared<CFlowGenCongested>(std::dynamic_pointer_cast<CFlowModelDataCongested>(m_pFlowModelData));
		break;
	default:	// Poisson arrivals
		m_pFlowModelData = std::make_shared<CFlowModelDataPoisson>(m_Config, lfc);
		m_pFlowGen = std::make_shared<CFlowGenPoisson>(std::dynamic_pointer_cast<CFlowModelDataPoisson>(m_pFlowModelData));
		break;
	}

	initLane(m_pFlowModelData);
}


void CLaneGenTraffic::setLaneData(CLaneFlowComposition lfc, CVehicleGenerator_sp pVehicleGen, CFlowGenerator_sp pFlowGen, const double startTime)
{
	setLaneAttributes(lfc, startTime);

	m_pVehicleGen = pVehicleGen;
	m_pFlowGen = pFlowGen;
}


void CLaneGenTraffic::initLane(CFlowModelData_sp pFlowModelData)
{
	// Now update the vehicle generator about the flow model
	m_pVehicleGen->update(pFlowModelData);

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
	m_pNextVeh->setLocalFromGlobalLane(m_LaneIndex + 1, NO_LANES);
}


void CLaneGenTraffic::GenNextTime()
{
	double gap = 0.0;
	
	m_pFlowGen->prepareNextGen(m_NextArrivalTime, m_pPrevVeh, m_pNextVeh);
	gap = m_pFlowGen->Generate();

	m_NextArrivalTime += gap;	
}