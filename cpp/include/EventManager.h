/**
 * @file EventManager.h
 * @brief Interface for the CEventManager class — records load-effect events for one bridge.
 */

#if !defined(AFX_EVENTMANAGER_H__3ED9F26C_A94D_4EA8_A87C_4DB2819160E5__INCLUDED_)
#define AFX_EVENTMANAGER_H__3ED9F26C_A94D_4EA8_A87C_4DB2819160E5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <math.h>
#include "Event.h"
#include "Effect.h"
#include "Vehicle.h"
#include "EventBuffer.h"
#include "BlockMaxManager.h"
#include "POTManager.h"
#include "StatsManager.h"
#include "FatigueManager.h"

/**
 * @brief Records load-effect events for one bridge and dispatches them to
 *        the output managers.
 *
 * CEventManager sits between the simulation inner loop and the various
 * output writers. @ref CBridge::Update drives it through three calls:
 *
 * 1. AddNewEvent() at the start of a new vehicle configuration, passing
 *    the current set of vehicles on the bridge.
 * 2. UpdateEffects() at every inner-loop time step, passing the current
 *    load-effect values and the lead vehicle position. This tracks the
 *    running maximum and minimum per load effect.
 * 3. EndEvent() when a vehicle enters or leaves the bridge, closing the
 *    current event and handing the completed CEvent to the output
 *    managers (POT, block maxima, running statistics, fatigue rainflow,
 *    all-events buffer).
 *
 * The four @c WRITE_* flags and @c DO_FATIGUE_RAINFLOW flag control which
 * outputs are actually produced; they are read from the configuration
 * passed to the constructor.
 *
 * @see CBridge::Update
 * @see CEvent
 * @see CPOTManager
 * @see CBlockMaxManager
 * @see CStatsManager
 * @see CFatigueManager
 */
class CEventManager
{
public:
	/**
	 * @brief Construct an event manager bound to a configuration.
	 *
	 * Does not initialize output files — Initialize() must be called
	 * after the bridge length, thresholds and start time are known.
	 *
	 * @param[in] config Shared configuration block.
	 */
	CEventManager(CConfigDataCore& config);
	virtual ~CEventManager();

	/**
	 * @brief Initialize output managers and buffers.
	 *
	 * Opens output files for each enabled writer (all-events, fatigue,
	 * block maxima, POT, statistics) and prepares the per-load-effect
	 * state.
	 *
	 * @param[in] BridgeLength Bridge length in metres (used to stem output filenames).
	 * @param[in] vThresholds  Peak-recording thresholds, one per load effect.
	 * @param[in] SimStartTime Simulation start time in seconds.
	 */
	void Initialize(double BridgeLength, std::vector<double> vThresholds, double SimStartTime);

	/**
	 * @brief Close the current event and dispatch it to the output managers.
	 *
	 * Called when the vehicle configuration on the bridge changes (a
	 * vehicle enters or leaves). Hands the completed @ref CEvent to the
	 * POT, block-max, statistics and fatigue managers, and appends it to
	 * the all-events buffer.
	 */
	void EndEvent();

	/**
	 * @brief Flush all output buffers and close output files.
	 *
	 * Call once at the end of the simulation to ensure any partial
	 * buffers are written.
	 */
	void Finish();

	/**
	 * @brief Update the running maxima for the current event.
	 *
	 * Called once per inner-loop time step by @ref CBridge::Update. For
	 * each load effect, compares @p vEffs[i] against the current
	 * maximum/minimum and updates if larger/smaller. Optionally writes
	 * a row to the time-history output file.
	 *
	 * @param[in] vEffs    Current load-effect values, one per load effect.
	 * @param[in] position Position of the lead vehicle in metres.
	 * @param[in] time     Current simulation time in seconds.
	 */
	void UpdateEffects(std::vector<double> vEffs, double position, double time);

	/**
	 * @brief Begin a new event with a copy of the current vehicles.
	 * @param[in] vVehs    Vehicles currently on the bridge (value copies).
	 * @param[in] curTime  Current simulation time in seconds.
	 */
	void AddNewEvent(std::vector<CVehicle> vVehs, double curTime);

	/**
	 * @brief Begin a new event with shared-pointer vehicles.
	 *
	 * Overload used by @ref CBridge::Update, which holds vehicles as
	 * shared pointers across lanes.
	 *
	 * @param[in] vVehs    Vehicles currently on the bridge.
	 * @param[in] curTime  Current simulation time in seconds.
	 */
	void AddNewEvent(const std::vector<CVehicle_sp> vVehs, double curTime);

	/**
	 * @brief Open the event output file, stemming the filename by bridge length.
	 * @param[in] BridgeLength Bridge length in metres.
	 */
	void setEventOutputFile(double BridgeLength);

private:
	/// @brief Flush the all-events buffer to disk when the threshold size is reached.
	void WriteEventBuffer();

	CConfigDataCore&	m_Config;            ///< Reference to the shared configuration block.

	CEventBuffer		m_AllEventBuffer;    ///< Buffer of every completed event, flushed in batches.
	CEventBuffer		m_FatigueEventBuffer;///< Buffer of events flagged for fatigue analysis.
	CBlockMaxManager	m_BlockMaxManager;   ///< Block-maxima writer, one maximum per time block per load effect.
	CPOTManager			m_POTManager;        ///< Peaks-over-threshold writer.
	CStatsManager		m_StatsManager;      ///< Running statistics (mean, variance, etc.) writer.
	CFatigueManager		m_FatigueManager;    ///< Fatigue damage accumulator (rainflow counting).

	std::vector<CVehicle> m_vVehicles;       ///< Vehicles contributing to the current event.

	std::ofstream		m_TimeHistoryFile;   ///< Optional per-timestep time-history output stream.
	CEvent				m_CurEvent;          ///< Event currently being accumulated.
	long				m_BlockSize;         ///< Block size in seconds for block-maxima output.
	int					m_MaxNoVehsInEvent;  ///< Largest number of vehicles observed in any completed event.
	int					m_CurEventType;      ///< Dispatch type for the current event (used by multi-type outputs).
	int					m_NoEvents;          ///< Running count of completed events since Initialize().
	int					m_CurBlockNo;        ///< Zero-based index of the current time block.
	double				m_CurTime;           ///< Most recent simulation time passed to UpdateEffects().
	double				m_BridgeLength;      ///< Bridge length in metres (copied from Initialize).
	size_t				m_NoLoadEffects;     ///< Number of load effects tracked.

	/// @brief Update @ref m_CurEvent with the per-timestep maxima computed in UpdateEffects().
	void	UpdateEvent();

	/// @brief Append one row to the time-history output stream for load effect @p i.
	void	DoTimeHistory(int i, std::vector<double>& vEff);

	std::vector<double> m_vThresholds;       ///< Peak-recording thresholds, one per load effect.

	bool	WRITE_TIME_HISTORY;              ///< If true, a time-history row is written every inner-loop step.
	bool	WRITE_EACH_EVENT;                ///< If true, every completed event is written (not just peaks).
	int		WRITE_EVENT_BUFFER_SIZE;         ///< Number of events buffered before flushing to disk.
	bool	WRITE_FATIGUE_EVENT;             ///< If true, completed events feed the fatigue buffer.

	bool	WRITE_BM;                        ///< Enable block-maxima output.
	bool	WRITE_POT;                       ///< Enable peaks-over-threshold output.
	bool	WRITE_STATS;                     ///< Enable running-statistics output.
	bool	DO_FATIGUE_RAINFLOW;             ///< Enable fatigue rainflow counting.

	template <typename T> std::string to_string(T const& value);
	template <typename T> std::string to_string(T const& value, int nDigits);
};

#endif // !defined(AFX_EVENTMANAGER_H__3ED9F26C_A94D_4EA8_A87C_4DB2819160E5__INCLUDED_)
