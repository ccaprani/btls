#pragma once

#include <iostream>
#include <iomanip>
#include <fstream>
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

vector<CBridge_sp> PrepareBridges();
void GetGeneratorLanes(CVehicleClassification_sp pVC, vector<CLane_sp>& vpLanes, const double& StartTime, double& EndTime);
void GetTrafficFileLanes(CVehicleClassification_sp pVC, vector<CLane_sp>& vpLanes, double& StartTime, double& EndTime);
void doSimulation(CVehicleClassification_sp pVC, vector<CBridge_sp> pBridges, vector<CLane_sp> pLanes, double SimStartTime, double SimEndTime);
// inline bool lane_compare(const CLane_sp& pL1, const CLane_sp& pL2);
