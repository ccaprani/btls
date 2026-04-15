/**
 * @file LaneFileTraffic.h
 * @brief Interface for the CLaneFileTraffic class — lane replaying pre-recorded traffic.
 */

#pragma once
#include "Lane.h"
#include <vector>

/**
 * @brief A traffic lane that replays a pre-recorded sequence of vehicles.
 *
 * CLaneFileTraffic owns a vector of vehicles parsed from a traffic data
 * file (via @ref CVehicleTrafficFile) and hands them to the simulation
 * loop in the order they were recorded. No statistical generation is
 * involved — arrival times and vehicle properties are whatever the file
 * contains.
 *
 * Typically used for replaying observed traffic (e.g. WIM data) against
 * a bridge to compute load-effect statistics for that exact traffic
 * stream.
 *
 * @see CLane
 * @see CVehicleTrafficFile
 */
class CLaneFileTraffic : public CLane
{
public:
	CLaneFileTraffic(void);
	~CLaneFileTraffic(void);

	/**
	 * @brief Pop and return the next vehicle from the stored vector.
	 *
	 * Advances the internal cursor to the following vehicle and refreshes
	 * the cached @c m_NextArrivalTime.
	 */
	virtual CVehicle_sp GetNextVehicle();

	/**
	 * @brief Configure the lane's direction and lane number.
	 * @param[in] dirn   Travel direction (1 or 2).
	 * @param[in] laneNo Global lane index.
	 */
	void setLaneData(int dirn, int laneNo);

	/**
	 * @brief Append a vehicle to the stored sequence.
	 * @param[in] pVeh Shared pointer to the vehicle.
	 */
	void addVehicle(CVehicle_sp pVeh);

	/**
	 * @brief Prime the cached next-vehicle / next-arrival-time fields
	 *        from the first vehicle in the stored sequence.
	 *
	 * Call once after all vehicles have been added via addVehicle().
	 */
	void setFirstArrivalTime();

	/// @brief Get the total number of stored vehicles still to be consumed.
	size_t GetNoVehicles() { return m_vVehicles.size(); };

private:
	std::vector<CVehicle_sp> m_vVehicles;  ///< Pre-recorded vehicles, consumed in order.
};
typedef std::shared_ptr<CLaneFileTraffic> CLaneFileTraffic_sp;  ///< Shared-pointer alias for CLaneFileTraffic.
