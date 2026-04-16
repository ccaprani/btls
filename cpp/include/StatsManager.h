/**
 * @file StatsManager.h
 * @brief Interface for the CStatsManager class — running-statistics output manager.
 */

#pragma once

#include "OutputManagerBase.h"
#include "EventStatistics.h"

/**
 * @brief Accumulates running statistics (mean, variance, skewness,
 *        kurtosis) of load effects and writes interval/cumulative
 *        summary files.
 *
 * CStatsManager receives completed events from @ref CEventManager,
 * folds each event's maximum into a @ref CEventStatistics accumulator,
 * and periodically flushes the accumulated moments to disk. Two output
 * modes are supported:
 *
 * - **Cumulative**: one set of statistics over the full simulation,
 *   written at the end (controlled by @c WRITE_SS_CUMULATIVE).
 * - **Interval**: statistics reset every @c WRITE_SS_INTERVAL_SIZE
 *   seconds, each interval's summary written as a row in the output
 *   (controlled by @c WRITE_SS_INTERVALS).
 *
 * Derives from @ref COutputManagerBase for the shared file-handling
 * plumbing.
 *
 * @see CEventStatistics
 * @see COutputManagerBase
 * @see CEventManager
 */
class CStatsManager : public COutputManagerBase
{
public:
	CStatsManager(CConfigDataCore& config);
	virtual ~CStatsManager(void);

	/**
	 * @brief Consume one completed event and fold its max into the accumulators.
	 * @param[in] Ev Completed event.
	 */
	virtual void Update(CEvent Ev);

	/**
	 * @brief Initialize with bridge length, load-effect count, and simulation start time.
	 */
	virtual void Initialize(double BridgeLength, size_t nLE, double SimStartTime);

private:
	/// @brief Write per-load-effect summary files at end-of-simulation.
	virtual void WriteSummaryFiles();

	/// @brief Flush the interval buffer if it has reached capacity.
	virtual void CheckBuffer(bool bForceOutput);

	/// @brief Write buffered interval rows to disk.
	virtual void WriteBuffer();

	/// @brief Write the cumulative statistics file.
	void WriteCumulativeFile();

	/// @brief Write the interval-file column headings.
	void WriteIntervalHeadings();

	/// @brief Fold one value into the accumulator for load effect @p i.
	void accumulateLE(unsigned int i, double x);

	std::vector<CEventStatistics> m_vIntervalStats;                  ///< Per-LE interval accumulators.
	std::vector<CEventStatistics> m_vCumulativeStats;                ///< Per-LE cumulative accumulators.
	std::vector< std::vector<CEventStatistics> > m_vIntStatsBuffer;  ///< Buffered interval rows.

	double m_CurTime;                  ///< Current simulation time (for interval boundary checks).
	unsigned int m_CurIntervalNo;      ///< Current interval index.

	unsigned int WRITE_SS_INTERVAL_SIZE;  ///< Interval size in seconds.
	bool WRITE_SS_INTERVALS;             ///< Enable interval-based output.
	bool WRITE_SS_CUMULATIVE;            ///< Enable cumulative output.
};
