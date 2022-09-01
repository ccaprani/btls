// py_main.cpp
// the main file for the BtlsPy Build

#include "PrepareSim.h"


//extern CConfigData g_ConfigData;
using namespace std;


vector<CBridge_sp> PrepareBridges()
{
	CReadILFile readIL;
	CBridgeFile BridgeFile(CConfigData::get().Sim.BRIDGE_FILE,
		readIL.getInfLines(CConfigData::get().Sim.INFLINE_FILE,0),	// discrete ILs
		readIL.getInfLines(CConfigData::get().Sim.INFSURF_FILE,1));	// Influence Surfaces

	vector<CBridge_sp> vpBridges = BridgeFile.getBridges();
	CConfigData::get().Gen.NO_OVERLAP_LENGTH = BridgeFile.getMaxBridgeLength();

	for(unsigned int i = 0; i < vpBridges.size(); i++)
		vpBridges.at(i)->setCalcTimeStep( CConfigData::get().Sim.CALC_TIME_STEP );

	return vpBridges;
}

void GetGeneratorLanes(CVehicleClassification_sp pVC, vector<CLane_sp>& vpLanes, const double& StartTime, double& EndTime)
{
	// Useful debugging tool - set start time higher
	EndTime = (double)(CConfigData::get().Gen.NO_DAYS)*24*3600;
	
	CLaneFlowData LaneFlowData; 
	LaneFlowData.ReadDataIn();
	
	CConfigData::get().Road.NO_LANES		= LaneFlowData.getNoLanes();
	CConfigData::get().Road.NO_DIRS			= LaneFlowData.getNoDirn();
	CConfigData::get().Road.NO_LANES_DIR1	= LaneFlowData.getNoLanesDir1();
	CConfigData::get().Road.NO_LANES_DIR2	= LaneFlowData.getNoLanesDir2();

	for (size_t i = 0; i < LaneFlowData.getNoLanes(); ++i)
	{
		CLaneGenTraffic_sp pLane = std::make_shared<CLaneGenTraffic>();
		pLane->setLaneData(pVC, LaneFlowData.getLaneComp(i), StartTime);
		vpLanes.push_back(pLane);
	}
}

void GetTrafficFileLanes(CVehicleClassification_sp pVC, vector<CLane_sp>& vpLanes, double& StartTime, double& EndTime)
{
	CVehicleTrafficFile TrafficFile(pVC, CConfigData::get().Read.USE_CONSTANT_SPEED, 
		CConfigData::get().Read.USE_AVE_SPEED, CConfigData::get().Read.CONST_SPEED);
	cout << "Reading traffic file..." << endl;
	TrafficFile.Read(CConfigData::get().Read.TRAFFIC_FILE,CConfigData::get().Read.FILE_FORMAT);
	
	CConfigData::get().Gen.NO_DAYS		= TrafficFile.getNoDays();
	CConfigData::get().Road.NO_LANES		= TrafficFile.getNoLanes();
	CConfigData::get().Road.NO_DIRS		= TrafficFile.getNoDirn();
	CConfigData::get().Road.NO_LANES_DIR1 = TrafficFile.getNoLanesDir1();
	CConfigData::get().Road.NO_LANES_DIR2 = TrafficFile.getNoLanesDir2();

	if(CConfigData::get().Gen.NO_DAYS == 0)	std::cout << "*** ERROR: No traffic in vehicle file" << std::endl;
	if(CConfigData::get().Road.NO_LANES == 0)	std::cout << "*** ERROR: No lanes in vehicle file" << std::endl;
	if(CConfigData::get().Road.NO_DIRS == 0)	std::cout << "*** ERROR: No directions in vehicle file" << std::endl;
	if(CConfigData::get().Road.NO_DIRS == 2 && CConfigData::get().Road.NO_LANES == 1)	
		std::cout << "*** ERROR: Two directions and one lane detected" << std::endl;

	for(unsigned int i = 0; i < TrafficFile.getNoLanes(); ++i)
	{
		CLaneFileTraffic_sp pLane = std::make_shared<CLaneFileTraffic>();
		int dirn = i < TrafficFile.getNoLanesDir1() ? 1 : 2;
		pLane->setLaneData(dirn,i); // direction and cumulative lane number
		vpLanes.push_back(pLane);
	}
	for(unsigned int i = 0; i < TrafficFile.getNoVehicles(); ++i)
	{
		CVehicle_sp pVeh = TrafficFile.getNextVehicle();

		// Map vehicle to lane using zero based cumulative lane no
		size_t iLane = pVeh->getGlobalLane(CConfigData::get().Road.NO_LANES) - 1;
		CLaneFileTraffic_sp pLane = std::static_pointer_cast<CLaneFileTraffic>(vpLanes.at(iLane));
		pLane->addVehicle(pVeh);
	}
	for(unsigned int i = 0; i < TrafficFile.getNoLanes(); ++i)
	{
		CLaneFileTraffic_sp pLane = std::static_pointer_cast<CLaneFileTraffic>(vpLanes.at(i));
		if(pLane->GetNoVehicles() > 0)
			pLane->setFirstArrivalTime();
		else
			std::cout << "*** WARNING: Lane " << i+1 << " Direction " << pLane->GetDirection() << " has no vehicles" << std::endl;
	}
	
	StartTime = TrafficFile.getStartTime();
	EndTime = TrafficFile.getEndTime();
}

// bool lane_compare(const CLane_sp& pL1, const CLane_sp& pL2)
// {
// 	return pL1->GetNextArrivalTime() < pL2->GetNextArrivalTime();
// }
