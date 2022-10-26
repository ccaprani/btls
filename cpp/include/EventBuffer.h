// EventBuffer.h: interface for the CEventBuffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EVENTBUFFER_H__55D47416_48E5_47C4_B36E_B9227C33D7DC__INCLUDED_)
#define AFX_EVENTBUFFER_H__55D47416_48E5_47C4_B36E_B9227C33D7DC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include "Event.h"
#include "Effect.h"
#include "Vehicle.h"
#include "PyConfigData.h"

class CEventBuffer
{
public:
	CEventBuffer();
	CEventBuffer(CPyConfigData& pyConfig);
	CEventBuffer(std::string OutFile);
	CEventBuffer(std::string OutFile, CPyConfigData& pyConfig);
	virtual ~CEventBuffer();

	void setMode(bool bFatigue);
	void FlushBuffer();
	void SetBufferSize(int size);
	void AddEvent(CEvent Ev);
	void setOutFile(double BridgeLength);
	void setOutFile(std::string OutFile);

private:
	void FlushAllEventsBuff();
	void FlushFatigueBuff();

	enum Mode {ALLEVENTS, FATIGUE};
	Mode m_Mode; 
	std::vector<CEvent> m_vEvents;
	std::ofstream m_OutFile;
	int m_BufferSize;
	int m_NoEvents;
	double m_BridgeLength;
	template <typename T> std::string to_string(T const& value);
};

#endif // !defined(AFX_EVENTBUFFER_H__55D47416_48E5_47C4_B36E_B9227C33D7DC__INCLUDED_)
