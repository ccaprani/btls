/**
 * @file OutputManagerBase.h
 * @brief Interface for the COutputManagerBase class — abstract base for event-output writers.
 */

#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <math.h>
#include "Event.h"
#include "Effect.h"
#include "Vehicle.h"

/**
 * @brief Abstract base class for event-output writers.
 *
 * Concrete subclasses — @ref CPOTManager and @ref CBlockMaxManager —
 * consume completed @ref CEvent instances and write them to disk using
 * their own summarisation policy. COutputManagerBase factors out the
 * shared plumbing: output file paths, per-load-effect summary files,
 * buffer flush protocol, bridge length and simulation start time.
 *
 * Subclasses must implement Update() (called by @ref CEventManager for
 * every completed event), WriteSummaryFiles() and CheckBuffer(). They
 * may optionally override WriteVehicleFiles() and OpenVehicleFiles()
 * when they also emit per-vehicle detail.
 *
 * @see CPOTManager
 * @see CBlockMaxManager
 * @see CEventManager
 */
class COutputManagerBase
{
public:
	/**
	 * @brief Construct a writer whose output filenames are stemmed by @p filestem.
	 * @param[in] filestem Output-file name stem (e.g. "POT", "BM").
	 */
	COutputManagerBase(std::string filestem);
	virtual ~COutputManagerBase(void);

	/**
	 * @brief Consume one completed event.
	 *
	 * Called by @ref CEventManager::EndEvent once per completed event.
	 * Implementations decide whether to record it and how.
	 *
	 * @param[in] Ev Completed event (by value so subclasses can mutate).
	 */
	virtual void Update(CEvent Ev) = 0;

	/// @brief Flush remaining buffers and close output files.
	void Finish();

	/**
	 * @brief Threshold-aware initialisation (used by @ref CPOTManager).
	 * @param[in] BridgeLength Bridge length in metres (for filename stemming).
	 * @param[in] vThreshold   Peak-recording thresholds, one per load effect.
	 * @param[in] SimStartTime Simulation start time in seconds.
	 */
	virtual void Initialize(double BridgeLength, std::vector<double> vThreshold, double SimStartTime) {};

	/**
	 * @brief Load-effect-count initialisation (used by @ref CBlockMaxManager).
	 * @param[in] BridgeLength Bridge length in metres.
	 * @param[in] nLE          Number of load effects tracked.
	 */
	virtual void Initialize(double BridgeLength, size_t nLE) {};

protected:
	/// @brief Write the per-vehicle detail files. Default: no-op.
	virtual void	WriteVehicleFiles(){};

	/// @brief Write the per-load-effect summary files. Must be implemented by subclasses.
	virtual void	WriteSummaryFiles() = 0;

	/**
	 * @brief Flush the output buffer if it is full, or if @p bForceOutput is true.
	 * @param[in] bForceOutput Force the flush regardless of buffer state.
	 */
	virtual void	CheckBuffer(bool bForceOutput) = 0;

	/// @brief Open the per-vehicle detail files. Default: no-op.
	virtual void	OpenVehicleFiles(){};

	/// @brief Append the current buffer contents to disk. Default-implementation helper.
	virtual void	WriteBuffer();

	/// @brief Open the summary output files (one per load effect) using the configured filestem.
	void OpenSummaryFiles();

	/**
	 * @brief Open a single per-vehicle output file at index @p i.
	 * @param[in] i Zero-based load-effect index.
	 */
	void OpenVehicleFile(size_t i);

	double	m_BridgeLength;                     ///< Bridge length in metres, used for filename stemming.
	const std::string m_FileStem;               ///< Output-file name stem supplied at construction.

	double m_SimStartTime;                      ///< Simulation start time in seconds.

	size_t m_NoLoadEffects;                     ///< Number of load effects tracked.
	std::ofstream m_OutFile;                    ///< Generic output file stream.
	std::vector<std::string>	m_vOutFiles;    ///< Per-load-effect vehicle output paths.
	std::vector<std::string>	m_vSummaryFiles;///< Per-load-effect summary output paths.

	bool	WRITE_VEHICLES;                     ///< If true, per-vehicle detail files are produced.
	bool	WRITE_SUMMARY;                      ///< If true, summary files are produced.
	size_t	WRITE_BUFFER_SIZE;                  ///< Number of buffered entries before flushing.
	size_t	FILE_FORMAT;                        ///< File format tag used when writing events.

	// define template function in header file
	// See http://www.parashift.com/c++-faq-lite/templates.html#faq-35.12
	template <typename T>
	std::string to_string(T const& value)
	{
		std::stringstream sstr;
		sstr << value;
		return sstr.str();
	}

	template <typename T>
	std::string to_string(T const& value, int nDigits)
	{
		std::stringstream sstr;
		sstr << std::fixed << std::setprecision(nDigits) << value;
		return sstr.str();
	}
};
