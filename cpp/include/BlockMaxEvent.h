// BlockMaxEvent.h: interface for the CBlockMaxEvent class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include "Event.h"

class CBlockMaxEvent  
{
public:
	CBlockMaxEvent(size_t file_format);
	virtual ~CBlockMaxEvent();
	
	void AddExtraEvents(size_t nVehs);
	void UpdateEvent(size_t iEvent, CEvent Ev);
	CEvent& getEvent(size_t iEv);
	void clear();
	void setID(size_t ID);
	size_t getID();
	size_t getSize();
	void setNoLoadEffects(size_t nLE);
	
private:
	size_t m_NoLoadEffects;
	size_t m_NoEvents;
	size_t m_ID;
	std::vector<CEvent>	m_vEvents;

	size_t FILE_FORMAT;
};
