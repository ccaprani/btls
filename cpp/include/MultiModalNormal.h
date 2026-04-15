/**
 * @file MultiModalNormal.h
 * @brief Interface for the CMultiModalNormal class — weighted sum of normal modes.
 */

#pragma once
#include <vector>

/**
 * @brief A mixture of one or more normal distributions, each with its
 *        own weight, mean, and standard deviation.
 *
 * CMultiModalNormal stores a weighted sum of @c Mode structs, where
 * each Mode is a (weight, mean, stddev) triple. The mixture is used
 * throughout the traffic model layer to capture observed bi- and
 * tri-modal WIM histograms — e.g. empty vs loaded trucks, or distinct
 * vehicle sub-populations — that a single normal cannot represent.
 *
 * Sampling is performed by @ref CDistribution::GenerateMultiModalNormal,
 * which picks one mode weighted by @c Weight and then draws from the
 * corresponding @c Normal.
 *
 * @see CDistribution::GenerateMultiModalNormal
 * @see CAxleWeight23
 * @see CAxleSpacing
 */
class CMultiModalNormal
{
public:
	/**
	 * @brief Append one mode to the mixture.
	 * @param[in] w Mode weight (should sum to 1 across all modes).
	 * @param[in] m Mode mean.
	 * @param[in] s Mode standard deviation.
	 */
	void AddMode(double w, double m, double s);

	CMultiModalNormal();
	virtual ~CMultiModalNormal();

	/// @brief Get the number of modes in the mixture.
	std::size_t getNoModes() const {return m_vModes.size();};

	/**
	 * @brief One normal mode of the mixture.
	 */
	struct Mode
	{
		double Weight;  ///< Mode weight in the mixture (should sum to 1 across modes).
		double Mean;    ///< Mode mean.
		double StdDev;  ///< Mode standard deviation.
	};

	std::vector<Mode> m_vModes;  ///< Modes making up the mixture.
};
