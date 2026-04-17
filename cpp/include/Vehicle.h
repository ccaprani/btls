/**
 * @file Vehicle.h
 * @brief Interface for the CVehicle class — a single vehicle with axle geometry and kinematics.
 */

#if !defined(AFX_VEHICLE_H__028A909A_9588_4305_9A3E_D255BD8D332A__INCLUDED_)
#define AFX_VEHICLE_H__028A909A_9588_4305_9A3E_D255BD8D332A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include <sstream>
#include <string>
#include <memory>
#include <math.h>
#include "VehicleClassification.h"

#ifdef PyBTLS
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

namespace py = pybind11;
#endif


/**
 * @brief A single vehicle with axle geometry, lane, and kinematic state.
 *
 * CVehicle is the core data structure carried through the traffic stream:
 * it stores the vehicle's identifier, arrival time, velocity, gross weight
 * and classification, together with axle-level geometry (weights,
 * cumulative spacings and per-axle track widths). When the vehicle is
 * placed on a bridge lane, @ref CBridgeLane flattens its axles into a set
 * of @ref CAxle instances for the inner simulation loop.
 *
 * Vehicles can be parsed from any of four supported traffic data file
 * formats (CASTOR, BeDIT, DITIS, MON) via create(), and serialised back
 * with Write(). In the PyBTLS (pybind11) build the full vehicle state is
 * also exposed to Python via a tuple interface (getPropInTuple() /
 * setPropByTuple()), guarded by the @c PyBTLS preprocessor macro.
 *
 * @note Unit conventions: length in metres, velocity in metres per
 *       second, gross and axle weights in kN, axle spacings in metres.
 *       File-format parsers convert from the native on-disk units
 *       (typically decimetres, kg×100) during create().
 *
 * @see CAxle
 * @see CBridgeLane
 * @see Classification
 */
class CVehicle
{
public:
	CVehicle();
#ifdef PyBTLS
	/**
	 * @brief Construct a template vehicle with @p noAxles axles and zero state.
	 *
	 * Used by the Python bindings to build vehicles programmatically
	 * before their axle weights, spacings and track widths are populated.
	 *
	 * @param[in] noAxles Number of axles to allocate.
	 */
	CVehicle(size_t noAxles);
#endif
	virtual ~CVehicle();

	/// @brief True if the vehicle's classification ID is 0 (the default "car" class).
	bool IsCar();

	/// @brief Order vehicles chronologically by arrival time.
	bool operator<(const CVehicle& x);

	/**
	 * @brief Set the vehicle's record identifier.
	 * @param[in] head Integer ID (typically a sequence number from the source file).
	 */
	void setHead(int head);

	/**
	 * @brief Serialise the vehicle to a fixed-width string in the given format.
	 *
	 * @param[in] file_type 1 = CASTOR, 2 = BeDIT, 3 = DITIS, 4 = MON.
	 * @return Fixed-width string representation in the requested format.
	 */
	std::string Write(size_t file_type);

	/**
	 * @brief Populate the vehicle from a row of traffic file data.
	 *
	 * @param[in] str    One row of fixed-width data from the source file.
	 * @param[in] format 1 = CASTOR, 2 = BeDIT, 3 = DITIS, 4 = MON.
	 */
	void create(std::string str, size_t format);

	/**
	 * @name Setters
	 * @{
	 */

	/// @brief Set the vehicle arrival time in seconds (updates the day/month/year/hour/min/sec fields).
	void	setTime(double time);
	/// @brief Set the overall vehicle length in metres.
	void	setLength(double length);
	/// @brief Set the vehicle velocity in metres per second.
	void	setVelocity(double velocity);
	/// @brief Set the local (per-direction) lane index, 1-based.
	void	setLocalLane(size_t localLaneIndex);
	/// @brief Set the local lane index from a global lane index, given the total number of road lanes.
	void	setLocalFromGlobalLane(size_t globalLaneIndex, size_t nRoadLanes);
	/// @brief Set the direction of travel (1 or 2).
	void	setDirection(size_t d);
	/// @brief Set the gross vehicle weight in kN.
	void	setGVW(double weight);
	/// @brief Set the number of axles (resizes the internal axle vector).
	void	setNoAxles(size_t noAxle);
	/**
	 * @brief Set axle weight for axle @p i.
	 * @param[in] i Zero-based axle index.
	 * @param[in] w Axle weight in kN.
	 */
	void	setAW(size_t i, double w);
	/**
	 * @brief Set axle spacing for axle @p i.
	 * @param[in] i Zero-based axle index.
	 * @param[in] s Cumulative spacing from the front axle, in metres.
	 */
	void	setAS(size_t i, double s);
	/**
	 * @brief Set axle track width for axle @p i.
	 * @param[in] i  Zero-based axle index.
	 * @param[in] tw Wheel separation in metres.
	 */
	void	setAT(size_t i, double tw);
	/// @brief Set the transverse position of the vehicle within the lane, in metres from the lane centreline.
	void	setTrans(double trans);
	/**
	 * @brief Compute and cache the on- and off-bridge times.
	 *
	 * On-bridge time is the arrival time (the BTLS time datum is the
	 * bridge entry point); off-bridge time is the on-bridge time plus the
	 * time to traverse the bridge length plus the vehicle length at the
	 * vehicle's velocity.
	 *
	 * @param[in] BridgeLength Bridge length in metres.
	 */
	void	setBridgeTimes(double BridgeLength);
	/// @brief Set the on-bridge time to the vehicle's current arrival time.
	void	setTimeOnBridge();
	/// @brief Set the zero-based bridge lane index for this vehicle.
	void	setBridgeLaneNo(size_t BridgeLaneNo);
	/// @brief Set the default track width in metres (used when per-axle widths are not supplied).
	void	setTrackWidth(double tw);
	/// @brief Set the transverse eccentricity from the lane centreline in metres.
	void	setLaneEccentricity(double e);
	/// @brief Set the vehicle classification.
	void	setClass(Classification cl);

	/** @} */

	/**
	 * @name Getters
	 * @{
	 */

	/// @brief Get the arrival time as a "DD/MM/YYYY HH:MM:SS.ss" string.
	std::string getTimeStr();
	/// @brief Get the record identifier.
	size_t	getHead();
	/// @brief Get the arrival time in seconds since the simulation epoch.
	double  getTime() const;
	/// @brief Get the overall vehicle length in metres.
	double	getLength();
	/// @brief Get the velocity in metres per second.
	double	getVelocity();
	/// @brief Get the local (per-direction) lane index, 1-based.
	size_t	getLocalLane();
	/// @brief Get the global (1-based) lane index across both directions.
	size_t	getGlobalLane(size_t nRoadLanes);
	/// @brief Get the direction of travel (1 or 2).
	size_t	getDirection();
	/// @brief Get the gross vehicle weight in kN.
	double	getGVW();
	/// @brief Get the number of axles.
	size_t	getNoAxles();
	/// @brief Get the axle weight at axle @p i, in kN.
	double	getAW(size_t i);
	/// @brief Get the cumulative axle spacing for axle @p i, in metres.
	double	getAS(size_t i);
	/// @brief Get the axle track width at axle @p i, in metres.
	double	getAT(size_t i);
	/// @brief Get the transverse position in the lane, in metres.
	double	getTrans();
	/// @brief Get the time at which the vehicle enters the bridge, in seconds.
	double	getTimeOnBridge();
	/// @brief Get the time at which the vehicle leaves the bridge, in seconds.
	double	getTimeOffBridge();
	/// @brief Return true if the vehicle is on the bridge at @p atTime.
	bool	IsOnBridge(double atTime);
	/// @brief Get the zero-based bridge lane index.
	size_t	getBridgeLaneNo(void);
	/// @brief Get the default track width in metres.
	double	getTrackWidth(void);
	/// @brief Get the transverse eccentricity from the lane centreline in metres.
	double	getLaneEccentricity(void);
	/// @brief Get the vehicle classification.
	Classification getClass();

	/** @} */

#ifdef PyBTLS
	/**
	 * @brief Serialise the vehicle's state to a Python tuple.
	 *
	 * Tuple field order: head, day, month, year, hour, min, sec, noAxles,
	 * noAxleGroups, GVW, velocity, length, lane, direction, trans,
	 * axleWeights (list), axleSpacings (list), axleWidths (list).
	 *
	 * Used by the pybind11 layer to exchange vehicle state with Python.
	 */
	py::tuple getPropInTuple();

	/**
	 * @brief Populate the vehicle from a tuple returned by getPropInTuple().
	 * @param[in] propTuple Tuple with the field order described in getPropInTuple().
	 */
	void setPropByTuple(py::tuple propTuple);
#endif

private:
	void setConstants();
	int Round(double val) {return int(val + 0.5);};
	template <typename T> std::string to_string(T const& value);
	//template<typename T> T from_string(const std::string& s);
	template <typename T> std::string truncate(T const& value, unsigned int digits);

	void createCASTORVehicle(const std::string data);
	void createBEDITVehicle(const std::string data);
	void createDITISVehicle(const std::string data);
	void createMONVehicle(const std::string data);

	std::string	writeBEDITData();
	std::string	writeCASTORData();
	std::string writeDITISData();
	std::string writeMONData();

	Classification m_Class;    ///< Vehicle classification (ID plus label).

	size_t	m_Dir;              ///< Direction of travel (1 or 2).
	size_t	m_Lane;             ///< Local (per-direction) lane number, 1-based.
	double	m_Velocity;         ///< Velocity in metres per second.
	size_t	m_Head;             ///< Record identifier from the source data row.
	size_t	m_Year;             ///< Year of arrival.
	size_t	m_Month;            ///< Month of arrival.
	size_t	m_Day;              ///< Day of arrival.
	size_t	m_Hour;             ///< Hour of arrival.
	size_t	m_Min;              ///< Minute of arrival.
	double	m_Sec;              ///< Seconds within the arrival minute (fractional).
	double	m_GVW;              ///< Gross vehicle weight in kN.
	double	m_Trns;             ///< Transverse position within the lane, in metres.
	size_t	m_NoAxles;          ///< Number of axles.
	size_t	m_NoAxleGroups;     ///< Number of axle groups (not used in the load-effect calculation).
	double	m_Length;           ///< Overall vehicle length in metres.
	double	m_TrackWidth;       ///< Default track width in metres (fallback when per-axle widths are absent).
	double	m_TimeOnBridge;     ///< Cached time at which the vehicle enters the bridge, in seconds.
	double	m_TimeOffBridge;    ///< Cached time at which the vehicle leaves the bridge, in seconds.
	size_t	m_BridgeLaneNo;     ///< Zero-based bridge lane index.
	double	m_LaneEccentricity; ///< Transverse offset from the lane centreline, in metres.

	/**
	 * @brief Per-axle geometry stored on the vehicle.
	 *
	 * Spacing is cumulative from the front axle (the first axle has spacing 0).
	 */
	struct Axle
	{
		double Weight;      ///< Axle weight in kN.
		double Spacing;     ///< Cumulative spacing from the front axle, in metres.
		double TrackWidth;  ///< Wheel track width in metres.
	};
	std::vector<Axle> m_vAxles;  ///< Per-axle geometry; size == m_NoAxles.

	size_t	DAYS_PER_MT;       ///< Days per month (from CConfigData::Time).
	size_t	MTS_PER_YR;        ///< Months per year (from CConfigData::Time).
	size_t	HOURS_PER_DAY;     ///< Hours per day (from CConfigData::Time).
	size_t	SECS_PER_HOUR;     ///< Seconds per hour (from CConfigData::Time).
	size_t	MINS_PER_HOUR;     ///< Minutes per hour (from CConfigData::Time).
	size_t	SECS_PER_MIN;      ///< Seconds per minute (from CConfigData::Time).

	double	KG100_TO_KN;       ///< Unit conversion: kg×100 → kN (0.981).
	double	KG_TO_KN;          ///< Unit conversion: kg → kN (9.81e-3).
	size_t	CASTOR_MAX_AXLES;  ///< Maximum axles supported by the CASTOR file format (9).
	size_t	BEDIT_MAX_AXLES;   ///< Maximum axles supported by the BeDIT file format (20).
	size_t	DITIS_MAX_AXLES;   ///< Maximum axles supported by the DITIS file format (20).
	size_t	MON_MAX_AXLES;     ///< Maximum axles supported by the MON file format (99).
	size_t	MON_BASE_YEAR;     ///< MON base year offset (2010) used to avoid year overflows.

	//enum eFileFormat
	//{
	//	eCASTOR,
	//	eBEDIT,
	//	eDITIS,
	//	eMON
	//} m_FileFormat;

	template<typename T>
	T from_string(const std::string& s)
	{
		// Convert from a string to a T, Type T must support >> operator
		T t;
		std::istringstream ist(s);
		ist >> t;
		return t;
	}

};

typedef std::unique_ptr<CVehicle> CVehicle_up;   ///< Unique-pointer alias for CVehicle.
typedef std::weak_ptr<CVehicle> CVehicle_wp;     ///< Weak-pointer alias for CVehicle.
typedef std::shared_ptr<CVehicle> CVehicle_sp;   ///< Shared-pointer alias for CVehicle.

#endif // !defined(AFX_VEHICLE_H__028A909A_9588_4305_9A3E_D255BD8D332A__INCLUDED_)
