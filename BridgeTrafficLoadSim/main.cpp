//	main.cpp
// the main file for the BridgeTrafficLoadSim program

#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdlib.h> 
#include <math.h>
#include <string>
#include <algorithm>
#include <time.h>
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
#include "LaneFlow.h"

extern CConfigData g_ConfigData;
using namespace std;

void GetGeneratorLanes(vector<CLane*>& vpLanes, double& StartTime, double& EndTime);
void GetTrafficFileLanes(vector<CLane*>& vpLanes, double& StartTime, double& EndTime);
vector<CBridge*> PrepareBridges();
void doSimulation(vector<CBridge*> pBridges, vector<CLane*> pLanes, double SimStartTime, double SimEndTime);
bool lane_compare(const CLane* pL1, const CLane* pL2);

void main()
{
	cout << "---------------------------------------------" << endl;
	cout << "Bridge Traffic Load Simulation - C.C. Caprani" << endl;
	cout << "                Version 1.2.4                " << endl;
	cout << "---------------------------------------------" << endl << endl;

	if (!g_ConfigData.ReadData("BTLSin.txt") )
	{
		cout << "BTLSin file could not be opened" << endl;
		cout << "Using default values" << endl;
	}

	cout << "Program Mode: " << g_ConfigData.Mode.PROGRAM_MODE << endl;
	cout << endl;
	
	double StartTime = 0.0; double EndTime = 0.0;
	vector<CLane*> vLanes;
	if(g_ConfigData.Gen.GEN_TRAFFIC) GetGeneratorLanes(vLanes, StartTime, EndTime);
	if(g_ConfigData.Read.READ_FILE)	GetTrafficFileLanes(vLanes, StartTime, EndTime);
	
	vector<CBridge*> vBridges;
	if(g_ConfigData.Sim.CALC_LOAD_EFFECTS)
		vBridges = PrepareBridges();

	clock_t start = clock();
	doSimulation(vBridges, vLanes, StartTime, EndTime);

	cout << endl << "Simulation complete" << endl;

	clock_t end = clock();
	cout << endl << "Duration of analysis: " << std::fixed << std::setprecision(3) 
		<< (double)((end - start)/(double)CLOCKS_PER_SEC) << " s" << endl;

	system("PAUSE");
}

vector<CBridge*> PrepareBridges()
{
	CReadILFile readIL;
	CBridgeFile BridgeFile(g_ConfigData.Sim.BRIDGE_FILE,
		readIL.getInfLines(g_ConfigData.Sim.INFLINE_FILE,0),	// discrete ILs
		readIL.getInfLines(g_ConfigData.Sim.INFSURF_FILE,1));	// Influence Surfaces
	vector<CBridge*> vpBridges = BridgeFile.getBridges();
	g_ConfigData.Gen.NO_OVERLAP_LENGTH = BridgeFile.getMaxBridgeLength();

	for(unsigned int i = 0; i < vpBridges.size(); i++)
		vpBridges.at(i)->setCalcTimeStep( g_ConfigData.Sim.CALC_TIME_STEP );

	return vpBridges;
}

void GetGeneratorLanes(vector<CLane*>& vpLanes, double& StartTime, double& EndTime)
{
	// Useful debugging tool - set start time higher
	//int startday = 0;
	StartTime = 0.0; // (double)(startday)*86400;
	EndTime = (double)(g_ConfigData.Gen.NO_DAYS)*24*3600;
	
	// read in the vehicle generation data
	CTrafficFiles DataIn;//g_ConfigData.Gen.SITE_WEIGHT);
	DataIn.ReadPhysical();
	if( g_ConfigData.Traffic.HEADWAY_MODEL == 0 )
		DataIn.ReadFile_NHM();
	vector<CLaneFlow> vLaneFlow = DataIn.ReadLaneFlow(g_ConfigData.Road.LANES_FILE);
	CTrafficData TD = DataIn.GetTrafficData();
	TD.SetLaneFlow(vLaneFlow);
	
	g_ConfigData.Road.NO_LANES		= TD.getNoLanes();
	g_ConfigData.Road.NO_DIRS		= TD.getNoDirn();
	g_ConfigData.Road.NO_LANES_DIR1 = TD.getNoLanesDir1();
	g_ConfigData.Road.NO_LANES_DIR2 = TD.getNoLanesDir2();

	for(int i = 0; i < TD.getNoLanes(); ++i)
	{
		CLaneGenTraffic* pLane = new CLaneGenTraffic;
		CVehicleGenerator* pGen = new CVehicleGenerator(TD, vLaneFlow.at(i));
		pLane->setLaneData(pGen, vLaneFlow.at(i), StartTime);
		vpLanes.push_back(pLane);
	}
}

void GetTrafficFileLanes(vector<CLane*>& vpLanes, double& StartTime, double& EndTime)
{
	CVehicleTrafficFile TrafficFile(g_ConfigData.Read.USE_CONSTANT_SPEED, 
		g_ConfigData.Read.USE_AVE_SPEED, g_ConfigData.Read.CONST_SPEED);
	cout << "Reading traffic file..." << endl;
	TrafficFile.Read(g_ConfigData.Read.TRAFFIC_FILE,g_ConfigData.Read.FILE_FORMAT);
	
	g_ConfigData.Gen.NO_DAYS		= TrafficFile.getNoDays();
	g_ConfigData.Road.NO_LANES		= TrafficFile.getNoLanes();
	g_ConfigData.Road.NO_DIRS		= TrafficFile.getNoDirn();
	g_ConfigData.Road.NO_LANES_DIR1 = TrafficFile.getNoLanesDir1();
	g_ConfigData.Road.NO_LANES_DIR2 = TrafficFile.getNoLanesDir2();

	if(g_ConfigData.Gen.NO_DAYS == 0)	std::cout << "*** ERROR: No traffic in vehicle file" << std::endl;
	if(g_ConfigData.Road.NO_LANES == 0)	std::cout << "*** ERROR: No lanes in vehicle file" << std::endl;
	if(g_ConfigData.Road.NO_DIRS == 0)	std::cout << "*** ERROR: No directions in vehicle file" << std::endl;
	if(g_ConfigData.Road.NO_DIRS == 2 && g_ConfigData.Road.NO_LANES == 1)	std::cout << "*** ERROR: Two directions and one lane detected" << std::endl;

	for(unsigned int i = 0; i < TrafficFile.getNoLanes(); ++i)
	{
		CLaneFileTraffic* pLane = new CLaneFileTraffic;
		int dirn = i < TrafficFile.getNoLanesDir1() ? 1 : 2;
		pLane->setLaneData(dirn,i); // direction and lane number
		vpLanes.push_back(pLane);
	}
	for(unsigned int i = 0; i < TrafficFile.getNoVehicles(); ++i)
	{
		CVehicle* pVeh = TrafficFile.getNextVehicle();
		CLaneFileTraffic* pLane = static_cast<CLaneFileTraffic*>( vpLanes.at( pVeh->getLane() - 1 ) ); // zero based lane no.
		pLane->addVehicle(pVeh);
	}
	for(unsigned int i = 0; i < TrafficFile.getNoLanes(); ++i)
	{
		CLaneFileTraffic* pLane = static_cast<CLaneFileTraffic*>( vpLanes.at(i) );
		if(pLane->GetNoVehicles() > 0)
			pLane->setFirstArrivalTime();
		else
			std::cout << "*** WARNING: Lane " << i+1 << " Direction " << pLane->GetDirection() << " has no vehicles" << std::endl;
	}
	
	StartTime = TrafficFile.getStartTime();
	EndTime = TrafficFile.getEndTime();
}

void doSimulation(vector<CBridge*> vBridges, vector<CLane*> vLanes, double SimStartTime, double SimEndTime)
{
	CVehicleBuffer VehBuff;//(g_ConfigData.Output.VehicleFile.WRITE_VEHICLE_FILE, 
		//g_ConfigData.Output.VehicleFile.VEHICLE_FILENAME, g_ConfigData.Output.VehicleFile.WRITE_VEHICLE_BUFFER_SIZE);
	size_t nLanes = vLanes.size();
	double curTime = SimStartTime;
	double nextTime = 0.0;
	int curDay = (int)(SimStartTime/(3600*24));
	
	cout << "Starting simulation..." << endl;
	cout << "Day complete..." << endl;

	while(curTime <= SimEndTime)
	{
//		if(curTime >= 195725062.20)
//			cout << "here" << endl;

		// find the next arrival lane and the time
		sort(vLanes.begin(), vLanes.end(), lane_compare);
		double NextArrivalTime = vLanes[0]->GetNextArrivalTime();
		
		// generate the next vehicle from the lane with the next arrival time		
		CVehicle* pVeh = vLanes[0]->GetNextVehicle();
		VehBuff.AddVehicle(pVeh);
		if(g_ConfigData.Sim.CALC_LOAD_EFFECTS)
		{
			for(unsigned int i = 0; i < vBridges.size(); i++)
			{
				// update each bridge until the next vehicle comes on
				vBridges[i]->Update(NextArrivalTime, curTime);
				//vBridges[i]->UpdateMT(NextArrivalTime, curTime);
				// Add the next vehicle to the bridge, if it is not a car
				if(pVeh != NULL && pVeh->getGVW() > g_ConfigData.Sim.MIN_GVW)
					vBridges[i]->AddVehicle(pVeh);
			}
			//for(unsigned int i = 0; i < vBridges.size(); i++)
			//	vBridges[i]->join();
		}

		// update the current time to that of the vehicle just added
		// and delete it
		if(pVeh != NULL)
		{
			curTime = pVeh->getTime();
			delete pVeh;
		}
		else	// finish
			curTime = SimEndTime + 1.0;

		// Keep informing the user
		if(curTime > (double)(86400)*(curDay+1))
		{
			curDay += 1;
			cout << curDay;
			curDay%10==0 ? 	cout << endl : cout << '\t';
		}
	}
	cout << endl;
	
	if(g_ConfigData.Sim.CALC_LOAD_EFFECTS)
	{
		for(unsigned int i = 0; i < vBridges.size(); i++)
			vBridges[i]->Finish();
	}

	VehBuff.FlushBuffer();
}

bool lane_compare(const CLane* pL1, const CLane* pL2)
{
	return pL1->GetNextArrivalTime() < pL2->GetNextArrivalTime();
}
