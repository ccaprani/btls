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
	CBridgeFile(CConfigDataCore& config, std::vector<CInfluenceLine> vDiscreteIL, std::vector<CInfluenceLine> vInfSurf);
	~CBridgeFile(void);

	void ReadBridges(std::string file, std::vector<CInfluenceLine> vDiscreteIL,std::vector<CInfluenceLine> vInfSurf);
	std::vector<CBridge_sp> getBridges(void);
	double getMaxBridgeLength(void);

private:
	double ReadLoadEffect(CBridge_sp pBridge, std::vector<CInfluenceLine> vDiscreteIL, std::vector<CInfluenceLine> vInfSurf);
	int GetNextDataLine(std::string& str);

	CConfigDataCore& m_Config;

	CCSVParse m_CSV;
	std::string m_CommentString;
	std::vector<CBridge_sp> m_vpBridge;
};

