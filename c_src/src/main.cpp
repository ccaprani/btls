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
