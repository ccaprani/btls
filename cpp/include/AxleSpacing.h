/**
 * @file AxleSpacing.h
 * @brief Interface for the CAxleSpacing class — axle-spacing distribution tables.
 */

#if !defined(AFX_AXLESPACING_H__429CA222_93DF_47F1_A601_251A90BD7355__INCLUDED_)
#define AFX_AXLESPACING_H__429CA222_93DF_47F1_A601_251A90BD7355__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MultiModalNormal.h"

/**
 * @brief Storage for axle-spacing distributions indexed by number of axles.
 *
 * CAxleSpacing holds @ref CMultiModalNormal distributions for the axle
 * spacings of 2-, 3-, 4-, and 5-axle trucks. Each call to
 * GetSpacingDist() returns the distribution for one spacing slot
 * (e.g. the 3rd spacing of a 5-axle truck), from which the vehicle
 * generator samples.
 *
 * The four Add*AxleData() helpers load the per-bucket distribution
 * sets at configuration time. Distributions are multi-modal normals
 * because observed WIM axle-spacing histograms are often bimodal or
 * trimodal (distinct vehicle sub-populations).
 *
 * @see CVehicleGenGrave
 * @see CMultiModalNormal
 */
class CAxleSpacing
{
public:
	CAxleSpacing();
	virtual ~CAxleSpacing();

	/**
	 * @brief Get the spacing distribution for one axle slot of one truck type.
	 * @param[in] iTruck Zero-based truck-axle-count bucket (0 = 2-axle, 1 = 3-axle, etc.).
	 * @param[in] iSpace Zero-based spacing slot (0 = between axles 1 and 2, and so on).
	 * @return The multi-modal normal distribution for that slot.
	 */
	CMultiModalNormal GetSpacingDist(int iTruck, int iSpace);

	/// @brief Load the 2-axle-truck spacing distributions.
	void Add2AxleData(std::vector<CMultiModalNormal> vSpace);

	/// @brief Load the 3-axle-truck spacing distributions.
	void Add3AxleData(std::vector<CMultiModalNormal> vSpace);

	/// @brief Load the 4-axle-truck spacing distributions.
	void Add4AxleData(std::vector<CMultiModalNormal> vSpace);

	/// @brief Load the 5-axle-truck spacing distributions.
	void Add5AxleData(std::vector<CMultiModalNormal> vSpace);

private:
	std::vector<CMultiModalNormal> m_v2AxleData;  ///< Spacing distributions for 2-axle trucks.
	std::vector<CMultiModalNormal> m_v3AxleData;  ///< Spacing distributions for 3-axle trucks.
	std::vector<CMultiModalNormal> m_v4AxleData;  ///< Spacing distributions for 4-axle trucks.
	std::vector<CMultiModalNormal> m_v5AxleData;  ///< Spacing distributions for 5-axle trucks.
};

#endif // !defined(AFX_AXLESPACING_H__429CA222_93DF_47F1_A601_251A90BD7355__INCLUDED_)
