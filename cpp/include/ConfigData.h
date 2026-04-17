/**
 * @file ConfigData.h
 * @brief Interface for the CConfigDataCore and CConfigData classes — simulation configuration.
 */

#pragma once

#include "CSVParse.h"

/**
 * @brief Core simulation configuration block — the full settings tree
 *        driving a BTLS run.
 *
 * CConfigDataCore holds every knob a BTLS simulation exposes, organised
 * into a set of nested structs (Mode, Road, Gen, Read, Traffic, Sim,
 * Output, Time) each of which groups related fields. When the
 * standalone BTLS binary parses a @c BTLSin.txt file, the parsed values
 * are written directly into these structs; the PyBTLS Python layer sets
 * them programmatically via pybind11 bindings.
 *
 * The @ref CConfigData singleton below exposes a single process-wide
 * instance for classes that need configuration access without receiving
 * a reference through their constructor.
 *
 * @see CConfigData
 */
class CConfigDataCore
{

public:
	CConfigDataCore();
	CConfigDataCore(const CConfigDataCore& other);

	~CConfigDataCore() {};

	/**
	 * @brief Parse a @c BTLSin.txt configuration file into this object.
	 *
	 * @param[in] inFile Path to the BTLSin file.
	 * @return True on success, false if the file could not be opened.
	 */
	bool ReadData(std::string inFile);

	/// @brief Top-level run-mode settings.
	struct Mode_Config
	{
		size_t PROGRAM_MODE;  ///< Program operating mode (simulation / generation / analysis).
	} Mode = {1};

	/// @brief Road layout (number of lanes, directions) and lane-flow source file.
	struct Road_Config
	{
		std::string LANES_FILE;  ///< CSV file describing per-lane flow rates.
		size_t NO_LANES_DIR1;    ///< Lanes in direction 1 (populated from LANES_FILE).
		size_t NO_LANES_DIR2;    ///< Lanes in direction 2 (populated from LANES_FILE).
		size_t NO_LANES;         ///< Total lanes across both directions.
		size_t NO_DIRS;          ///< Number of traffic directions (1 or 2).
	} Road = {"laneflow.csv", 1, 1, 2, 2};

	/// @brief Traffic-generation settings (generation mode only).
	struct Gen_Config
	{
		std::string TRAFFIC_FOLDER;    ///< Folder containing the generation input files.
		bool GEN_TRAFFIC;              ///< If true, generate traffic rather than read from file.
		size_t NO_DAYS;                ///< Number of days to generate.
		double TRUCK_TRACK_WIDTH;      ///< Default truck track width in mm (input unit, not metres).
		double LANE_ECCENTRICITY_STD;  ///< Standard deviation of lane-eccentricity random perturbation.
		int KERNEL_TYPE;               ///< Kernel type for garage/nominal models (EKernelType enum).
		double NO_OVERLAP_LENGTH;      ///< Reference length for multi-vehicle no-overlap reasoning.
	} Gen = {"../Traffic/Auxerre/", false, 1, 190.0, 0.0, 0, 100.0};

	/// @brief Traffic-reading settings (replay mode only).
	struct Read_Config
	{
		bool READ_FILE;              ///< If true, replay traffic from a file instead of generating.
		std::string TRAFFIC_FILE;    ///< Traffic data file path (CASTOR/BeDIT/DITIS/MON format).
		std::string GARAGE_FILE;     ///< Pre-recorded vehicle pool file (for the Garage model).
		std::string KERNEL_FILE;     ///< Kernel parameters file for randomisation.
		std::string NOMINAL_FILE;    ///< Nominal vehicle definition file.
		size_t FILE_FORMAT;          ///< File format tag (1=CASTOR, 2=BeDIT, 3=DITIS, 4=MON).
		bool USE_CONSTANT_SPEED;     ///< Override each vehicle's speed with @c CONST_SPEED.
		bool USE_AVE_SPEED;          ///< Override each vehicle's speed with the file average.
		double CONST_SPEED;          ///< Constant speed used when USE_CONSTANT_SPEED is true.
	} Read = {false, "traffic.txt", "garage.txt", "kernel.csv", "nominal_vehicle.csv", 4, false, false, 0.0};

	/// @brief Traffic model selection (which vehicle generator and headway model).
	struct Traffic_Config
	{
		int VEHICLE_MODEL;              ///< Vehicle generator selector (EVehicleModel enum).
		int HEADWAY_MODEL;              ///< Flow/headway generator selector (EFlowModel enum).
		int CLASSIFICATION;             ///< Vehicle classification scheme to apply.
		double CONGESTED_SPACING;       ///< Mean spacing in congested mode, in metres.
		double CONGESTED_SPEED;         ///< Speed in congested mode, in m/s.
		double CONGESTED_GAP;           ///< Mean gap in congested mode (derived), in seconds.
		double CONGESTED_GAP_COEF_VAR;  ///< Coefficient of variation of the congested gap.
		double CONSTANT_SPEED;          ///< Speed used by the Constant flow model, in m/s.
		double CONSTANT_GAP;            ///< Gap used by the Constant flow model, in seconds.
	} Traffic = {0, 0, 1, 5.0, 30.0, 5.0, 0.05, 36.0, 10.0};

	/// @brief Bridge load-effect calculation settings.
	struct Sim_Config
	{
		bool CALC_LOAD_EFFECTS;    ///< If true, run the bridge load-effect calculation at all.
		std::string BRIDGE_FILE;   ///< Bridge definitions file.
		std::string INFLINE_FILE;  ///< Influence lines file.
		std::string INFSURF_FILE;  ///< Influence surfaces file.
		double CALC_TIME_STEP;     ///< Inner-loop time step in seconds (typical 0.01–0.1).
		size_t MIN_GVW;            ///< Minimum GVW (in kN) below which vehicles are treated as cars.
	} Sim = {false, "bridges.txt", "IL.txt", "IS.csv", 0.1, 35};

	/// @brief Output configuration for every writer in the event pipeline.
	struct Output_Config
	{
		bool WRITE_TIME_HISTORY;           ///< Enable per-timestep time-history file output.
		bool WRITE_EACH_EVENT;             ///< Enable all-events file output (every completed event).
		size_t WRITE_EVENT_BUFFER_SIZE;    ///< Buffer size for the all-events stream.
		bool WRITE_FATIGUE_EVENT;          ///< Enable the fatigue-events buffer.

		/// @brief Generated vehicle output (for generation mode).
		struct VehicleFile_Config
		{
			bool WRITE_VEHICLE_FILE;            ///< Enable the generated vehicle output file.
			size_t FILE_FORMAT;                 ///< Output file format tag.
			std::string VEHICLE_FILENAME;       ///< Output file path.
			size_t WRITE_VEHICLE_BUFFER_SIZE;   ///< Buffer size in vehicles.
			bool WRITE_FLOW_STATS;              ///< Enable per-hour flow statistics output.
		} VehicleFile;

		/// @brief Block-maxima writer configuration.
		struct BlockMax_Config
		{
			bool WRITE_BM;                  ///< Enable the block-maxima writer.
			bool WRITE_BM_VEHICLES;         ///< Include per-vehicle detail in block-max files.
			bool WRITE_BM_SUMMARY;          ///< Include summary files.
			bool WRITE_BM_MIXED;            ///< Enable the mixed-stream (all-vehicle-counts) output.
			size_t BLOCK_SIZE_DAYS;         ///< Block size in days.
			size_t BLOCK_SIZE_SECS;         ///< Block size in seconds (derived).
			size_t WRITE_BM_BUFFER_SIZE;    ///< Buffer size in completed blocks.
		} BlockMax;

		/// @brief Peaks-over-threshold writer configuration.
		struct POT_Config
		{
			bool WRITE_POT;                 ///< Enable the POT writer.
			bool WRITE_POT_VEHICLES;        ///< Include per-vehicle detail in POT files.
			bool WRITE_POT_SUMMARY;         ///< Include summary files.
			bool WRITE_POT_COUNTER;         ///< Enable the per-block exceedance counter file.
			size_t POT_COUNT_SIZE_DAYS;     ///< Counter block size in days.
			size_t POT_COUNT_SIZE_SECS;     ///< Counter block size in seconds (derived).
			size_t WRITE_POT_BUFFER_SIZE;   ///< Buffer size in POT events.
		} POT;

		/// @brief Running-statistics writer configuration.
		struct Stats_Config
		{
			bool WRITE_STATS;               ///< Enable the statistics writer.
			bool WRITE_SS_CUMULATIVE;       ///< Write cumulative statistics over the full run.
			bool WRITE_SS_INTERVALS;        ///< Write per-interval statistics.
			size_t WRITE_SS_INTERVAL_SIZE;  ///< Interval size in seconds.
			size_t WRITE_SS_BUFFER_SIZE;    ///< Buffer size in intervals.
		} Stats;

		/// @brief Fatigue rainflow-counting configuration.
		struct Fatigue_Config
		{
			bool DO_FATIGUE_RAINFLOW;       ///< Enable fatigue rainflow counting.
			int RAINFLOW_DECIMAL;           ///< Decimal precision for rainflow binning.
			double RAINFLOW_CUTOFF;         ///< Amplitude cut-off below which cycles are ignored.
			size_t WRITE_FATIGUE_BUFFER_SIZE;  ///< Buffer size for rainflow events.
		} Fatigue;

	} Output = {false, false, 10000, false,
				{false, 4, "output_traffic.txt", 10000, false},	// VehicleFile_Config
				{false, false, false, false, 1, 0, 10000},		// BlockMax_Config
				{false, false, false, false, 1, 0, 10000},		// POT_Config
				{false, false, false, 3600, 10000},				// Stats_Config
				{false, 3, 0.0, 10000}};						// Fatigue_Config

	/// @brief Time-unit constants used throughout the simulation.
	struct Time_Config
	{
		size_t DAYS_PER_MT;     ///< Days per "month" (simulation month, not calendar).
		size_t MTS_PER_YR;      ///< Months per year (simulation year).
		size_t HOURS_PER_DAY;   ///< Hours per day (24).
		size_t SECS_PER_HOUR;   ///< Seconds per hour (3600).
		size_t MINS_PER_HOUR;   ///< Minutes per hour (60).
		size_t SECS_PER_MIN;    ///< Seconds per minute (60).
	} Time = {25, 10, 24, 3600, 60, 60};

	/**
	 * @brief Populate @ref Road_Config from externally-derived counts.
	 *
	 * Used when the road layout is driven from a lane-flow file parsed
	 * outside of this class.
	 *
	 * @param[in] noLanes     Total number of lanes.
	 * @param[in] noDirs      Number of directions (1 or 2).
	 * @param[in] noLanesDir1 Lanes in direction 1.
	 * @param[in] noLanesDir2 Lanes in direction 2.
	 */
	void setRoad(size_t noLanes, size_t noDirs, size_t noLanesDir1, size_t noLanesDir2);

private:
	CCSVParse m_CSV;              ///< CSV parser used by ReadData().
	std::string m_CommentString;  ///< Comment prefix recognised in BTLSin.txt.

	/// @brief Compute derived constants from the parsed configuration.
	void doDerivedConstants();

	/// @brief Pull the parsed CSV rows into the struct fields.
	void ExtractData();

	/// @brief Read the next non-comment, non-empty line from the CSV parser.
	std::string GetNextDataLine();

};


/**
 * @brief Process-wide singleton wrapper around @ref CConfigDataCore.
 *
 * CConfigData exposes a single lazily-initialised instance via get(),
 * so classes that need read access to the configuration can fetch it
 * without being passed a reference through every constructor. The
 * standard singleton pattern (deleted copy + assignment) is used.
 *
 * Most classes should still accept a @ref CConfigDataCore reference
 * through their constructor — the singleton is there for the handful
 * of deeply-nested helpers (e.g. @ref CVehicle::setConstants) that
 * would otherwise need a reference threaded through many layers.
 *
 * @see CConfigDataCore
 */
class CConfigData : public CConfigDataCore
{
	// Using Singleton pattern here:
	// https://stackoverflow.com/questions/1008019/c-singleton-design-pattern/1008289#1008289
public:
	/**
	 * @brief Get the singleton instance (lazily initialised on first call).
	 */
	static CConfigData& get()
	{
		static CConfigData instance; // Guaranteed to be destroyed.
		return instance;             // Instantiated on first use.
	}

	// C++ 11
	// =======
	// We can use the better technique of deleting the methods
	// we don't want.
	CConfigData(CConfigData const &) = delete;
	void operator=(CConfigData const &) = delete;
	// Note: Scott Meyers mentions in his Effective Modern
	//       C++ book, that deleted functions should generally
	//       be public as it results in better error messages
	//       due to the compilers behavior to check accessibility
	//       before deleted status

private:
	CConfigData() {};
};
