/**
 * @file BridgeFile.h
 * @brief Interface for the CBridgeFile class — bridge definitions file reader.
 */

#pragma once

#include <string>
#include <filesystem>
#include <vector>
#include "Bridge.h"
#include "InfluenceLine.h"
#include "CSVParse.h"

/**
 * @brief Parses the bridge definitions file and constructs fully
 *        configured @ref CBridge instances.
 *
 * CBridgeFile reads the bridge file named in the configuration, creates
 * one @ref CBridge per bridge entry, and wires up the influence lines
 * or surfaces for each load effect on each lane. The result is a vector
 * of ready-to-simulate bridge shared pointers, retrieved via
 * getBridges().
 *
 * @see CBridge
 * @see CInfluenceLine
 */
class CBridgeFile
{
public:
	/**
	 * @brief Construct with a configuration and the pre-loaded discrete influence lines and surfaces.
	 *
	 * @param[in] config      Shared configuration block.
	 * @param[in] vDiscreteIL Discrete influence lines loaded from the IL file.
	 * @param[in] vInfSurf    Influence surfaces loaded from the IS file.
	 */
	CBridgeFile(CConfigDataCore& config, std::vector<CInfluenceLine> vDiscreteIL, std::vector<CInfluenceLine> vInfSurf);
	~CBridgeFile(void);

	/**
	 * @brief Parse the bridge definitions file.
	 *
	 * @param[in] file        Path to the bridge definitions file.
	 * @param[in] vDiscreteIL Discrete influence lines.
	 * @param[in] vInfSurf    Influence surfaces.
	 */
	void ReadBridges(std::string file, std::vector<CInfluenceLine> vDiscreteIL, std::vector<CInfluenceLine> vInfSurf);

	/// @brief Get the fully configured bridge instances.
	std::vector<CBridge_sp> getBridges(void);

	/// @brief Get the longest bridge length across all parsed bridges, in metres.
	double getMaxBridgeLength(void);

private:
	/// @brief Read one load-effect block for a single bridge.
	double ReadLoadEffect(CBridge_sp pBridge, std::vector<CInfluenceLine> vDiscreteIL, std::vector<CInfluenceLine> vInfSurf);

	/// @brief Read the next non-comment, non-empty line from the bridge file.
	int GetNextDataLine(std::string& str);

	CConfigDataCore& m_Config;               ///< Shared configuration reference.

	CCSVParse m_CSV;                         ///< CSV parser for the bridge file.
	std::string m_CommentString;             ///< Comment prefix recognised in the bridge file.
	std::vector<CBridge_sp> m_vpBridge;      ///< Parsed and configured bridges.
};
