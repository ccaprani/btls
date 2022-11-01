#pragma once

#include "OutputManagerBase.h"
#include "BlockMaxEvent.h"

class CBlockMaxManager : public COutputManagerBase
{
public:
	CBlockMaxManager(CConfigDataCore& config);
	virtual ~CBlockMaxManager();

	virtual void Update(CEvent Ev);
	virtual void Initialize(double BridgeLength, size_t nLE, double SimStartTime);

private:
	virtual void WriteVehicleFiles();
	virtual void WriteSummaryFiles();
	virtual void CheckBuffer(bool bForceOutput);
	virtual void OpenVehicleFiles();
	virtual void WriteBuffer();
	
	void	AddExtraEvents();
	void	UpdateMixedEvents(CEvent Ev);

	CConfigDataCore& m_Config;

	CBlockMaxEvent				m_BlockMaxEvent;
	CEvent						m_BMMixedEvent;
	std::vector<CBlockMaxEvent>	m_vBMEventsBuffer;
	std::vector<CEvent>			m_vMixedEvents;

	std::string m_MixedEventFile;
	
	size_t m_MaxEvTypesForBridge;
	size_t m_BlockSize;
	size_t m_CurBlockNo;
	size_t m_CurEventNoVehicles;
	
	size_t	BLOCK_SIZE_SECS;
	size_t	BLOCK_SIZE_DAYS;
	bool	WRITE_BM_MIXED;
};