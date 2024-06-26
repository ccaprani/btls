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

	run("BTLSin.txt");
	return 1;
}
