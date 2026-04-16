/**
 * @file PrepareSim.h
 * @brief Free functions forming the standalone BTLS binary entry point.
 *
 * When BTLS is built as a standalone executable (the @c Binary CMake
 * target), @c main() calls run(), which parses the @c BTLSin.txt
 * configuration file, prepares the lane and bridge objects, and
 * dispatches to doSimulation(). These functions are not compiled into
 * the PyBTLS pybind11 extension — the Python layer reimplements the
 * same orchestration in @c simulation.py.
 */

#pragma once

#include <iostream>
#include <iomanip>
#include <fstream>
#include <math.h>
#include <string>
#include <algorithm>
#include <time.h>
#include <stdlib.h>
#include <numeric>
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


#ifdef WIN_DEBUG
// for tracking memory leaks
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif


/// @brief Read influence line/surface files and construct the bridge objects.
std::vector<CBridge_sp> PrepareBridges();

/**
 * @brief Build the lane objects for generated-traffic mode.
 *
 * @param[in]  pVC        Vehicle classifier.
 * @param[out] vpLanes    Populated vector of lane shared pointers.
 * @param[in]  StartTime  Simulation start time in seconds.
 * @param[out] EndTime    Computed simulation end time in seconds.
 */
void GetGeneratorLanes(CVehicleClassification_sp pVC, std::vector<CLane_sp>& vpLanes, const double& StartTime, double& EndTime);

/**
 * @brief Build the lane objects for traffic-file replay mode.
 *
 * @param[in]  pVC       Vehicle classifier.
 * @param[out] vpLanes   Populated vector of lane shared pointers.
 * @param[out] StartTime Start time determined from the traffic file.
 * @param[out] EndTime   End time determined from the traffic file.
 */
void GetTrafficFileLanes(CVehicleClassification_sp pVC, std::vector<CLane_sp>& vpLanes, double& StartTime, double& EndTime);

/**
 * @brief Run the core time-stepping simulation.
 *
 * Pulls vehicles from lanes, places them on bridges, advances the
 * bridges via CBridge::Update, and records events until the simulation
 * end time is reached.
 *
 * @param[in] pVC           Vehicle classifier.
 * @param[in] pBridges      Bridges to simulate.
 * @param[in] pLanes        Traffic-lane streams.
 * @param[in] SimStartTime  Simulation start time in seconds.
 * @param[in] SimEndTime    Simulation end time in seconds.
 */
void doSimulation(CVehicleClassification_sp pVC, std::vector<CBridge_sp> pBridges, std::vector<CLane_sp> pLanes, double SimStartTime, double SimEndTime);

/**
 * @brief Main entry point for the standalone BTLS binary.
 *
 * Parses the BTLSin configuration file, sets up bridges and lanes,
 * runs the simulation, and prints a summary. Called from main() in
 * @c main.cpp.
 *
 * @param[in] inFile Path to the BTLSin.txt file.
 */
void run(std::string inFile);

/// @brief Print the BTLS version and copyright preamble.
void preamble();
// inline bool lane_compare(const CLane_sp& pL1, const CLane_sp& pL2);
