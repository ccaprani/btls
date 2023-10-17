#pragma once

#include <iostream>
#include <iomanip>
#include <fstream>
#include <math.h>
#include <string>
#include <algorithm>
#include <time.h>
#include <stdlib.h>
#include <filesystem>
#include "ConfigData.h"
#include "Lane.h"
#include "LaneGenTraffic.h"
#include "LaneFileTraffic.h"
#include "VehicleGenerator.h"
#include "VehicleTrafficFile.h"
#include "VehicleBuffer.h"
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


std::vector<CBridge_sp> PrepareBridges();
void GetGeneratorLanes(CVehicleClassification_sp pVC, std::vector<CLane_sp>& vpLanes, const double& StartTime, double& EndTime);
void GetTrafficFileLanes(CVehicleClassification_sp pVC, std::vector<CLane_sp>& vpLanes, double& StartTime, double& EndTime);
void doSimulation(CVehicleClassification_sp pVC, std::vector<CBridge_sp> pBridges, std::vector<CLane_sp> pLanes, double SimStartTime, double SimEndTime);
void run(std::string inFile);
void get_info();
// inline bool lane_compare(const CLane_sp& pL1, const CLane_sp& pL2);
