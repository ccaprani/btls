/**
 * @file BlockMaxManager.h
 * @brief Interface for the CBlockMaxManager class — block-maxima event writer.
 */

#pragma once

#include "OutputManagerBase.h"
#include "BlockMaxEvent.h"

/**
 * @brief Records block maxima of each load effect over fixed time blocks.
 *
 * CBlockMaxManager splits the simulation timeline into fixed-length
 * blocks (configured by @c BLOCK_SIZE_DAYS, derived to seconds via
 * @c BLOCK_SIZE_SECS) and, within each block, tracks the maximum
 * @ref CEvent seen so far for each vehicle-count bucket. The
 * per-vehicle-count resolution is stored in a @ref CBlockMaxEvent
 * container.
 *
 * When a block boundary is crossed, the completed block maxima are
 * flushed via the inherited COutputManagerBase file machinery. A
 * "mixed" output stream that combines maxima across vehicle counts
 * is optionally maintained when @c WRITE_BM_MIXED is enabled.
 *
 * @see CBlockMaxEvent
 * @see COutputManagerBase
 * @see CEventManager
 */
class CBlockMaxManager : public COutputManagerBase
{
public:
	/**
	 * @brief Construct a block-maxima manager bound to the shared configuration.
	 * @param[in] config Shared configuration block.
	 */
	CBlockMaxManager(CConfigDataCore& config);
	virtual ~CBlockMaxManager();

	/**
	 * @brief Consume one completed event and update the running block maxima.
	 *
	 * Dispatches @p Ev to the matching vehicle-count bucket and, if the
	 * current block boundary has been crossed, flushes the completed
	 * block before starting a new one.
	 *
	 * @param[in] Ev Completed event.
	 */
	virtual void Update(CEvent Ev);

	/**
	 * @brief Initialize bucket storage and output files.
	 *
	 * @param[in] BridgeLength Bridge length in metres.
	 * @param[in] nLE          Number of load effects tracked.
	 * @param[in] SimStartTime Simulation start time in seconds.
	 */
	virtual void Initialize(double BridgeLength, size_t nLE, double SimStartTime);

private:
	/// @brief Write the per-vehicle event files.
	virtual void WriteVehicleFiles();

	/// @brief Write the per-load-effect summary files.
	virtual void WriteSummaryFiles();

	/// @brief Flush completed blocks to disk if the buffer has filled.
	virtual void CheckBuffer(bool bForceOutput);

	/// @brief Open the per-load-effect vehicle output files.
	virtual void OpenVehicleFiles();

	/// @brief Flush the current block-max buffer to disk.
	virtual void WriteBuffer();

	/// @brief Grow the vehicle-count bucket list to accommodate larger events.
	void	AddExtraEvents();

	/// @brief Update the mixed-events stream with @p Ev.
	void	UpdateMixedEvents(CEvent Ev);

	CBlockMaxEvent				m_BlockMaxEvent;   ///< Running block max indexed by vehicle count.
	CEvent						m_BMMixedEvent;    ///< Running block max across all vehicle counts (mixed stream).
	std::vector<CBlockMaxEvent>	m_vBMEventsBuffer; ///< Buffer of completed blocks awaiting flush.
	std::vector<CEvent>			m_vMixedEvents;    ///< Buffer of completed mixed-stream events awaiting flush.

	std::string m_MixedEventFile;                  ///< Output filename for the mixed-events stream.

	size_t m_MaxEvTypesForBridge;                  ///< Maximum vehicle-count bucket observed on this bridge.
	size_t m_BlockSize;                            ///< Block size in seconds.
	size_t m_CurBlockNo;                           ///< Zero-based index of the current time block.
	size_t m_CurEventNoVehicles;                   ///< Vehicle count of the event being processed.

	size_t	BLOCK_SIZE_SECS;                       ///< Block size in seconds (derived).
	size_t	BLOCK_SIZE_DAYS;                       ///< Block size in days (from config).
	bool	WRITE_BM_MIXED;                        ///< Enable the mixed-stream output.

	size_t FILE_FORMAT;                            ///< Output file format tag.
};
