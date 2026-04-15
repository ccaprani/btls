/**
 * @file Bridge.h
 * @brief Interface for the CBridge class — a single bridge in the simulation.
 */

#pragma once

//#include <boost/thread.hpp>
//#include <boost/ref.hpp>
#include <memory>
#include "BridgeLane.h"
#include "EventManager.h"
#include "Vehicle.h"
#include "CalcEffect.h"
#include "ConfigData.h"


/**
 * @brief A single bridge carrying one or more lanes of traffic.
 *
 * CBridge owns the lanes on a bridge, the load-effect event manager, and the
 * set of vehicles currently on the bridge. It advances the simulation clock
 * through Update(), stepping through load-effect calculations at a fixed
 * interval until the next vehicle arrives at the bridge entry.
 *
 * A BTLS simulation typically contains one or more bridges, each built
 * from a @ref CConfigDataCore describing the road layout, influence lines,
 * and event thresholds. Bridges are independent of each other: a vehicle on
 * the road is copied onto every bridge it interacts with.
 *
 * @note Units used throughout: length in metres, time in seconds, load
 *       effects in whatever units the influence line returns (typically
 *       kN·m for moments, kN for shears).
 *
 * @see CBridgeLane
 * @see CEventManager
 * @see CCalcEffect
 */
class CBridge
{
public:
	/**
	 * @brief Construct a bridge from a road configuration.
	 *
	 * Copies lane-count fields from the road configuration but does not
	 * initialize lanes; InitializeLanes() must be called before use.
	 *
	 * @param[in] config Road configuration block shared by all bridges in
	 *                   the simulation.
	 */
	CBridge(CConfigDataCore& config);
	virtual ~CBridge();

	/**
	 * @brief Set the internal simulation clock.
	 *
	 * Called at the start of Update(); rarely needs to be invoked directly.
	 *
	 * @param[in] curTime Current simulation time in seconds.
	 */
	void setCurrentSimTime(double curTime);

	/**
	 * @brief Add a vehicle to the bridge.
	 *
	 * The vehicle is copied onto the heap so the same source vehicle can be
	 * added to multiple bridges in a multi-bridge simulation. The copy is
	 * routed to the lane whose index matches the vehicle's global lane
	 * number.
	 *
	 * @param[in] pVeh Shared pointer to the source vehicle.
	 */
	void AddVehicle(const CVehicle_sp& pVeh);

	/**
	 * @brief Set the time step used for load-effect calculation.
	 *
	 * Smaller steps give more accurate peak capture but scale the inner
	 * loop cost linearly. Typical values are 0.01–0.1 seconds for
	 * short-span bridges.
	 *
	 * @param[in] calcTimeStep Time step in seconds.
	 */
	void setCalcTimeStep(double calcTimeStep);

	/**
	 * @brief Set the bridge length.
	 *
	 * Must be called before InitializeLanes() and InitializeDataMgr().
	 *
	 * @param[in] length Bridge length in metres.
	 */
	void setLength(double length);

	/**
	 * @brief Advance the simulation to the next vehicle arrival.
	 *
	 * Core inner loop of the simulation. Starting from @p curTime, the
	 * method steps the clock forward by @c m_CalcTimeStep until either
	 * @p NextArrivalTime is reached or a vehicle leaves the bridge,
	 * whichever is earlier. At each step it updates every lane, sums the
	 * per-lane load effects, and hands the result to the event manager.
	 * When a vehicle leaves or the next arrival is reached it closes the
	 * current event and begins a new one. The method returns when no
	 * vehicles remain on the bridge or the next arrival time is reached.
	 *
	 * The wall-clock cost of long simulations is dominated by this method.
	 *
	 * @param[in] NextArrivalTime Time of the next vehicle arrival, in seconds.
	 * @param[in] curTime         Current simulation time, in seconds.
	 *
	 * @see CBridgeLane::Update
	 * @see CEventManager::AddNewEvent
	 * @see CEventManager::UpdateEffects
	 */
	void Update(double NextArrivalTime, double curTime);

	/**
	 * @brief Set the peak-recording thresholds used by the event manager.
	 *
	 * Each element corresponds to one load effect and controls which local
	 * maxima are recorded. A threshold of zero captures every local maximum.
	 *
	 * @param[in] vThresholds Threshold per load effect; size must equal the
	 *                        value set by setNoLoadEffects().
	 */
	void setThresholds(std::vector<double> vThresholds);

	//void UpdateMT(double NextArrivalTime, double curTime);
	//void join();

	/**
	 * @brief Flush remaining events and close the event manager.
	 *
	 * Call once at the end of the simulation so that buffered peaks and
	 * block maxima are written out by their respective output managers.
	 */
	void Finish();

	/// @brief Get the bridge index within the simulation.
	size_t getIndex(void);

	/// @brief Set the bridge index within the simulation.
	void setIndex(size_t index);

	/// @brief Get the bridge length in metres.
	double getLength(void);

	/**
	 * @brief Create @p NoLanes lanes on the bridge.
	 *
	 * Each lane is constructed with the current bridge length and assigned
	 * a zero-based index. setLength() must be called first.
	 *
	 * @param[in] NoLanes Number of lanes to create (across both directions).
	 */
	void InitializeLanes(size_t NoLanes);

	/**
	 * @brief Set the number of load effects tracked on this bridge.
	 *
	 * @param[in] nLE Number of load effects.
	 */
	void setNoLoadEffects(size_t nLE);

	/// @brief Get the number of load effects tracked on this bridge.
	size_t getNoLoadEffects(void);

	/**
	 * @brief Initialize the event-manager data stores.
	 *
	 * Allocates per-load-effect buffers inside the event manager. Must be
	 * called after setLength(), setThresholds() and setNoLoadEffects().
	 *
	 * @param[in] SimStartTime Simulation start time in seconds.
	 */
	void InitializeDataMgr(double SimStartTime);

	/**
	 * @brief Get a reference to the lane at index @p iLane.
	 *
	 * @param[in] iLane Zero-based lane index.
	 * @return Reference to the lane.
	 */
	CBridgeLane& getBridgeLane(size_t iLane);

	/// @brief Get the number of lanes on the bridge.
	size_t getNoLanes();

private:
	/// @brief Comparator ordering lanes by the time at which their next vehicle leaves the bridge.
	bool	lane_compare(const CBridgeLane* pL1, const CBridgeLane* pL2);

	/// @brief Compute the earliest time at which any vehicle will leave the bridge, across all lanes.
	double	TimeNextVehOffBridge();

	/// @brief Collect every vehicle currently on the bridge, across all lanes.
	const std::vector<CVehicle_sp> AssembleVehicles(void);

	CEventManager				m_EventMgr;        ///< Load-effect event manager for this bridge.
	CCalcEffect					m_CalcEff;         ///< Load-effect calculator associated with the bridge.
	std::vector<double>			m_vEffectValues;   ///< Working buffer for per-step load-effect values, one per load effect.
	std::vector<double>			m_vThresholds;     ///< Peak-recording thresholds, one per load effect.
	std::vector<CBridgeLane>	m_vLanes;          ///< Lanes on the bridge (across both directions).

	double	m_CurTime;          ///< Current simulation time in seconds.
	double	m_CalcTimeStep;     ///< Load-effect calculation time step in seconds.
	double	m_Length;           ///< Bridge length in metres.

	size_t	NO_LANES_DIR1;      ///< Number of lanes in direction 1 (copied from config).
	size_t	NO_DIRS;            ///< Number of traffic directions, 1 or 2 (copied from config).
	size_t	NO_LANES;           ///< Total number of lanes across both directions (copied from config).

	size_t	m_Index;            ///< Zero-based bridge index within the simulation.
	size_t	m_NoLanes;          ///< Number of lanes on this bridge.
	size_t	m_NoLoadEffects;    ///< Number of load effects tracked on this bridge.
	size_t	m_NoVehs;           ///< Number of vehicles currently on the bridge.

//	boost::thread m_Thread;
};

/// @brief Shared-pointer alias for CBridge.
typedef std::shared_ptr<CBridge> CBridge_sp;
