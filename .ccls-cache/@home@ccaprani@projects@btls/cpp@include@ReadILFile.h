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

class CReadILFile
{
public:
	CReadILFile(void);
	CReadILFile(std::filesystem::path file);
	CReadILFile(std::filesystem::path file, unsigned int mode);
	~CReadILFile(void);

	// Reads the IL file in and sets the distance and ordinate vectors
	void ReadILFile(std::string file);
	void ReadInfSurfFile(std::string file);
	std::vector<CInfluenceLine> getInfLines();
	std::vector<CInfluenceLine> getInfLines(std::filesystem::path file, unsigned int mode);
	int getNoInfLines(void);

private:
	double stringToDouble(std::string line);
	int stringToInt(std::string line);
	
	CCSVParse m_CSV;
	// The file name where the IL data is stored
	std::string m_File;
	std::ifstream m_InFileStream;
	std::vector<CInfluenceLine> m_vInfLine;
	int m_NoInfLines;
};

