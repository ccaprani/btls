// EventBuffer.h: interface for the CEventBuffer class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include "Event.h"
#include "Effect.h"
#include "Vehicle.h"
#include "ConfigData.h"

class CEventBuffer
{
public:
	CEventBuffer(size_t bufferSize);
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
	size_t m_BufferSize;
	size_t m_NoEvents;
	double m_BridgeLength;
	template <typename T> std::string to_string(T const& value);
};
