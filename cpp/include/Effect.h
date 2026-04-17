/**
 * @file Effect.h
 * @brief Interface for the CEffect class — a single load-effect observation.
 */

#if !defined(AFX_EFFECT_H__0EFA9024_74FF_4E82_BF7C_D5F28B09284B__INCLUDED_)
#define AFX_EFFECT_H__0EFA9024_74FF_4E82_BF7C_D5F28B09284B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Vehicle.h"
#include <vector>
#include <iostream>
#include <algorithm>

/**
 * @brief A single recorded load-effect observation: value, time, position,
 *        and the vehicles that contributed to it.
 *
 * CEffect is a lightweight value container used by @ref CEvent to hold
 * one max or min observation for one load effect. It remembers the load
 * effect value, the simulation time and lead-vehicle position at which
 * the value was observed, and the list of vehicles on the bridge at that
 * moment (so that per-vehicle detail can be written out in event files).
 *
 * Effects are orderable by value for sorting in the output buffers.
 *
 * @see CEvent
 * @see CEventManager
 */
class CEffect
{
public:
	/// @brief Default-construct an empty effect with all fields zeroed.
	CEffect();

	/**
	 * @brief Construct an effect with an initial value, time and position.
	 *
	 * @param[in] value    Load-effect value, in the influence line's native units.
	 * @param[in] time     Simulation time in seconds at which the value was observed.
	 * @param[in] distance Lead vehicle position along the bridge, in metres.
	 */
	CEffect(double value, double time, double distance);

	virtual ~CEffect();

	/**
	 * @brief Order effects by value, for sorting in the output buffers.
	 */
	bool operator<(const CEffect& x);

	/// @brief Get the lead-vehicle position in metres.
	double getPosition();

	/// @brief Get the observation time in seconds.
	double getTime();

	/// @brief Get the load-effect value in the influence line's native units.
	double getValue() const;

	/// @brief Set the observation time in seconds.
	void setTime(double time);

	/// @brief Set the load-effect value.
	void setValue(double time);

	/// @brief Set the lead-vehicle position in metres.
	void setPosition(double time);

	/**
	 * @brief Get the i-th contributing vehicle.
	 * @param[in] i Index into the stored vehicle list.
	 * @return Copy of the vehicle at index @p i.
	 */
	CVehicle giveVehicle(size_t i) const;

	/// @brief Append a vehicle to the contributing-vehicle list.
	void AddVehicle(CVehicle& Vehicle);

	/// @brief Append all vehicles from @p vVeh to the contributing-vehicle list.
	void AddVehicles(std::vector<CVehicle> vVeh);

	/// @brief Sort the stored vehicles chronologically by arrival time.
	void sortVehicles();

	size_t m_NoVehicles;   ///< Number of vehicles that contributed to this observation.
	size_t m_IndEvent;     ///< Index of the parent event in the enclosing buffer.
	size_t m_IndEff;       ///< Index of the load effect this observation belongs to.

private:
	double m_Time;         ///< Observation time in seconds.
	double m_Val;          ///< Load-effect value in native units.
	double m_Dist;         ///< Lead-vehicle position along the bridge, in metres.
	std::vector<CVehicle> m_vVehicles;  ///< Vehicles contributing to this observation.
};

#endif // !defined(AFX_EFFECT_H__0EFA9024_74FF_4E82_BF7C_D5F28B09284B__INCLUDED_)
