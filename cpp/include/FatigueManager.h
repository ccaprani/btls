/**
 * @file FatigueManager.h
 * @brief Interface for the CFatigueManager class — fatigue rainflow output manager.
 */

#pragma once

#include "OutputManagerBase.h"
#include "Rainflow.h"

/**
 * @brief Accumulates load-effect time series and performs fatigue
 *        rainflow cycle counting.
 *
 * CFatigueManager derives from @ref COutputManagerBase but follows a
 * different accumulation pattern than the peak/block-max managers:
 * instead of consuming completed @ref CEvent instances, it receives
 * raw per-timestep load-effect values via addLoadEffectValues(). When
 * enough values have accumulated (controlled by the event buffer size)
 * or at the end of the simulation, it runs the ASTM E1049-85 rainflow
 * algorithm on the accumulated series, counts cycles, and writes the
 * result to the rainflow output file.
 *
 * The @c DO_FATIGUE_RAINFLOW, @c RAINFLOW_DECIMAL, and
 * @c RAINFLOW_CUTOFF fields from the configuration control whether
 * the analysis runs, the precision of the cycle-range binning, and the
 * minimum amplitude below which cycles are ignored.
 *
 * @see CRainflow
 * @see COutputManagerBase
 * @see CEventManager
 */
class CFatigueManager : public COutputManagerBase
{
public:
	/**
	 * @brief Construct bound to the shared configuration.
	 * @param[in] config Shared configuration block.
	 */
	CFatigueManager(CConfigDataCore &config);
	virtual ~CFatigueManager(void);

	/**
	 * @brief Initialize with the bridge length and number of load effects.
	 * @param[in] bridgeLength   Bridge length in metres (for filename stemming).
	 * @param[in] noLoadEffects  Number of load effects to accumulate.
	 */
	void Initialize(double bridgeLength, size_t noLoadEffects);

	/// @brief No-op event consumer (fatigue uses addLoadEffectValues instead).
	void Update(CEvent Ev) {};

	/// @brief Trigger a flush of the accumulated series through the rainflow algorithm.
	void Update();

	/**
	 * @brief Append one timestep's load-effect values to the accumulator.
	 * @param[in] vEffs Current load-effect values, one per load effect.
	 */
	void addLoadEffectValues(std::vector<double> vEffs);

private:
	/// @brief Reset the accumulated load-effect series.
	void cleanLoadEffectValues();

	/// @brief No-op summary writer (rainflow output uses its own format).
	void WriteSummaryFiles() {};

	/// @brief Flush if the accumulated series has reached the buffer threshold.
	virtual void CheckBuffer(bool bForceOutput);

	/**
	 * @brief Run the rainflow algorithm on the accumulated series.
	 * @param[in] bIsFinal If true, this is the final flush at end-of-simulation.
	 */
	void doRainflow(bool bIsFinal);

	/// @brief Write the current rainflow output buffer to disk.
	void writeRainflowBuffer();

	/// @brief Zero the cycle counts in the rainflow output maps.
	void cleanRainflowOutCountValues();

	std::vector<CRainflow> m_vRainflow;                  ///< One rainflow counter per load effect.
	std::ofstream m_RainflowOutFile;                     ///< Rainflow output file stream.
	std::vector<std::vector<double>> m_vLoadEffectValues; ///< Accumulated per-timestep values, one vector per LE.

	size_t m_EventCount;                                 ///< Number of events processed since last flush.

	bool m_WriteRainflowHeadLine;                        ///< True if the heading row has not yet been written.
	bool DO_FATIGUE_RAINFLOW;                            ///< Enable fatigue rainflow counting (from config).
	int RAINFLOW_DECIMAL;                                ///< Decimal precision for rainflow binning (from config).
	double RAINFLOW_CUTOFF;                              ///< Amplitude cut-off below which cycles are ignored (from config).
};
