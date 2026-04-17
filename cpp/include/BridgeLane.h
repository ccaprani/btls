/**
 * @file BridgeLane.h
 * @brief Interface for the CBridgeLane class — a single lane on a bridge.
 */

#pragma once

#include <vector>
#include "Vehicle.h"
#include "InfluenceLine.h"
#include "Axle.h"

/**
 * @brief A single lane on a bridge, holding the vehicles currently on it and
 *        computing per-lane load effects.
 *
 * CBridgeLane owns a list of vehicles, a flattened axle vector derived from
 * those vehicles, and the influence lines used to compute load effects. The
 * axle vector is rebuilt whenever vehicles are added or removed, so that
 * the time-stepping loop in @ref CBridge::Update only has to advance axle
 * positions cheaply through @ref CAxle::UpdatePosition.
 *
 * Lanes on the same bridge may carry traffic in opposite directions; the
 * direction is recorded per vehicle, not per lane.
 *
 * @note The default lane width is 3.65 m (set in the constructor). It is
 *       overwritten when an influence surface is registered with
 *       addLoadEffect().
 *
 * @see CBridge
 * @see CAxle
 * @see CInfluenceLine
 */
class CBridgeLane
{
public:
	/// @brief Default-construct a zero-length lane with default lane width.
	CBridgeLane();

	/**
	 * @brief Construct a lane of a given length.
	 * @param[in] length Lane length in metres.
	 */
	CBridgeLane(double length);
	~CBridgeLane(void);

	/**
	 * @brief Order lanes by the time at which their next vehicle leaves the bridge.
	 */
	bool	operator<(const CBridgeLane& other);

	/**
	 * @brief Advance axle positions on the lane to @p curTime.
	 *
	 * Every @ref CAxle stored in the lane has its position updated via
	 * @ref CAxle::UpdatePosition. The vehicle list itself is unchanged —
	 * purgeVehicles() is the corresponding operation that removes vehicles
	 * that have left the bridge.
	 *
	 * @param[in] curTime Current simulation time in seconds.
	 */
	void	Update(double curTime);

	/// @brief Set the global lane number (across the whole road layout).
	void	setLaneNo(size_t LaneNo);

	/// @brief Get the global lane number.
	size_t	getLaneNo(void);

	/**
	 * @brief Register an influence line for one load effect on this lane.
	 *
	 * The influence line is copied into the lane's list of influence lines
	 * and scaled by @p weight. If the influence line is a surface, the
	 * lane width is overwritten by the surface's lane width so the
	 * transverse ordinate lookup is consistent with the surface definition.
	 *
	 * @param[in] IL     Influence line or surface for this load effect.
	 * @param[in] weight Scaling factor applied to all ordinates (e.g. a
	 *                   girder distribution factor).
	 */
	void	addLoadEffect(CInfluenceLine IL, double weight);

	/**
	 * @brief Add a vehicle to this lane and rebuild the axle vector.
	 *
	 * Takes ownership of the vehicle, appends it to the vehicle list,
	 * rebuilds the flattened axle vector, and updates the
	 * time-of-next-vehicle-off cache.
	 *
	 * @param[in] pVeh Unique pointer to the vehicle (ownership transferred).
	 */
	void	AddVehicle(CVehicle_up pVeh);

	/// @brief Get the zero-based lane index within the bridge.
	size_t	getIndex(void);

	/// @brief Set the zero-based lane index within the bridge.
	void	setIndex(size_t indx);

	/**
	 * @brief Remove vehicles that have left the bridge.
	 *
	 * A vehicle is removed when @ref CVehicle::IsOnBridge returns false for
	 * @p curTime. When vehicles are removed, the axle vector and
	 * time-of-next-vehicle-off cache are rebuilt. Emits a warning to
	 * stdout if two vehicles are leaving within 0.1 s of each other, which
	 * can indicate a potential overlap in the traffic model.
	 *
	 * @param[in] curTime Current simulation time in seconds.
	 * @return Number of vehicles remaining on the lane after the purge.
	 */
	size_t	purgeVehicles(double curTime);

	/// @brief Get the lane length in metres.
	double	getLength(void);

	/// @brief Set the lane length in metres.
	void	setLength(double length);

	/// @brief Get the cached time at which the next vehicle will leave the bridge, in seconds.
	double	getTimeNextVehOff(void) const;

	/**
	 * @brief Recompute and return the time at which the next vehicle will
	 *        leave the bridge.
	 *
	 * Updates the internal cache; subsequent calls to getTimeNextVehOff()
	 * return this value without recomputing. If the lane has no vehicles,
	 * a large sentinel value (1e300) is returned.
	 *
	 * @return Time at which the next vehicle will leave, in seconds.
	 */
	double	setTimeNextVehOff(void);

	/**
	 * @brief Compute the current load effect for one influence line.
	 *
	 * Sums the contribution of every axle on the lane using the influence
	 * line at index @p NoLE. Returns zero if the lane has no vehicles.
	 *
	 * @param[in] NoLE Zero-based index of the load effect / influence line.
	 * @return Current load effect value in the influence line's native units.
	 */
	double	getLoadEffect(size_t NoLE);

	/**
	 * @brief Get the position of the lead vehicle's first axle.
	 *
	 * Returns the position of the first axle in the internal axle vector.
	 * Axles are stored in the order they were added, so for a simply
	 * ordered traffic stream this is the front axle of the leading vehicle.
	 *
	 * @return Position in metres, relative to the lane datum.
	 */
	double	getLeadVehPosition(void);

	/// @brief Get the number of vehicles currently on the lane.
	size_t	getNoVehs(void);

	/// @brief Get a copy of the current vehicle list.
	const std::vector<CVehicle_sp> getVehicles(void);

private:
	/// @brief Rebuild the flattened axle vector from all vehicles on the lane.
	void	setAxleVector();

	std::vector<CInfluenceLine> m_vInfLine;   ///< Influence lines, one per load effect.
	std::vector<CVehicle_sp>	m_vVehicles;  ///< Vehicles currently on the lane.
	std::vector<CAxle>			m_vAxles;     ///< Flattened axle vector, rebuilt on vehicle add/remove.
	size_t	m_LaneNo;          ///< Global lane number from the road layout.
	double	m_CurTime;         ///< Most recent simulation time passed to Update().
	double	m_Length;          ///< Lane length in metres.
	size_t	m_NoLE;            ///< Number of load effects registered on this lane.
	double	m_TimeNextVehOff;  ///< Cached time at which the next vehicle leaves, in seconds.
	size_t	m_Index;           ///< Zero-based index of the lane within its bridge.
	double	m_LaneWidth;       ///< Lane width in metres (default 3.65).
};
