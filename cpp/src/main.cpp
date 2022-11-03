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

	std::cout << "---------------------------------------------" << std::endl;
	std::cout << "Bridge Traffic Load Simulation - C.C. Caprani" << std::endl;
	std::cout << "                Version 1.3.7			      " << std::endl;
	std::cout << "---------------------------------------------" << std::endl << std::endl;

	std::cout << "CWD: " << std::filesystem::current_path() << std::endl << std::endl;

	if (!CConfigData::get().ReadData("BTLSin.txt") )
	{
		std::cout << "BTLSin file could not be opened" << std::endl;
		std::cout << "Using default values" << std::endl;
	}

	std::cout << "Program Mode: " << CConfigData::get().Mode.PROGRAM_MODE << std::endl;
	std::cout << std::endl;
	
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
	std::vector<CBridge_sp> vBridges;
	if(CConfigData::get().Sim.CALC_LOAD_EFFECTS)
		vBridges = PrepareBridges();

	// Now read generator info for lanes
	std::vector<CLane_sp> vLanes;
	if (CConfigData::get().Gen.GEN_TRAFFIC)	GetGeneratorLanes(pVC, vLanes, StartTime, EndTime); 
	if (CConfigData::get().Read.READ_FILE)	GetTrafficFileLanes(pVC, vLanes, StartTime, EndTime);

	// Now we know the time, we can tell bridge data managers when to start
	for (auto& it : vBridges)
		it->InitializeDataMgr(StartTime);
	
	clock_t start = clock();
	doSimulation(pVC, vBridges, vLanes, StartTime, EndTime);

	std::cout << std::endl << "Simulation complete" << std::endl;

	clock_t end = clock();
	std::cout << std::endl << "Duration of analysis: " << std::fixed << std::setprecision(3) 
		<< ((double)(end) - (double)(start))/((double)CLOCKS_PER_SEC) << " s" << std::endl;

	// system("PAUSE");
	return 1;
}
