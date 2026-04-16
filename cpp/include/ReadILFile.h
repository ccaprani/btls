/**
 * @file ReadILFile.h
 * @brief Interface for the CReadILFile class — influence line and surface file reader.
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "InfluenceLine.h"
#include "InfluenceSurface.h"
#include "CSVParse.h"

/**
 * @brief Reads influence line and influence surface files and produces
 *        configured @ref CInfluenceLine instances.
 *
 * CReadILFile parses two kinds of input:
 *
 * - **Influence line files** (via ReadILFile()): a text file listing
 *   discrete (distance, ordinate) tables for one or more influence
 *   lines.
 * - **Influence surface files** (via ReadInfSurfFile()): a CSV file
 *   containing a 2D ordinate grid for one or more influence surfaces,
 *   which are wrapped in CInfluenceLine objects with type 3.
 *
 * The parsed influence lines are retrieved via getInfLines() and passed
 * to @ref CBridgeFile, which wires them onto the lanes of each bridge.
 *
 * @see CInfluenceLine
 * @see CInfluenceSurface
 * @see CBridgeFile
 */
class CReadILFile
{
public:
	CReadILFile(void);

	/**
	 * @brief Construct and immediately parse a file.
	 * @param[in] file Path to the influence line/surface file.
	 */
	CReadILFile(std::filesystem::path file);

	/**
	 * @brief Construct with a file and an interpretation mode.
	 * @param[in] file Path to the file.
	 * @param[in] mode 0 = influence line file, 1 = influence surface CSV.
	 */
	CReadILFile(std::filesystem::path file, unsigned int mode);
	~CReadILFile(void);

	/**
	 * @brief Parse a text-format influence line file.
	 * @param[in] file Path to the file.
	 */
	void ReadILFile(std::string file);

	/**
	 * @brief Parse a CSV-format influence surface file.
	 * @param[in] file Path to the file.
	 */
	void ReadInfSurfFile(std::string file);

	/// @brief Get the parsed influence lines.
	std::vector<CInfluenceLine> getInfLines();

	/**
	 * @brief Parse a file and return the influence lines directly.
	 * @param[in] file Path to the file.
	 * @param[in] mode 0 = influence line, 1 = surface.
	 */
	std::vector<CInfluenceLine> getInfLines(std::filesystem::path file, unsigned int mode);

	/// @brief Get the number of influence lines parsed.
	int getNoInfLines(void);

private:
	double stringToDouble(std::string line);
	int stringToInt(std::string line);

	CCSVParse m_CSV;                          ///< CSV parser for the influence surface file.
	std::string m_File;                       ///< Path to the IL data file.
	std::ifstream m_InFileStream;             ///< Input stream for the IL file.
	std::vector<CInfluenceLine> m_vInfLine;   ///< Parsed influence lines.
	int m_NoInfLines;                         ///< Number of parsed influence lines.
};
