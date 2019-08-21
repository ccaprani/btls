// BlockMaxEvent.h: interface for the CBlockMaxEvent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BLOCKMAXEVENT_H__3BB07B26_8B47_4A0A_A256_19A1246CA613__INCLUDED_)
#define AFX_BLOCKMAXEVENT_H__3BB07B26_8B47_4A0A_A256_19A1246CA613__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include "Event.h"

class CBlockMaxEvent  
{
public:
	CBlockMaxEvent();
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
};

#endif // !defined(AFX_BLOCKMAXEVENT_H__3BB07B26_8B47_4A0A_A256_19A1246CA613__INCLUDED_)
