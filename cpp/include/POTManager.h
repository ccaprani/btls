/**
 * @file POTManager.h
 * @brief Interface for the CPOTManager class — peaks-over-threshold event writer.
 */

#pragma once

#include "OutputManagerBase.h"

/**
 * @brief Records peaks-over-threshold (POT) events for each load effect.
 *
 * CPOTManager inspects every completed @ref CEvent and, for each load
 * effect, retains the event if its peak value exceeds the configured
 * threshold for that effect. Events above threshold are buffered and
 * written in batches via the inherited COutputManagerBase file machinery.
 *
 * In addition to the event stream, the manager optionally maintains a
 * counter file: a running count of how many peaks exceeded each
 * threshold in each time block. Block size is configured via
 * @c POT_COUNT_SIZE_DAYS / @c POT_COUNT_SIZE_SECS.
 *
 * Output files are named using the filestem from the base class and a
 * suffix derived from the bridge length and load-effect index.
 *
 * @see COutputManagerBase
 * @see CEventManager
 */
class CPOTManager : public COutputManagerBase
{
public:
	/**
	 * @brief Construct a POT manager bound to the shared configuration.
	 * @param[in] config Shared configuration block.
	 */
	CPOTManager(CConfigDataCore& config);
	virtual ~CPOTManager(void);

	/**
	 * @brief Consume one completed event and record it if any load effect
	 *        exceeds its threshold.
	 *
	 * @param[in] Ev Completed event.
	 */
	virtual void Update(CEvent Ev);

	/**
	 * @brief Initialize thresholds, buffers and output files.
	 *
	 * @param[in] BridgeLength Bridge length in metres.
	 * @param[in] vThreshold   Peak thresholds, one per load effect.
	 * @param[in] SimStartTime Simulation start time in seconds.
	 */
	virtual void Initialize(double BridgeLength, std::vector<double> vThreshold, double SimStartTime);

private:
	/// @brief Write the per-vehicle event files.
	virtual void	WriteVehicleFiles();

	/// @brief Write per-load-effect summary files.
	virtual void	WriteSummaryFiles();

	/// @brief Flush the event buffer if full, or on @p bForceOutput.
	virtual void	CheckBuffer(bool bForceOutput);

	/// @brief Open the per-load-effect vehicle output files.
	virtual void	OpenVehicleFiles();

	/// @brief Flush the current buffer contents to disk.
	virtual void	WriteBuffer();

	/// @brief Open the optional block counter file.
	void OpenCounterFile();

	/// @brief Write the accumulated per-block counts to the counter file.
	void WriteCounter();

	/// @brief Increment the counter for the current block.
	void UpdateCounter();

	size_t m_NoPeaks;                                 ///< Running count of peaks written across all load effects.
	std::vector< std::vector<CEvent> >	m_vEvents;    ///< Buffered events per load effect.
	std::string m_EventFile;                          ///< Current event output filename.
	std::vector<double> m_vThreshold;                 ///< Peak thresholds, one per load effect.
	std::string m_CounterFile;                        ///< Block counter output filename.

	unsigned int m_BlockSize;                         ///< Block size in seconds.
	unsigned int m_CurBlockNo;                        ///< Zero-based index of the current time block.
	std::vector< std::vector<unsigned int> > m_vCounter;  ///< Per-load-effect, per-block exceedance counts.

	bool WRITE_POT_COUNTER;                           ///< Enable the counter file output.
	int  POT_COUNT_SIZE_DAYS;                         ///< Counter block size in days (from config).
	int  POT_COUNT_SIZE_SECS;                         ///< Counter block size in seconds (derived).
};
