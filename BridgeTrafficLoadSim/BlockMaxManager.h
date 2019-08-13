#pragma once

#include "OutputManagerBase.h"
#include "BlockMaxEvent.h"

class CBlockMaxManager : public COutputManagerBase
{
public:
	CBlockMaxManager();
	virtual ~CBlockMaxManager();

	virtual void Update(CEvent Ev);
	virtual void Initialize(double BridgeLength, size_t nLE);

private:
	virtual void	WriteVehicleFiles();
	virtual void	WriteSummaryFiles();
	virtual void	CheckBuffer(bool bForceOutput);
	virtual void	OpenVehicleFiles();
	virtual void	WriteBuffer();
	
	void	AddExtraEvents();
	void	UpdateMixedEvents(CEvent Ev);

	CBlockMaxEvent				m_BlockMaxEvent;
	CEvent						m_BMMixedEvent;
	std::vector<CBlockMaxEvent>	m_vBMEventsBuffer;
	std::vector<CEvent>			m_vMixedEvents;

	std::string m_MixedEventFile;
	
	unsigned int m_MaxEvTypesForBridge;
	unsigned int m_BlockSize;
	unsigned int m_CurBlockNo;
	unsigned int m_CurEventNoVehicles;
	
	int		BLOCK_SIZE_SECS;
	int		BLOCK_SIZE_DAYS;
	bool	WRITE_BM_MIXED;
};