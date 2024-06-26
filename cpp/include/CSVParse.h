// CSVParse.h: interface for the CCSVParse class.
//
//////////////////////////////////////////////////////////////////////

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


	/**A class to parse Comma Seperated Values input*/

class CCSVParse {	// read and parse comma-separated values
	// sample input: "LU",86.25,"11/4/1998","2:19PM",+4.0625

public:
	void CloseFile();
	// Is overloaded by a filesystem::path
	bool OpenFile(std::string inFile, std::string sep);
	CCSVParse();
	virtual ~CCSVParse();

	int getline(std::string&);
	std::string getfield(size_t n);
	size_t getnfield() const { return nfield; }
	std::vector<double> GetVectorFromNextLine();
	std::vector<double> GetVectorFromCurrentLine();

	double stringToDouble(std::string line);
	int stringToInt(std::string line);
	bool stringToBool(std::string line);

private:
	std::ifstream fin;			// input file pointer
	std::string line;			// input line
	std::vector<std::string> field;	// field strings
	size_t nfield;			// number of fields
	std::string fieldsep;		// separator characters

	size_t split();
	int endofline(char);
	size_t advplain(const std::string& line, std::string& fld, size_t);
	size_t advquoted(const std::string& line, std::string& fld, size_t);

};


#endif // !defined(AFX_CSVPARSE_H__A5DED40F_C9D8_4B26_9333_14BF8D583637__INCLUDED_)
