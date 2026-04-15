/**
 * @file Event.h
 * @brief Interface for the CEvent class — a recorded load-effect event.
 */

#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <sstream>
#include "Effect.h"
#include "ConfigData.h"

/**
 * @brief A recorded load-effect event on a bridge, spanning one vehicle
 *        configuration.
 *
 * A CEvent is opened when a new vehicle enters the bridge (or an existing
 * one leaves) and closed when the next change happens. While open, the
 * event tracks the running maximum and minimum of every load effect. On
 * close it is handed to the output managers (@ref CPOTManager,
 * @ref CBlockMaxManager, etc.) via the @ref CEventManager.
 *
 * The maximum and minimum are stored as per-effect vectors of
 * @ref CEffect, one slot per load effect configured for the bridge.
 * Events are orderable by their load-effect value for sorting in the
 * output buffers.
 *
 * @see CEffect
 * @see CEventManager
 */
class CEvent
{
public:
	/**
	 * @brief Construct an event with the given output file format.
	 * @param[in] fileFormat File format tag used when writing this event.
	 */
	CEvent(size_t fileFormat);

	/**
	 * @brief Construct an event with a file format and ID.
	 * @param[in] fileFormat File format tag used when writing this event.
	 * @param[in] ID         Event identifier (monotonically increasing).
	 */
	CEvent(size_t fileFormat, size_t ID);

	/**
	 * @brief Construct an event with a file format, ID and effect count.
	 * @param[in] fileFormat File format tag used when writing this event.
	 * @param[in] ID         Event identifier.
	 * @param[in] noEffects  Number of load effects tracked by this event.
	 */
	CEvent(size_t fileFormat, size_t ID, size_t noEffects);

	virtual ~CEvent();

	/**
	 * @brief Order events by the maximum value of their first load effect.
	 *
	 * Used when sorting buffers of events for output writing.
	 */
	bool operator<(const CEvent& x);

	/**
	 * @brief Reset the event to an empty state with default values.
	 *
	 * Zeros the running maxima/minima and clears the current effect index
	 * so the event can be reused.
	 */
	void reset();

	/**
	 * @brief Get the running maximum for load effect @p effNo.
	 * @param[in] effNo Zero-based load-effect index.
	 * @return Reference to the maximum @ref CEffect observed so far.
	 */
	CEffect&	getMaxEffect(size_t effNo);

	/**
	 * @brief Get the running minimum for load effect @p effNo.
	 * @param[in] effNo Zero-based load-effect index.
	 * @return Reference to the minimum @ref CEffect observed so far.
	 */
	CEffect&	getMinEffect(size_t effNo);

	/// @brief Get the number of vehicles that contributed to this event.
	size_t		getNoVehicles();

	/// @brief Get the number of trucks (non-car vehicles) in this event.
	size_t		getNoTrucks() const;

	/// @brief Get the event identifier.
	size_t		getID();

	/// @brief Get the time at which the overall maximum effect was recorded, in seconds.
	double		getMaxTime();

	/// @brief Get the time at which the current effect's maximum was recorded, in seconds.
	double		getMaxEffectTime();

	/// @brief Get the number of load effects tracked.
	size_t		getNoEffects();

	/// @brief Get the event start time in seconds.
	double		getStartTime();

	/// @brief Get the event start time formatted as "DD/MM/YYYY HH:MM:SS.ss".
	std::string getTimeStr();

	/// @brief Set the event start time in seconds.
	void		setStartTime(double StartTime);

	/// @brief Set the event identifier.
	void		setID(size_t id);

	/**
	 * @brief Replace the maximum effect for load effect @p i.
	 * @param[in] Eff New maximum.
	 * @param[in] i   Zero-based load-effect index.
	 */
	void		setMaxEffect(CEffect Eff, size_t i);

	/**
	 * @brief Replace the minimum effect for load effect @p i.
	 * @param[in] Eff New minimum.
	 * @param[in] i   Zero-based load-effect index.
	 */
	void		setMinEffect(CEffect Eff, size_t i);

	/**
	 * @brief Resize the per-effect max/min vectors to @p noEffects.
	 * @param[in] noEffects Number of load effects to track.
	 */
	void		setNoEffects(size_t noEffects);

	/// @brief Set the current load-effect index used by getMaxEffectTime().
	void		setCurEffect(size_t ce);

	/**
	 * @brief Write one load effect to an output file.
	 *
	 * @param[in] k      Zero-based load-effect index.
	 * @param[in] file   Path to the output file.
	 * @param[in] trucks If true, include per-truck detail.
	 */
	void		writeEffect(size_t k, std::string file, bool trucks);

	/**
	 * @brief Write the full event to an output file in the configured format.
	 * @param[in] file Path to the output file.
	 */
	void		writeToFile(std::string file);

	/**
	 * @brief Add a single effect as both the max and min for its slot.
	 *
	 * Helper used when reconstructing events from disk or when the
	 * event only has a single observation.
	 *
	 * @param[in] effect Effect to store.
	 */
	void		AddSingleEffect(CEffect effect);

	std::vector<CEffect> m_vMaxEffects;  ///< Running maxima, one per load effect.
	std::vector<CEffect> m_vMinEffects;  ///< Running minima, one per load effect.

private:
	void setDefaults();
	template <typename T> std::string to_string(T const& value);

	size_t	m_CurEffect;    ///< Index of the load effect currently being queried.
	size_t	m_NoEffects;    ///< Number of load effects tracked.
	size_t	m_EventID;      ///< Monotonically increasing event identifier.
	double	m_StartTime;    ///< Event start time in seconds.

	size_t	DAYS_PER_MT;    ///< Days per month (from CConfigData::Time).
	size_t	MTS_PER_YR;     ///< Months per year (from CConfigData::Time).
	size_t	HOURS_PER_DAY;  ///< Hours per day (from CConfigData::Time).
	size_t	SECS_PER_HOUR;  ///< Seconds per hour (from CConfigData::Time).
	size_t	MINS_PER_HOUR;  ///< Minutes per hour (from CConfigData::Time).
	size_t	SECS_PER_MIN;   ///< Seconds per minute (from CConfigData::Time).

	size_t	FILE_FORMAT;    ///< File format tag used by writeToFile() and writeEffect().
};
