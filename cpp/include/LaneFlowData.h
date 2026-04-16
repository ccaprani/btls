/**
 * @file LaneFlowData.h
 * @brief Interface for the CLaneFlowData class — reads the lane-flow CSV and builds per-lane compositions.
 */

#pragma once
#include "ModelData.h"
#include "LaneFlowComposition.h"
#include <memory>
// forward declare
class CVehicle; typedef std::shared_ptr<CVehicle> CVehicle_sp;

/**
 * @brief Reads the lane-flow CSV file and builds a
 *        @ref CLaneFlowComposition for each lane.
 *
 * The lane-flow file defines the traffic stream for every lane in the
 * road layout: per-block (typically hourly) flow rates, truck
 * percentages, class percentages, and speed distributions. The file
 * is named by @c CConfigDataCore::Road::LANES_FILE.
 *
 * On ReadDataIn(), CLaneFlowData parses the CSV, determines the number
 * of lanes, directions, and block structure, and populates the
 * road-layout fields in the configuration via
 * @ref CConfigDataCore::setRoad().
 *
 * @see CLaneFlowComposition
 * @see CModelData
 * @see CLaneGenTraffic
 */
class CLaneFlowData : public CModelData
{
public:
	/**
	 * @brief Construct bound to the shared configuration.
	 * @param[in] config Shared configuration block.
	 */
	CLaneFlowData(CConfigDataCore& config);
	virtual ~CLaneFlowData();

	/// @brief Parse the lane-flow CSV and populate internal state.
	virtual void ReadDataIn();

	/**
	 * @brief Get the composition for lane @p i.
	 * @param[in] i Zero-based lane index.
	 */
	CLaneFlowComposition getLaneComp(size_t i) const;

	/// @brief Get the number of directions represented (1 or 2).
	size_t getNoDirn() const { return m_NoDir;};

	/// @brief Get the total number of lanes.
	size_t getNoLanes() const { return m_NoLanes; };

	/// @brief Get the number of lanes in direction 1.
	size_t getNoLanesDir1() const { return m_NoLanesDir1; };

	/// @brief Get the number of lanes in direction 2.
	size_t getNoLanesDir2() const { return m_NoLanesDir2; };

private:
	/// @brief Parse the lane-flow CSV file.
	void ReadLaneFlow(std::string file);

	/// @brief Populate road-layout fields in the configuration from parsed data.
	void SetRoadProperties();

	size_t m_NoLanes;          ///< Total lanes.
	size_t m_NoDir;            ///< Number of directions (1 or 2).
	size_t m_NoLanesDir1;      ///< Lanes in direction 1.
	size_t m_NoLanesDir2;      ///< Lanes in direction 2.

	size_t m_TruckClassCount;  ///< Number of truck classes in the CSV.
	size_t m_BlockSize;        ///< Block size in seconds (typically 3600).
	size_t m_NoBlocks;         ///< Number of blocks per cycle (typically 24).

	std::vector<CLaneFlowComposition> m_vLaneComp;  ///< Per-lane compositions.

	std::vector<CVehicle_sp> m_vVehicles;  ///< (Reserved for future use.)
};
typedef std::shared_ptr<CLaneFlowData> CLaneFlowData_sp;  ///< Shared-pointer alias for CLaneFlowData.
