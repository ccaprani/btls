/**
 * @file EventStatistics.h
 * @brief Interface for the CEventStatistics class — running moments of a load effect.
 */

#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <math.h>
#include "Event.h"
#include "Effect.h"
#include "Vehicle.h"

/**
 * @brief Running statistics (mean, variance, skewness, kurtosis) of one
 *        load effect, accumulated over completed events.
 *
 * CEventStatistics uses an online moment accumulator so that the full
 * set of central moments can be updated one event at a time without
 * storing the entire event history. Each call to update() folds one
 * event's max into the running moments; outputString() returns the
 * formatted summary row for the output file.
 *
 * In addition to the load-effect moments, the class tracks composition
 * information — total number of vehicles and number of trucks seen, and
 * a histogram of the number of trucks per event in @c m_vNoTrucksInEvent.
 *
 * @see CStatsManager
 */
class CEventStatistics
{
public:
	CEventStatistics(void);
	virtual ~CEventStatistics(void);

	/**
	 * @brief Fold one event's maximum for load effect @p iLE into the running moments.
	 *
	 * @param[in] Ev  Completed event.
	 * @param[in] iLE Zero-based load-effect index.
	 */
	void update(CEvent Ev, unsigned int iLE);

	/**
	 * @brief Format the current statistics as a single output row.
	 * @return Tab/space-delimited row of moments and composition counts.
	 */
	std::string outputString();

	/**
	 * @brief Get the column headings matching outputString().
	 * @return Row of column headings.
	 */
	std::string headingsString();

	size_t m_N;         ///< Number of events folded into the moments.
	size_t m_ID;        ///< Identifier (typically the load-effect index).

	// load effect related statistics
	double m_Max;       ///< Running maximum load-effect value seen.
	double m_Min;       ///< Running minimum load-effect value seen.
	double m_Mean;      ///< Running mean.
	double m_Variance;  ///< Running sample variance.
	double m_StdDev;    ///< Running sample standard deviation.
	double m_Skewness;  ///< Running sample skewness (third standardised moment).
	double m_Kurtosis;  ///< Running sample kurtosis (fourth standardised moment).
	double m_M2;        ///< Accumulator for second central moment (online algorithm).
	double m_M3;        ///< Accumulator for third central moment (online algorithm).
	double m_M4;        ///< Accumulator for fourth central moment (online algorithm).

	// Event composition
	size_t m_NoVehicles;///< Total vehicles seen across all folded events.
	size_t m_NoTrucks;  ///< Total trucks (non-car vehicles) seen across all folded events.

	std::vector<size_t> m_vNoTrucksInEvent;  ///< Histogram of the number of trucks per event.

private:
	/// @brief Update the online moment accumulators with a single value.
	void accumulator(double x);

	/// @brief Derive mean/variance/skewness/kurtosis from the moment accumulators.
	void finalize();

	size_t m_MaxNoTrucksInEvent;  ///< Largest number of trucks observed in any single event.
};
