// main.cpp
// the main file for the BridgeTrafficLoadSim Build

#include "PrepareSim.h"
#include "BTLS_Config.h"

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
		sort(vLanes.begin(), vLanes.end(), [](const CLane_sp& pL1, const CLane_sp& pL2){
			return pL1->GetNextArrivalTime() < pL2->GetNextArrivalTime();
			});
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

	// system("PAUSE");
	return 1;
}
