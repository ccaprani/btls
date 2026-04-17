/**
 * @file LaneFlow.h
 * @brief Interface for the CLaneFlow class — per-lane hourly traffic data.
 */

#pragma once

#include <iostream>
#include <vector>

/**
 * @brief Stores per-hour traffic data for a single lane: flow rate,
 *        speed distribution, and axle-class composition.
 *
 * CLaneFlow holds one row per hour, each containing the flow rate
 * (veh/h), speed mean and standard deviation, and the car/truck/class
 * percentages. It is populated from the lane-flow CSV file and used
 * internally by the lane-flow parsing pipeline.
 *
 * @see CLaneFlowData
 * @see CLaneFlowComposition
 */
class CLaneFlow
{
public:
	CLaneFlow(void);

	/**
	 * @brief Construct for a specific lane and direction.
	 * @param[in] lane Lane index.
	 * @param[in] dirn Direction (1 or 2).
	 */
	CLaneFlow(int lane, int dirn);
	~CLaneFlow(void);

	/**
	 * @brief Set one hour's worth of data from a flattened vector.
	 *
	 * The vector layout is: flow, speed mean, speed std, car%,
	 * 2-axle%, 3-axle%, 4-axle%, 5-axle%.
	 *
	 * @param[in] vHrData Flattened hour data.
	 */
	void setHourData(std::vector<double> vHrData);

	/// @brief Set the lane index.
	void setLaneNo(int lane);

	/// @brief Set the direction.
	void setDirn(int dirn);

	/// @brief Get the lane index.
	int getLaneNo(void) const;

	/// @brief Get the direction.
	int getDirn(void) const;

private:
	/// @brief Per-hour speed distribution (mean, standard deviation).
	struct Speed
	{
		double Mean;
		double StdDev;
	};

	/// @brief Per-hour axle-class composition percentages.
	struct CP
	{
		double CP_cars;   ///< Car percentage.
		double CP_2Axle;  ///< 2-axle truck percentage.
		double CP_3Axle;  ///< 3-axle truck percentage.
		double CP_4Axle;  ///< 4-axle truck percentage.
		double CP_5Axle;  ///< 5-axle truck percentage.
	};

	/// @brief One hour's data record.
	struct HourData
	{
		double flow;        ///< Flow rate in vehicles per hour.
		Speed hourSpeed;    ///< Speed distribution for this hour.
		CP hourCP;          ///< Axle-class composition for this hour.
	};

	std::vector<HourData> m_vHourData;  ///< Per-hour data records.
	int m_LaneNo;                       ///< Lane index.
	int m_Dirn;                         ///< Direction.
	void Initialize(void);              ///< Reset to default (24 zero-filled hours).

public:
	/// @brief Get the flow rate for hour @p iHour (veh/h).
	double getFlow(int iHour);
	/// @brief Get the mean speed for hour @p iHour (m/s).
	double getSpeedMean(int iHour);
	/// @brief Get the speed standard deviation for hour @p iHour (m/s).
	double getSpeedStDev(int iHour);
	/// @brief Get the car percentage for hour @p iHour.
	double getCP_cars(int iHour);
	/// @brief Get the 2-axle truck percentage for hour @p iHour.
	double getCP_2Axle(int iHour);
	/// @brief Get the 3-axle truck percentage for hour @p iHour.
	double getCP_3Axle(int iHour);
	/// @brief Get the 4-axle truck percentage for hour @p iHour.
	double getCP_4Axle(int iHour);
	/// @brief Get the 5-axle truck percentage for hour @p iHour.
	double getCP_5Axle(int iHour);
	/// @brief Get all class percentages for hour @p iHour as a vector.
	std::vector<double> getCP(int iHour);
};
