/**
 * @file AxleWeight45.h
 * @brief Interface for the CAxleWeight45 class — axle-weight distributions for 4- and 5-axle trucks.
 */

#if !defined(AFX_AXLEWEIGHT45_H__CB51C460_9A57_493D_9649_FA33D6F9805E__INCLUDED_)
#define AFX_AXLEWEIGHT45_H__CB51C460_9A57_493D_9649_FA33D6F9805E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>

/**
 * @brief Storage for GVW-banded axle-weight distributions of 4- and
 *        5-axle trucks.
 *
 * Unlike @ref CAxleWeight23, which holds per-axle multi-modal normal
 * distributions, CAxleWeight45 stores GVW-conditioned weight bands:
 * for each GVW range of each truck type, the mean and standard
 * deviation of the front axle (W1), the rear axle group (W2), and the
 * total weight (WT) are recorded. This lets the Grave model sample
 * individual axle weights conditional on the overall truck weight.
 *
 * The Dist struct holds (mean, standard deviation) for one weight slot;
 * the GVWRange struct bundles three Dists (W1, W2, WT) for one GVW band.
 *
 * @see CVehicleGenGrave
 * @see CAxleWeight23
 */
class CAxleWeight45
{
public:
	/**
	 * @brief Get the GVW-range distribution parameters for one truck and range.
	 *
	 * Returns a flattened vector of (mean, std-dev) pairs for W1, W2,
	 * and WT in the requested range.
	 *
	 * @param[in] iTruck Zero-based truck-axle-count bucket (0 = 4-axle, 1 = 5-axle).
	 * @param[in] iRange Zero-based GVW band index.
	 * @return Flattened vector of distribution parameters.
	 */
	std::vector<double> GetGVWRange(int iTruck, int iRange);

	/**
	 * @brief Load the distribution parameters for one GVW band.
	 * @param[in] data   Flattened (mean, std-dev) vector.
	 * @param[in] iTruck Truck bucket (0 = 4-axle, 1 = 5-axle).
	 * @param[in] iRange GVW band index.
	 */
	void AddGVWRange(std::vector<double> data, int iTruck, int iRange);

	CAxleWeight45();
	virtual ~CAxleWeight45();

private:
	/// @brief A single distribution slot: mean and standard deviation.
	struct Dist
	{
		double Mean;
		double StdDev;
	};

	/**
	 * @brief Weight parameters for one GVW band: front axle W1, rear axle group W2, and total WT.
	 */
	struct GVWRange
	{
		Dist W1;  ///< Front axle weight distribution.
		Dist W2;  ///< Rear axle group weight distribution.
		Dist WT;  ///< Total weight distribution.
	};

	std::vector<GVWRange> m_v4AxleTrucks;  ///< GVW bands for 4-axle trucks.
	std::vector<GVWRange> m_v5AxleTrucks;  ///< GVW bands for 5-axle trucks.
};

#endif // !defined(AFX_AXLEWEIGHT45_H__CB51C460_9A57_493D_9649_FA33D6F9805E__INCLUDED_)
