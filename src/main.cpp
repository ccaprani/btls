//	main.cpp
// the main file for the BridgeTrafficLoadSim program

#include <omp.h> // for parallel
#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdlib.h> 
#include <math.h>
#include <string>
#include <algorithm>
#include <time.h>
#include <stdlib.h>
#include "ConfigData.h"
#include "TrafficFiles.h"
#include "Lane.h"
#include "LaneGenTraffic.h"
#include "LaneFileTraffic.h"
#include "VehicleGenerator.h"
#include "VehicleTrafficFile.h"
#include "VehicleBuffer.h"
#include "TrafficData.h"
#include "ReadILFile.h"
#include "BridgeFile.h"
#include "Bridge.h"
#include "LaneFlowData.h"

#ifdef Win
// for tracking memory leaks
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

//extern CConfigData g_ConfigData;
using namespace std;

void GetGeneratorLanes(CVehicleClassification_sp pVC, vector<CLane_sp>& vpLanes, const double& StartTime, double& EndTime);
void GetTrafficFileLanes(CVehicleClassification_sp pVC, vector<CLane_sp>& vpLanes, double& StartTime, double& EndTime);
vector<CBridge_sp> PrepareBridges();
void doSimulation(CVehicleClassification_sp pVC, vector<CBridge_sp> pBridges, vector<CLane_sp> pLanes, double SimStartTime, double SimEndTime);
inline bool lane_compare(const CLane_sp& pL1, const CLane_sp& pL2);

int main()
{
	#ifdef Win
	// For debugging memory leaks to the std::cout, but only after
	// all other execution has finished, otherwise false reports of
	// leaks occur (e.g. std::string)
	// https://stackoverflow.com/questions/4748391/string-causes-a-memory-leak
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
	#endif

	cout << "---------------------------------------------" << endl;
	cout << "Bridge Traffic Load Simulation - C.C. Caprani" << endl;
	cout << "                Version 1.3.5			      " << endl;
	cout << "---------------------------------------------" << endl << endl;

	if (!CConfigData::get().ReadData("BTLSin.txt") )
	{
		cout << "BTLSin file could not be opened" << endl;
		cout << "Using default values" << endl;
	}

	cout << "Program Mode: " << CConfigData::get().Mode.PROGRAM_MODE << endl;
	cout << endl;
	
	CVehicleClassification_sp pVC;
	switch (CConfigData::get().Traffic.CLASSIFICATION)
	{
	case 1: // Pattern
		pVC = std::make_shared<CVehClassPattern>(); break;
	default: // Axle count
		pVC = std::make_shared<CVehClassAxle>(); break;
	}

	double StartTime = 0.0;
	double EndTime = 0.0;

	// Bridges are read in first, to set max bridge length, needed for flow generators
	vector<CBridge_sp> vBridges;
	if(CConfigData::get().Sim.CALC_LOAD_EFFECTS)
		vBridges = PrepareBridges();

	// Now read generator info for lanes
	vector<CLane_sp> vLanes;
	if (CConfigData::get().Gen.GEN_TRAFFIC)	GetGeneratorLanes(pVC, vLanes, StartTime, EndTime); 
	if (CConfigData::get().Read.READ_FILE)	GetTrafficFileLanes(pVC, vLanes, StartTime, EndTime);

	// Now we know the time, we can tell bridge data managers when to start
	for (auto& it : vBridges)
		it->InitializeDataMgr(StartTime);
	
	clock_t start = clock();
	doSimulation(pVC, vBridges, vLanes, StartTime, EndTime);

	cout << endl << "Simulation complete" << endl;

	clock_t end = clock();
	cout << endl << "Duration of analysis: " << std::fixed << std::setprecision(3) 
		<< ((double)(end) - (double)(start))/((double)CLOCKS_PER_SEC) << " s" << endl;

	//system("PAUSE");
	return 0;
}

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

void doSimulation(CVehicleClassification_sp pVC, vector<CBridge_sp> vBridges, vector<CLane_sp> vLanes, double SimStartTime, double SimEndTime)
{
	CVehicleBuffer VehBuff(pVC, SimStartTime);
	size_t nLanes = vLanes.size();
	double curTime = SimStartTime;
	double nextTime = 0.0;
	int curDay = (int)(SimStartTime/86400);
	
	cout << "Starting simulation..." << endl;
	cout << "Day complete..." << endl;

//#pragma omp parallel
//	{
	while (curTime <= SimEndTime)
	{
		//if(curTime >= 74821.73)
		//	cout << "here" << endl;

		// find the next arrival lane and the time
		sort(vLanes.begin(), vLanes.end(), lane_compare);
		double NextArrivalTime = vLanes[0]->GetNextArrivalTime();

		// generate the next vehicle from the lane with the next arrival time	
		// see https://stackoverflow.com/questions/18565167/non-const-lvalue-references	
		const CVehicle_sp& pVeh = vLanes[0]->GetNextVehicle();
		VehBuff.AddVehicle(pVeh);
		if (CConfigData::get().Sim.CALC_LOAD_EFFECTS)
		{
//#pragma omp for
			for (size_t i = 0; i < vBridges.size(); i++)
			{
				// update each bridge until the next vehicle comes on
				vBridges[i]->Update(NextArrivalTime, curTime);
				//vBridges[i]->UpdateMT(NextArrivalTime, curTime);
				// Add the next vehicle to the bridge, if it is not a car
				if (pVeh != nullptr && pVeh->getGVW() > CConfigData::get().Sim.MIN_GVW)
					vBridges[i]->AddVehicle(pVeh);
			}
			//for(unsigned int i = 0; i < vBridges.size(); i++)
			//	vBridges[i]->join();
		}

		// update the current time to that of the vehicle just added
		// and delete it
		if (pVeh != nullptr)
		{
			curTime = pVeh->getTime();
			CVehicle_sp *pVeh = nullptr;
		}
		else	// finish
			curTime = SimEndTime + 1.0;

		// Keep informing the user
		if (curTime > (double)(86400)*(curDay + 1))
		{
			curDay += 1;
			cout << curDay;
			curDay % 10 == 0 ? cout << endl : cout << '\t';
		}
	}
//	}
	cout << endl;
	
	if(CConfigData::get().Sim.CALC_LOAD_EFFECTS)
	{
		for(unsigned int i = 0; i < vBridges.size(); i++)
			vBridges[i]->Finish();
	}

	VehBuff.FlushBuffer();
}

bool lane_compare(const CLane_sp& pL1, const CLane_sp& pL2)
{
	return pL1->GetNextArrivalTime() < pL2->GetNextArrivalTime();
}
