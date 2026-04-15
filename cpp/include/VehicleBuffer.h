/**
 * @file VehicleBuffer.h
 * @brief Interface for the CVehicleBuffer class — buffered writer for generated vehicles.
 */

#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <math.h>
#include "Vehicle.h"
#include "ConfigData.h"

/**
 * @brief Per-hour flow counts for a single lane.
 *
 * Tracks the total number of vehicles, cars, and trucks seen during one
 * hour block, along with a class-indexed histogram of how many vehicles
 * of each class were observed.
 */
struct CFlowRateData
{
	size_t m_ID;                          ///< Lane identifier for this flow record.
	size_t m_NoVehicles;                  ///< Total vehicles in the block.
	size_t m_NoTrucks;                    ///< Trucks (non-car) in the block.
	size_t m_NoCars;                      ///< Cars in the block.
	std::vector<size_t> m_vClassCount;    ///< Count of vehicles of each classification.

	/**
	 * @brief Construct with a histogram sized to @p n classes (all zero).
	 * @param[in] n Number of classification buckets.
	 */
	CFlowRateData(size_t n)
	{
		m_ID = 0;
		m_NoVehicles = 0;
		m_NoTrucks = 0;
		m_NoCars = 0;
		m_vClassCount.assign(n, 0);
	};

	/**
	 * @brief Increment the count for the given classification bucket.
	 *
	 * Out-of-range indices fall through to bucket 0 (the default class).
	 *
	 * @param[in] iClass Classification index.
	 */
	void addByClass(size_t iClass)
	{
		if (iClass > m_vClassCount.size())
			m_vClassCount.at(0)++; // count as default
		else
			m_vClassCount.at(iClass)++;
	}
};

/**
 * @brief Buffered writer for generated vehicles and per-hour flow statistics.
 *
 * CVehicleBuffer accumulates vehicles emitted by @ref CLaneGenTraffic
 * (via its vehicle generator) and periodically flushes them to the
 * configured vehicle output file in the current file format. It also
 * maintains per-hour flow statistics — total vehicles, trucks, cars,
 * and class histograms per lane per hour — that are written out
 * alongside the vehicle file.
 *
 * Both outputs are optional: @c WRITE_VEHICLE_FILE enables the vehicle
 * stream and @c WRITE_FLOW_STATS enables the flow statistics stream.
 * Buffer size is controlled by @c WRITE_VEHICLE_BUFFER_SIZE.
 *
 * @see CLaneGenTraffic
 * @see CVehicleClassification
 */
class CVehicleBuffer
{
public:
	/**
	 * @brief Construct a buffer bound to the simulation configuration.
	 *
	 * @param[in] config    Shared configuration block.
	 * @param[in] pVC       Vehicle classifier used to categorise buffered vehicles.
	 * @param[in] starttime Simulation start time in seconds (used to initialise the hour counter).
	 */
	CVehicleBuffer(CConfigDataCore& config, CVehicleClassification_sp pVC, double starttime);
	virtual ~CVehicleBuffer();

	/**
	 * @brief Append a vehicle to the buffer and update the current hour's flow counts.
	 *
	 * If the buffer is full, it is flushed to the output file before the
	 * new vehicle is appended.
	 *
	 * @param[in] pVeh Generated vehicle (shared pointer; the buffer takes a copy).
	 */
	void AddVehicle(const CVehicle_sp& pVeh);

	/**
	 * @brief Flush any buffered vehicles and flow data to disk immediately.
	 */
	void FlushBuffer();

private:
	/// @brief Write all buffered vehicles to the vehicle output file.
	void writeFlowData();

	/// @brief Update the current hour's flow-count record with @p pVeh.
	void updateFlowData(const CVehicle_sp& pVeh);

	/// @brief Flush the per-hour flow statistics to the flow output file.
	void flushFlowData();

	std::ofstream m_OutFileVeh;             ///< Output stream for serialised vehicles.
	std::ofstream m_OutFileFlow;            ///< Output stream for per-hour flow stats.
	std::vector<CVehicle_up> m_vVehicles;   ///< Buffered vehicles awaiting flush.
	size_t m_NoVehicles;                    ///< Running count of vehicles flushed to disk.

	size_t m_FirstHour;                     ///< Hour index at which the simulation started.
	size_t m_CurHour;                       ///< Current hour index.
	std::vector< std::vector<CFlowRateData> > m_vFlowData;  ///< Per-hour, per-lane flow counters.

	size_t FILE_FORMAT;                     ///< Output file format tag (CASTOR/BeDIT/DITIS/MON).
	bool WRITE_VEHICLE_FILE;                ///< Enable the vehicle output stream.
	std::string VEHICLE_FILENAME;           ///< Output file path for vehicles.
	size_t WRITE_VEHICLE_BUFFER_SIZE;       ///< Buffer capacity in vehicles.
	bool WRITE_FLOW_STATS;                  ///< Enable the flow-statistics output stream.

	size_t NO_LANES_DIR1;                   ///< Lanes in direction 1 (from config).
	size_t NO_LANES_DIR2;                   ///< Lanes in direction 2 (from config).
	size_t NO_LANES;                        ///< Total lanes across both directions.

	CVehicleClassification_sp m_pVehClassification;  ///< Shared vehicle classifier.

	template <typename T> std::string to_string(T const& value);
};
