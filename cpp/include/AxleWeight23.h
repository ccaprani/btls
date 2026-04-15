/**
 * @file AxleWeight23.h
 * @brief Interface for the CAxleWeight23 class — axle-weight distributions for 2- and 3-axle trucks.
 */

#if !defined(AFX_AXLEWEIGHT23_H__C9129144_60EA_46A6_B604_55DE6A7803BB__INCLUDED_)
#define AFX_AXLEWEIGHT23_H__C9129144_60EA_46A6_B604_55DE6A7803BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MultiModalNormal.h"

/**
 * @brief Storage for axle-weight distributions of 2- and 3-axle trucks.
 *
 * CAxleWeight23 holds @ref CMultiModalNormal distributions for each
 * individual axle of a 2- or 3-axle truck. Each call to GetAxleDist()
 * returns the distribution for one axle of one truck type. The
 * multi-modal-normal form captures the distinct sub-populations often
 * observed in WIM axle-weight histograms (e.g. empty vs. loaded
 * trucks).
 *
 * @see CAxleWeight45
 * @see CVehicleGenGrave
 * @see CMultiModalNormal
 */
class CAxleWeight23
{
public:
	/**
	 * @brief Get the axle-weight distribution for one axle of one truck type.
	 * @param[in] iTruck Zero-based truck-axle-count bucket (0 = 2-axle, 1 = 3-axle).
	 * @param[in] iAxle  Zero-based axle index within that truck.
	 * @return The multi-modal normal weight distribution.
	 */
	CMultiModalNormal GetAxleDist(int iTruck, int iAxle);

	/// @brief Load the per-axle weight distributions for 3-axle trucks.
	void Add3AxleData(std::vector<CMultiModalNormal> vAxle);

	/// @brief Load the per-axle weight distributions for 2-axle trucks.
	void Add2AxleData(std::vector<CMultiModalNormal> vAxle);

	CAxleWeight23();
	virtual ~CAxleWeight23();

private:
	std::vector<CMultiModalNormal> m_v2AxleData;  ///< Axle-weight distributions for 2-axle trucks.
	std::vector<CMultiModalNormal> m_v3AxleData;  ///< Axle-weight distributions for 3-axle trucks.
};

#endif // !defined(AFX_AXLEWEIGHT23_H__C9129144_60EA_46A6_B604_55DE6A7803BB__INCLUDED_)
