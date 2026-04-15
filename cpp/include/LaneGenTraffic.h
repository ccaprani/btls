/**
 * @file LaneGenTraffic.h
 * @brief Interface for the CLaneGenTraffic class — lane producing generated traffic.
 */

#pragma once

#include "Lane.h"
#include "VehicleGenerator.h"
#include "Distribution.h"
#include "LaneFlowComposition.h"
#include "FlowGenerator.h"
#include "VehicleGenGrave.h"
#include "VehicleGenNominal.h"
#include "VehicleGenGarage.h"

/**
 * @brief A traffic lane that produces vehicles on demand using a flow
 *        generator and a vehicle generator.
 *
 * CLaneGenTraffic couples a @ref CFlowGenerator (which produces arrival
 * times and gaps) with a @ref CVehicleGenerator (which produces vehicle
 * geometry and weights). Each call to GetNextVehicle() returns the
 * currently cached next vehicle and then generates the following one,
 * updating the cached arrival time and the reference vehicle used for
 * headway calculation.
 *
 * The per-lane state that couples successive vehicles — @c m_pPrevVeh
 * and @c m_pNextVeh — carries over the minimum-gap constraint between
 * consecutive arrivals, so the flow generator can honour car-following
 * behaviour across the entire simulation.
 *
 * @see CLane
 * @see CFlowGenerator
 * @see CVehicleGenerator
 * @see CLaneFlowComposition
 */
class CLaneGenTraffic : public CLane
{
public:
	/**
	 * @brief Construct a generated-traffic lane bound to the shared configuration.
	 * @param[in] config Shared configuration block.
	 */
	CLaneGenTraffic(CConfigDataCore& config);
	~CLaneGenTraffic(void);

	/**
	 * @brief Pop and return the next generated vehicle, then generate the
	 *        following one.
	 */
	virtual CVehicle_sp GetNextVehicle();

	/**
	 * @brief Configure the lane with a classification, flow composition, and start time.
	 *
	 * This overload constructs the vehicle generator and flow generator
	 * internally based on the configuration's @c VEHICLE_MODEL and
	 * @c HEADWAY_MODEL tags.
	 *
	 * @param[in] pVC       Vehicle classifier shared across the road.
	 * @param[in] lfc       Lane flow composition (lane number, direction, flow rates).
	 * @param[in] starttime Simulation start time in seconds.
	 */
	void setLaneData(CVehicleClassification_sp pVC, CLaneFlowComposition lfc, const double starttime);

	/**
	 * @brief Configure the lane with pre-built generators.
	 *
	 * Used when the caller wants to share vehicle or flow generators
	 * across lanes instead of letting each lane construct its own.
	 *
	 * @param[in] lfc         Lane flow composition.
	 * @param[in] pVehicleGen Vehicle generator (shared pointer).
	 * @param[in] pFlowGen    Flow (arrival/headway) generator (shared pointer).
	 * @param[in] startTime   Simulation start time in seconds.
	 */
	void setLaneData(CLaneFlowComposition lfc, CVehicleGenerator_sp pVehicleGen, CFlowGenerator_sp pFlowGen, const double startTime);

	/**
	 * @brief Initialize the flow model data and prime the first arrival.
	 *
	 * @param[in] pFlowModelData Flow model data supplied by the traffic configuration.
	 */
	void initLane(CFlowModelData_sp pFlowModelData);

private:
	/// @brief Compute the next arrival time, accounting for minimum gap constraints.
	void GenNextArrival();

	/// @brief Advance the lane clock to the next arrival time.
	void GenNextTime();

	/// @brief Generate the next vehicle via the vehicle generator and cache it.
	void GenNextVehicle();

	/// @brief Set lane bookkeeping (start time, direction, lane index) from a flow composition.
	inline void setLaneAttributes(CLaneFlowComposition lfc, const double startTime) {
		m_NextArrivalTime = startTime;
		m_Direction = lfc.getDirn();
		m_LaneIndex = lfc.getGlobalLaneNo();	// Map vehicles to global lane using zero based cumulative lane no.
	}

	CConfigDataCore& m_Config;              ///< Reference to the shared configuration.

	CVehicleGenerator_sp m_pVehicleGen;     ///< Vehicle geometry/weight generator.
	CVehicleModelData_sp m_pVehModelData;   ///< Vehicle model distributions (weights, spacings, etc.).
	CFlowGenerator_sp m_pFlowGen;           ///< Arrival/headway generator.
	CFlowModelData_sp m_pFlowModelData;     ///< Flow model distributions (gap, speed).

	CVehicle_sp m_pPrevVeh;                 ///< Most recently emitted vehicle (anchor for headway).
	CVehicle_sp m_pNextVeh;                 ///< Candidate next vehicle awaiting the simulation clock.

	int		HEADWAY_MODEL;                  ///< Selector for the flow-generator subclass (from config).
	int		VEHICLE_MODEL;                  ///< Selector for the vehicle-generator subclass (from config).
	size_t	NO_LANES;                       ///< Total lanes across the road (from config).
};
typedef std::shared_ptr<CLaneGenTraffic> CLaneGenTraffic_sp;  ///< Shared-pointer alias for CLaneGenTraffic.
