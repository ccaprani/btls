// BlockMaxEvent.cpp: implementation of the CBlockMaxEvent class.
//
//////////////////////////////////////////////////////////////////////

#include "BlockMaxEvent.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBlockMaxEvent::CBlockMaxEvent(size_t fileFormat) : FILE_FORMAT(fileFormat)
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

void CBlockMaxEvent::AddExtraEvents(size_t nVehs)
{
	while(m_NoEvents < nVehs)
	{
		// add the event to the vector
		m_NoEvents++;
		CEvent temp(FILE_FORMAT, m_ID, m_NoLoadEffects);
		m_vEvents.push_back(temp);
	}
}

void CBlockMaxEvent::UpdateEvent(size_t iEvent, CEvent Ev)
{
	if(iEvent < m_NoEvents)
		m_vEvents[iEvent] = Ev;
	else
		std::cout << "*** ERROR in CBlockMaxEvent" << std::endl;
}

void CBlockMaxEvent::setID(size_t ID)
{
	m_ID = ID;
}

CEvent& CBlockMaxEvent::getEvent(size_t iEv)
{
	return m_vEvents[iEv];
}

size_t CBlockMaxEvent::getSize()
{
	return m_NoEvents;
}

size_t CBlockMaxEvent::getID()
{
	return m_ID;
}


