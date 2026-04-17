/**
 * @file CSVParse.h
 * @brief Interface for the CCSVParse class — CSV file reader.
 *
 * Adapted from Kernighan and Pike, *The Practice of Programming* (1999).
 */

#if !defined(AFX_CSVPARSE_H__A5DED40F_C9D8_4B26_9333_14BF8D583637__INCLUDED_)
#define AFX_CSVPARSE_H__A5DED40F_C9D8_4B26_9333_14BF8D583637__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/* Copyright (C) 1999 Lucent Technologies */
/* Excerpted from 'The Practice of Programming' */
/* by Brian W. Kernighan and Rob Pike */

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <sstream>

/**
 * @brief General-purpose CSV file reader with configurable separator.
 *
 * CCSVParse reads a CSV file line by line, splits each line into fields
 * using a configurable separator, and exposes the fields by index. It
 * is used internally by @ref CConfigDataCore, @ref CModelData
 * subclasses, @ref CBridgeFile, and @ref CReadILFile for all
 * structured-text parsing.
 *
 * Usage: call OpenFile() to attach to a file and set the separator,
 * then call getline() in a loop. After each getline(), getfield()
 * returns individual fields. GetVectorFromNextLine() and
 * GetVectorFromCurrentLine() are helpers that parse an entire line
 * of numeric fields into a double vector.
 */
class CCSVParse {
	// sample input: "LU",86.25,"11/4/1998","2:19PM",+4.0625

public:
	/// @brief Close the currently open file, if any.
	void CloseFile();

	/**
	 * @brief Open a file for parsing.
	 * @param[in] inFile Path to the file.
	 * @param[in] sep    Field separator (e.g. "," or "\\t").
	 * @return True on success.
	 */
	bool OpenFile(std::string inFile, std::string sep);

	CCSVParse();
	virtual ~CCSVParse();

	/**
	 * @brief Read the next line from the file and split it into fields.
	 * @param[out] s The raw line text (before splitting).
	 * @return Number of fields, or @c -1 at EOF.
	 */
	int getline(std::string&);

	/**
	 * @brief Get the n-th field from the most recently read line.
	 * @param[in] n Zero-based field index.
	 * @return Field text, or empty string if out of range.
	 */
	std::string getfield(size_t n);

	/// @brief Get the number of fields in the most recently read line.
	size_t getnfield() const { return nfield; }

	/// @brief Read the next line and parse all fields as doubles.
	std::vector<double> GetVectorFromNextLine();

	/// @brief Parse all fields of the current (already-read) line as doubles.
	std::vector<double> GetVectorFromCurrentLine();

	/// @brief Parse a string as a double.
	double stringToDouble(std::string line);

	/// @brief Parse a string as an int.
	int stringToInt(std::string line);

	/// @brief Parse a string as a bool ("0" / "1" or "false" / "true").
	bool stringToBool(std::string line);

private:
	std::ifstream fin;                    ///< Input file stream.
	std::string line;                     ///< Most recently read raw line.
	std::vector<std::string> field;       ///< Fields split from @c line.
	size_t nfield;                        ///< Number of fields in the current line.
	std::string fieldsep;                 ///< Separator string.

	/// @brief Split @c line into @c field using @c fieldsep.
	size_t split();

	/// @brief Return true if the character is an end-of-line marker.
	int endofline(char);

	/// @brief Advance past a plain (unquoted) field.
	size_t advplain(const std::string& line, std::string& fld, size_t);

	/// @brief Advance past a quoted field (handles embedded quotes).
	size_t advquoted(const std::string& line, std::string& fld, size_t);

};

#endif // !defined(AFX_CSVPARSE_H__A5DED40F_C9D8_4B26_9333_14BF8D583637__INCLUDED_)
