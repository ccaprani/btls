// BlockMaxEvent.cpp: implementation of the CBlockMaxEvent class.
//
//////////////////////////////////////////////////////////////////////

#include "BlockMaxEvent.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBlockMaxEvent::CBlockMaxEvent()
{
	m_NoEvents = 0;
	m_ID = 0;
}

CBlockMaxEvent::~CBlockMaxEvent()
{

}

void CBlockMaxEvent::clear()
{
	m_vEvents.clear();
	m_NoEvents = 0;
	m_ID = 0;
}

void CBlockMaxEvent::setNoLoadEffects(size_t nLE)
{
	m_NoLoadEffects = nLE;
}

void CBlockMaxEvent::AddExtraEvents(unsigned int nVehs)
{
	while(m_NoEvents < nVehs)
	{
		// add the event to the vector
		m_NoEvents++;
		CEvent temp(m_ID,m_NoLoadEffects);
		m_vEvents.push_back(temp);
	}
}

void CBlockMaxEvent::UpdateEvent(unsigned int iEvent, CEvent Ev)
{
	if(iEvent < m_NoEvents)
		m_vEvents[iEvent] = Ev;
	else
		std::cout << "*** ERROR in CBlockMaxEvent" << std::endl;
}

void CBlockMaxEvent::setID(unsigned int ID)
{
	m_ID = ID;
}

CEvent& CBlockMaxEvent::getEvent(unsigned int iEv)
{
	return m_vEvents[iEv];
}

unsigned int CBlockMaxEvent::getSize()
{
	return m_NoEvents;
}

unsigned int CBlockMaxEvent::getID()
{
	return m_ID;
}


