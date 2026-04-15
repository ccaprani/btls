/**
 * @file Lane.h
 * @brief Interface for the CLane class — abstract base for a single traffic lane.
 */

#if !defined(AFX_LANE_H__7ED6AACE_9B6B_4619_98A7_E7EA4F38FFD1__INCLUDED_)
#define AFX_LANE_H__7ED6AACE_9B6B_4619_98A7_E7EA4F38FFD1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <vector>
#include "Vehicle.h"

/**
 * @brief Abstract base class for a single traffic lane producing vehicles
 *        for the simulation.
 *
 * CLane is the interface the main simulation loop talks to when pulling
 * vehicles onto bridges. Two concrete subclasses exist:
 *
 * - @ref CLaneGenTraffic: generates traffic on the fly using a flow
 *   generator and a vehicle generator.
 * - @ref CLaneFileTraffic: replays pre-recorded traffic from a file.
 *
 * The lane caches the next vehicle it will produce in @c m_pNextVeh and
 * its arrival time in @c m_NextArrivalTime, so the simulation loop can
 * sort lanes by next arrival without forcing the generator to commit.
 *
 * @see CLaneGenTraffic
 * @see CLaneFileTraffic
 */
class CLane
{
public:
	CLane();
	virtual ~CLane();

	/// @brief Get the global lane index in the road layout (zero-based).
	size_t GetLaneID();

	/// @brief Get the direction of travel (1 or 2).
	size_t GetDirection();

	/**
	 * @brief Pop and return the next vehicle for this lane.
	 *
	 * Concrete subclasses produce the next vehicle by either generating
	 * it from a statistical model or reading it from a traffic file.
	 * After this call, @c m_pNextVeh and @c m_NextArrivalTime are
	 * refreshed for the subsequent vehicle.
	 *
	 * @return Shared pointer to the next vehicle on this lane.
	 */
	virtual CVehicle_sp GetNextVehicle() = 0;

	/// @brief Get the arrival time of the cached next vehicle, in seconds.
	double GetNextArrivalTime() const;

protected:
	size_t	m_LaneIndex;        ///< Global lane index (zero-based).
	size_t	m_Direction;        ///< Travel direction (1 or 2).
	double	m_NextArrivalTime;  ///< Cached arrival time of @c m_pNextVeh, in seconds.

	CVehicle_sp m_pNextVeh;     ///< Cached next vehicle awaiting consumption.
};
typedef std::shared_ptr<CLane> CLane_sp;  ///< Shared-pointer alias for CLane.

#endif // !defined(AFX_LANE_H__7ED6AACE_9B6B_4619_98A7_E7EA4F38FFD1__INCLUDED_)
