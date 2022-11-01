#pragma once

#include "OutputManagerBase.h"
#include "EventStatistics.h"

class CStatsManager : public COutputManagerBase
{
public:
	CStatsManager(void);
	CStatsManager(CConfigDataCore& config);
	virtual ~CStatsManager(void);

	virtual void Update(CEvent Ev);
	virtual void Initialize(double BridgeLength, size_t nLE, double SimStartTime);

private:
	void Creator();
	
	virtual void WriteSummaryFiles();
	virtual void CheckBuffer(bool bForceOutput);
	virtual void WriteBuffer();

	void WriteCumulativeFile();
	void WriteIntervalHeadings();
	void accumulateLE(unsigned int i, double x);

	std::vector<CEventStatistics> m_vIntervalStats;
	std::vector<CEventStatistics> m_vCumulativeStats;
	std::vector< std::vector<CEventStatistics> > m_vIntStatsBuffer;

	double m_CurTime;
	unsigned int m_CurIntervalNo;

	unsigned int WRITE_SS_INTERVAL_SIZE;
	bool WRITE_SS_INTERVALS;
	bool WRITE_SS_CUMULATIVE;
};
