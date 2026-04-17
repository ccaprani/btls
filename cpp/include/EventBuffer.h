/**
 * @file EventBuffer.h
 * @brief Interface for the CEventBuffer class — buffered writer for CEvent streams.
 */

#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include "Event.h"
#include "Effect.h"
#include "Vehicle.h"
#include "ConfigData.h"

/**
 * @brief Buffered writer for a stream of @ref CEvent records.
 *
 * CEventBuffer accumulates completed events and flushes them to an output
 * file in batches, amortising file I/O cost across many events. It runs
 * in one of two modes selected at runtime:
 *
 * - **ALLEVENTS**: every completed event is serialised to the output
 *   file using its configured file format.
 * - **FATIGUE**: only events flagged by @ref CEventManager for fatigue
 *   analysis are written, in a format consumed by the fatigue rainflow
 *   post-processor.
 *
 * @see CEventManager
 * @see CEvent
 */
class CEventBuffer
{
public:
	/**
	 * @brief Construct a buffer with the given capacity.
	 * @param[in] bufferSize Number of events to hold before auto-flushing.
	 */
	CEventBuffer(size_t bufferSize);
	virtual ~CEventBuffer();

	/**
	 * @brief Select the output format mode.
	 * @param[in] bFatigue If true, use FATIGUE mode; otherwise ALLEVENTS.
	 */
	void setMode(bool bFatigue);

	/**
	 * @brief Flush all buffered events to the output file now.
	 *
	 * Called at end-of-simulation and whenever the buffer fills.
	 */
	void FlushBuffer();

	/**
	 * @brief Change the buffer capacity.
	 * @param[in] size New capacity in events.
	 */
	void SetBufferSize(int size);

	/**
	 * @brief Append a completed event to the buffer.
	 *
	 * Triggers an automatic FlushBuffer() when the buffer is full.
	 *
	 * @param[in] Ev Completed event (copied into the buffer).
	 */
	void AddEvent(CEvent Ev);

	/**
	 * @brief Open the output file using the default stemmed filename.
	 * @param[in] BridgeLength Bridge length in metres (used in the filename stem).
	 */
	void setOutFile(double BridgeLength);

	/**
	 * @brief Open a specific output file.
	 * @param[in] OutFile Path to the output file.
	 */
	void setOutFile(std::string OutFile);

private:
	/// @brief Flush helper: write every event in the buffer (ALLEVENTS mode).
	void FlushAllEventsBuff();

	/// @brief Flush helper: write fatigue-flagged events only (FATIGUE mode).
	void FlushFatigueBuff();

	enum Mode {ALLEVENTS, FATIGUE};
	Mode m_Mode;                       ///< Active output mode.
	std::vector<CEvent> m_vEvents;     ///< Buffered events awaiting flush.
	std::ofstream m_OutFile;           ///< Output file stream.
	size_t m_BufferSize;               ///< Buffer capacity in events.
	size_t m_NoEvents;                 ///< Running count of events flushed to disk.
	double m_BridgeLength;             ///< Bridge length in metres (used in filename stem).
	template <typename T> std::string to_string(T const& value);
};
