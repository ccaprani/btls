/**
 * @file ClassPercent.h
 * @brief Interface for the CClassPercent class — per-lane per-class vehicle percentages.
 */

#if !defined(AFX_CLASSPERCENT_H__8B0D33B8_B1E1_47EB_9BAC_3A2586719A60__INCLUDED_)
#define AFX_CLASSPERCENT_H__8B0D33B8_B1E1_47EB_9BAC_3A2586719A60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>

/**
 * @brief Per-lane truck-class composition table.
 *
 * CClassPercent holds, for each lane, the fractional composition of
 * the truck stream by axle count: 2-axle, 3-axle, 4-axle, and 5-axle.
 * The composition is used by the traffic generators to decide how many
 * trucks of each axle class to sample on each lane.
 *
 * @see CVehicleGenerator
 * @see CLaneFlowComposition
 */
class CClassPercent
{
public:
	/// @brief Get the number of lanes this table covers.
	int GetNoLanes();

	/**
	 * @brief Get the percentage of trucks in axle class @p iClass on lane @p iLane.
	 * @param[in] iLane  Zero-based lane index.
	 * @param[in] iClass Axle class (0 = 2-axle, 1 = 3-axle, 2 = 4-axle, 3 = 5-axle).
	 */
	double GetClassPercent(int iLane, int iClass);

	/**
	 * @brief Set the percentage for one (lane, class) cell.
	 * @param[in] iLane  Zero-based lane index.
	 * @param[in] iClass Axle class (0..3).
	 * @param[in] val    Fractional percentage (0..1).
	 */
	void AddClassPercent(int iLane, int iClass, double val);

	CClassPercent();
	virtual ~CClassPercent();

private:
	int m_NoLanes;  ///< Number of lanes in the table.

	/// @brief Class-percentage row for one lane.
	struct CP
	{
		double CP_2Axle;  ///< 2-axle truck fraction.
		double CP_3Axle;  ///< 3-axle truck fraction.
		double CP_4Axle;  ///< 4-axle truck fraction.
		double CP_5Axle;  ///< 5-axle truck fraction.
	};

	std::vector<CP> m_vCP;  ///< Per-lane composition rows.
};

#endif // !defined(AFX_CLASSPERCENT_H__8B0D33B8_B1E1_47EB_9BAC_3A2586719A60__INCLUDED_)
