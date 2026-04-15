/**
 * @file Axle.h
 * @brief Interface for the CAxle class — an individual axle on a bridge lane.
 */

#if !defined(AFX_AXLE_H__9EEC5C84_8B3B_4D68_A004_465AAC0FD15A__INCLUDED_)
#define AFX_AXLE_H__9EEC5C84_8B3B_4D68_A004_465AAC0FD15A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <memory>
// forward declare
class CVehicle; typedef std::shared_ptr<CVehicle> CVehicle_sp;

/**
 * @brief An individual axle carrying a wheel load, tracked independently of
 *        the vehicle it belongs to.
 *
 * @ref CBridgeLane flattens every axle from every vehicle on the lane into
 * a vector of CAxle instances. This lets the time-stepping loop in
 * @ref CBridge::Update advance all axle positions cheaply with a linear
 * sweep, rather than walking into each vehicle for each time step.
 *
 * Position is measured along the bridge longitudinal axis, with the sign
 * of motion determined by travel direction. Transverse position and
 * eccentricity are used by influence surfaces to capture the wheel-path
 * offset across the lane.
 *
 * @note Data members are public by design: this class is used as a value
 *       container accessed directly by @ref CBridgeLane and
 *       @ref CInfluenceLine.
 *
 * @see CBridgeLane
 * @see CInfluenceLine
 * @see CVehicle
 */
class CAxle
{
public:
	/// @brief Default constructor. Leaves all fields uninitialised.
	CAxle();

	/**
	 * @brief Construct an axle from explicit parameters.
	 *
	 * @param[in] i     Axle index (position in the flattened axle vector).
	 * @param[in] t     Time at which the axle is at the datum, in seconds.
	 * @param[in] v     Speed in metres per second.
	 * @param[in] x     Initial longitudinal position in metres.
	 * @param[in] w     Axle weight in kN.
	 * @param[in] tw    Track width in metres.
	 * @param[in] dirn  Direction of travel (1 or 2).
	 */
	CAxle(size_t i, double t, double v, double x, double w, double tw, int dirn);

	/**
	 * @brief Construct an axle from a vehicle and an axle index on that vehicle.
	 *
	 * Pulls weight, track width, speed, direction, transverse position,
	 * eccentricity and lane number from the source vehicle.
	 *
	 * @param[in] i      Axle index in the flattened per-lane axle vector.
	 * @param[in] iAxle  Zero-based axle index within the vehicle.
	 * @param[in] t      Time at which the axle is at the datum, in seconds.
	 * @param[in] x      Initial longitudinal position in metres.
	 * @param[in] pVeh   Source vehicle.
	 */
	CAxle(size_t i, size_t iAxle, double t, double x, const CVehicle_sp pVeh);
	virtual ~CAxle();

	/**
	 * @brief Advance the axle's longitudinal position to @p time.
	 *
	 * Linear update: @c position = sign * speed * (time - timeAtDatum),
	 * where @c sign is @c +1 for direction 1 and @c -1 for direction 2.
	 *
	 * @param[in] time Current simulation time in seconds.
	 */
	void UpdatePosition(double time);

	double m_TimeAtDatum;   ///< Reference time in seconds at which this axle is at the position datum.
	size_t m_Index;         ///< Axle index in the flattened per-lane axle vector.
	double m_Speed;         ///< Speed in metres per second.
	double m_Position;      ///< Current longitudinal position along the bridge, in metres.
	double m_AxleWeight;    ///< Axle weight in kN.
	size_t m_Dirn;          ///< Direction of travel (1 or 2).
	double m_TrackWidth;    ///< Axle track width (wheel separation) in metres.
	double m_TransPos;      ///< Transverse position of the axle centreline in metres (from measured traffic).
	double m_Eccentricity;  ///< Transverse eccentricity from the lane centreline in metres (from generated traffic).
	size_t m_Lane;          ///< Zero-based bridge lane index this axle belongs to.

private:
	int m_Sign;             ///< Direction sign: +1 if m_Dirn == 1, -1 otherwise.
	//CVehicle_sp m_pVeh;
};

#endif // !defined(AFX_AXLE_H__9EEC5C84_8B3B_4D68_A004_465AAC0FD15A__INCLUDED_)
