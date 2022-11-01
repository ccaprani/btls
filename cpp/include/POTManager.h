#pragma once

#include "OutputManagerBase.h"

class CPOTManager : public COutputManagerBase
{
public:
	CPOTManager(CConfigDataCore& config);
	virtual ~CPOTManager(void);

	virtual void Update(CEvent Ev);
	virtual void Initialize(double BridgeLength, std::vector<double> vThreshold, double SimStartTime);

private:
	virtual void	WriteVehicleFiles();
	virtual void	WriteSummaryFiles();
	virtual void	CheckBuffer(bool bForceOutput);
	virtual void	OpenVehicleFiles();
	virtual void	WriteBuffer();

	void OpenCounterFile();
	void WriteCounter();
	void UpdateCounter();

	size_t m_NoPeaks;
	std::vector< std::vector<CEvent> >	m_vEvents;
	std::string m_EventFile;
	std::vector<double> m_vThreshold;
	std::string m_CounterFile;

	unsigned int m_BlockSize;
	unsigned int m_CurBlockNo;
	std::vector< std::vector<unsigned int> > m_vCounter;

	bool WRITE_POT_COUNTER;
	int  POT_COUNT_SIZE_DAYS;
	int  POT_COUNT_SIZE_SECS;
};

