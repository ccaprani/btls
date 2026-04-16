/**
 * @file LaneFlowComposition.h
 * @brief Interface for the CLaneFlowComposition class — per-lane flow composition over the day.
 */

#pragma once
#include "ModelData.h"

/**
 * @brief Per-lane flow composition over a daily cycle of time blocks.
 *
 * CLaneFlowComposition holds, for one lane, the per-block total flow,
 * truck flow, car percentage, speed distribution, and class-composition
 * matrix. It is loaded from the lane-flow CSV file by
 * @ref CLaneFlowData and passed to @ref CLaneGenTraffic and the model
 * data classes to parametrise one lane's traffic stream.
 *
 * Blocks are typically hours (3600 s), and there are typically 24 per
 * daily cycle. The class carries both the block size and count so the
 * downstream generators can compute absolute times from block indices.
 *
 * @see CLaneFlowData
 * @see CLaneGenTraffic
 */
class CLaneFlowComposition
{
public:
	/**
	 * @brief Construct a composition for one lane.
	 *
	 * @param[in] lane      Global zero-based lane index.
	 * @param[in] dirn      Direction of travel (1 or 2).
	 * @param[in] blockSize Block size in seconds (typically 3600).
	 */
	CLaneFlowComposition(size_t lane, size_t dirn, size_t blockSize);
	~CLaneFlowComposition();

	/// @brief Get the global zero-based lane index.
	size_t getGlobalLaneNo() const { return m_GlobalLaneNo; };

	/// @brief Get the direction of travel (1 or 2).
	size_t getDirn() const { return m_Dirn; };

	/// @brief Get the per-block class-composition matrix.
	matrix getComposition() const { return m_mComposition; };

	/// @brief Get the per-block car percentage vector.
	vec getCarPercent() const { return m_vCarP; };

	/// @brief Get the per-block total flow rates (veh/h).
	vec getTotalFlow() const { return m_vTotalFlow; };

	/// @brief Get the per-block truck flow rates (truck/h).
	vec getTruckFlow() const { return m_vTruckFlow; };

	/// @brief Get the per-block speed distribution (Normal mean/stddev).
	std::vector<Normal> getSpeed() const { return m_vSpeed; };

	/// @brief Get the number of blocks in the daily cycle.
	size_t getBlockCount() const { return m_NoBlocks; };

	/// @brief Get the block size in seconds.
	size_t getBlockSize() const { return m_BlockSize; };

	/// @brief Get the number of truck classes represented.
	size_t getTruckClassCount() const { return m_TruckClassCount; };

	/**
	 * @brief Append one block's data row.
	 *
	 * Each row contains: total flow, truck flow, car %, speed mean,
	 * speed std, and one class percentage per truck class.
	 *
	 * @param[in] vData Flattened block data vector.
	 */
	void addBlockData(vec vData);

	/// @brief Derive secondary fields (e.g. composition matrix) after all blocks have been added.
	void completeData();

private:
	matrix m_mComposition;            ///< Per-block truck class composition matrix.
	vec m_vCarP;                      ///< Per-block car percentage.
	vec m_vTotalFlow;                 ///< Per-block total flow (veh/h).
	vec m_vTruckFlow;                 ///< Per-block truck flow (truck/h).
	std::vector<Normal> m_vSpeed;     ///< Per-block speed distribution.

	size_t m_GlobalLaneNo;            ///< Global zero-based lane index.
	size_t m_Dirn;                    ///< Direction (1 or 2).
	size_t m_BlockSize;               ///< Block size in seconds (typically 3600).
	size_t m_NoBlocks;                ///< Number of blocks in one cycle (typically 24).
	size_t m_TruckClassCount;         ///< Number of truck classes.
};
