#pragma once

#include <string>
#include <filesystem>
#include <vector>
#include "Bridge.h"
#include "InfluenceLine.h"
#include "CSVParse.h"

class CBridgeFile
{
public:
	CBridgeFile(void);
	CBridgeFile(std::filesystem::path file, std::vector<CInfluenceLine> vDiscreteIL);
	CBridgeFile(std::filesystem::path file, std::vector<CInfluenceLine> vDiscreteIL, CConfigDataCore& config);
	CBridgeFile(std::filesystem::path file, std::vector<CInfluenceLine> vDiscreteIL, std::vector<CInfluenceLine> vInfSurf);
	CBridgeFile(std::filesystem::path file, std::vector<CInfluenceLine> vDiscreteIL, std::vector<CInfluenceLine> vInfSurf, CConfigDataCore& config);
	~CBridgeFile(void);
	void ReadBridges(std::string file);
	void ReadBridges(std::string file, std::vector<CInfluenceLine> vDiscreteIL,std::vector<CInfluenceLine> vInfSurf);
	void ReadBridges(string file, vector<CInfluenceLine> vDiscreteIL, std::vector<CInfluenceLine> vInfSurf, CConfigDataCore& config);
	std::vector<CBridge_sp> getBridges(void);
	double getMaxBridgeLength(void);

private:
	double ReadLoadEffect(CBridge_sp pBridge, vector<CInfluenceLine> vDiscreteIL, std::vector<CInfluenceLine> vInfSurf);
	int GetNextDataLine(string& str);

	CCSVParse m_CSV;
	string m_CommentString;
	std::vector<CBridge_sp> m_vpBridge;
};

