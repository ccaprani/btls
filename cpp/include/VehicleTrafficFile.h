/**
 * @file VehicleTrafficFile.h
 * @brief Interface for the CVehicleTrafficFile class — parses a traffic data file into CVehicle instances.
 */

#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <memory>
#include "Vehicle.h"

class CVehicleFileParser
{
public:
	virtual ~CVehicleFileParser() = default;
	virtual CVehicle_sp nextVehicle() = 0;
};

/**
 * @brief Reads a traffic data file and produces a sequence of @ref CVehicle instances.
 *
 * CVehicleTrafficFile delegates file parsing to a CVehicleFileParser.
 * Fixed-width formats (CASTOR, BeDIT, DITIS, MON) and header-based CSV
 * formats (currently SiWIM) are handled by separate parser implementations
 * that all produce @ref CVehicle objects
 * with their timestamps, geometry and weights populated. It is used
 * both by the standalone BTLS binary (to feed a replay-mode simulation)
 * and by the PyBTLS Python layer (to load observed WIM data).
 *
 * After Read() populates the internal vehicle list, the file's metadata
 * (number of days covered, number of lanes, directions, and the
 * inclusive start/end times) can be queried via the getter accessors
 * and the vehicles can be popped in order via getNextVehicle(), or
 * retrieved in bulk via getVehicles().
 *
 * The constructor accepts a speed override. @p UseConstSpeed is the
 * master switch: when true, every vehicle's velocity is replaced by one
 * constant value. @p UseAveSpeed selects where that constant comes from:
 * the average speed across the file when true, @p ConstSpeed when false.
 * @p UseAveSpeed has no effect while @p UseConstSpeed is false, in which
 * case the speeds read from the file are used as-is.
 *
 * @see CVehicle
 * @see CLaneFileTraffic
 */
class CVehicleTrafficFile
{
public:
	/**
	 * @brief Construct with a vehicle classifier and speed override flags.
	 *
	 * @param[in] pVC           Vehicle classifier used to tag each parsed vehicle.
	 * @param[in] UseConstSpeed Master switch: if true, every vehicle's velocity is replaced by one constant value.
	 * @param[in] UseAveSpeed   Source of that constant: the file-average speed if true, @p ConstSpeed if false. Ignored when @p UseConstSpeed is false.
	 * @param[in] ConstSpeed    Constant speed in km/h, used when @p UseConstSpeed is true and @p UseAveSpeed is false.
	 */
	CVehicleTrafficFile(CVehicleClassification_sp pVC, bool UseConstSpeed, bool UseAveSpeed, double ConstSpeed);
	~CVehicleTrafficFile(void);

	/**
	 * @brief Parse a traffic file.
	 *
	 * @param[in] file     Path to the input file.
	 * @param[in] filetype File-format tag (1 = CASTOR, 2 = BeDIT, 3 = DITIS, 4 = MON, 5 = SiWIM CSV).
	 */
	void Read(std::filesystem::path file, int filetype);

	/**
	 * @brief Replace the internal vehicle list with an externally-supplied vector.
	 *
	 * Used by the PyBTLS Python layer when vehicles are constructed
	 * directly in Python rather than parsed from a file.
	 *
	 * @param[in] vVehicles Vehicles to attach.
	 */
	void AssignTraffic(std::vector<CVehicle_sp> vVehicles);

	/// @brief Get the number of calendar days spanned by the file.
	size_t getNoDays();
	/// @brief Get the total number of lanes in the file.
	size_t getNoLanes();
	/// @brief Get the number of traffic directions represented (1 or 2).
	size_t getNoDirn();
	/// @brief Get the number of lanes in direction 1.
	size_t getNoLanesDir1();
	/// @brief Get the number of lanes in direction 2.
	size_t getNoLanesDir2();
	/// @brief Get the total number of vehicles parsed.
	size_t getNoVehicles();

	/// @brief Pop and return the next vehicle from the internal cursor.
	CVehicle_sp getNextVehicle();

	/// @brief Get a copy of all parsed vehicles.
	std::vector<CVehicle_sp> getVehicles() const { return m_vVehicles; };

	/// @brief Get the timestamp of the first vehicle in the file, in seconds.
	double getStartTime();

	/// @brief Get the timestamp of the last vehicle in the file, in seconds.
	double getEndTime();

private:
	/// @brief Scan the vehicle list to populate day/lane/direction counts and start/end times.
	void AnalyseTraffic();

	/// @brief Refresh derived properties after AssignTraffic() or Read().
	void UpdateProperties();

	/// @brief Apply the speed-override flags to every vehicle.
	void SetSpeed();

	bool m_UseConstSpeed;       ///< Master switch: overwrite every vehicle's velocity with one constant value.
	bool m_UseAveSpeed;         ///< Source of that constant: file-average speed (true) or @c m_ConstSpeed (false). Ignored unless @c m_UseConstSpeed.
	double m_ConstSpeed;        ///< Constant speed in km/h, used when @c m_UseConstSpeed is true and @c m_UseAveSpeed is false.

	CVehicleClassification_sp m_pVehClassification;  ///< Vehicle classifier applied during parsing.

	size_t m_NoVehs;            ///< Number of vehicles parsed.
	size_t m_NoDays;            ///< Number of calendar days spanned.
	size_t m_NoLanes;           ///< Total lanes represented.
	size_t m_NoDirn;            ///< Number of directions (1 or 2).
	size_t m_NoLanesDir1;       ///< Lanes in direction 1.
	size_t m_NoLanesDir2;       ///< Lanes in direction 2.

	double m_Starttime;         ///< Timestamp of the first vehicle, in seconds.
	double m_Endtime;           ///< Timestamp of the last vehicle, in seconds.

	unsigned int m_iCurVehicle; ///< Cursor into the vehicle vector for getNextVehicle().
	std::vector<CVehicle_sp> m_vVehicles;  ///< Parsed vehicle stream.
};
