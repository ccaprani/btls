/**
 * @file Rainflow.h
 * @brief Interface for the CRainflow class — ASTM E1049-85 rainflow cycle counter.
 */

/*
Implements rainflow cycle counting algorythm for CFatigueManager
according to section 5.4.4 in ASTM E1049-85 (2011).
*/
#pragma once

#include <algorithm>
#include <deque>
#include <map>
#include <math.h>
#include <utility>
#include <vector>

/**
 * @brief Rainflow cycle counter implementing ASTM E1049-85 (2011) §5.4.4.
 *
 * CRainflow receives a load-effect time series via processData(),
 * extracts reversals (peaks and troughs), identifies full and half
 * cycles through the standard four-point algorithm, and accumulates
 * cycle counts keyed by stress range in the output map.
 *
 * The output is a map from rounded stress range to accumulated count
 * (full cycles = 1.0, half cycles = 0.5). Rounding precision is set
 * by @c m_Decimal, and cycles below @c m_Cutoff are discarded.
 *
 * @see CFatigueManager
 */
class CRainflow
{
public:
	/// @brief Result of extracting one cycle: range, mean, and count (0.5 or 1.0).
	struct ExtractCycleOut
	{
		double range;  ///< Stress range of the cycle.
		double mean;   ///< Mean stress of the cycle.
		double count;  ///< Cycle count (0.5 for half-cycle, 1.0 for full).
	};

public:
	CRainflow() {};

	/**
	 * @brief Construct with rounding precision and amplitude cut-off.
	 * @param[in] decimal Number of decimal places for range rounding.
	 * @param[in] cutoff  Minimum range below which cycles are discarded.
	 */
	CRainflow(int decimal, double cutoff) : m_Decimal(decimal), m_Cutoff(cutoff) {};
	~CRainflow() {};

	/// @brief Clear the accumulated cycle-count output map.
	void clearRainflowOutput();

	/**
	 * @brief Feed a segment of load-effect time series into the reversal buffer.
	 * @param[in] series Load-effect values over consecutive time steps.
	 */
	void processData(const std::vector<double> &series);

	/**
	 * @brief Run the rainflow algorithm on the accumulated reversals.
	 * @param[in] bIsFinal If true, extract residual half-cycles at the end of the series.
	 */
	void calcCycles(bool bIsFinal);

	/// @brief Get the accumulated rainflow output: map from rounded range to cycle count.
	const std::map<double, double>& getRainflowOutput() const { return m_RainflowOutput; }

private:
	/// @brief Round @p x up to @c m_Decimal decimal places.
	double doRoundUp(double x) const;

	/// @brief Format one cycle extraction into (range, mean, count).
	ExtractCycleOut formatOutput(double point1, double point2, double count) const;

	/// @brief Convert a map to a vector of pairs.
	template <typename T>
	std::vector<std::pair<T, T>> mapToVector(const std::map<T, T> &inputMap) const;

	/// @brief Extract local extrema (reversals) from the raw series.
	std::vector<double> extractReversals(const std::vector<double> &series) const;

	/// @brief Apply the four-point ASTM algorithm to extract cycles from the reversal buffer.
	std::vector<CRainflow::ExtractCycleOut> extractCycles() const;

	/// @brief Aggregate extracted cycles by rounded range.
	std::vector<std::pair<double, double>> countCycles(const std::vector<CRainflow::ExtractCycleOut> &cycles) const;

	std::vector<double> m_vReversals;          ///< Accumulated reversal (extrema) buffer.
	std::map<double, double> m_RainflowOutput; ///< Rounded range → accumulated cycle count.

	int m_Decimal;     ///< Decimal precision for range rounding.
	double m_Cutoff;   ///< Minimum range below which cycles are discarded.
};
