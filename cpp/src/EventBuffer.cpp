// EventBuffer.cpp: implementation of the CEventBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "EventBuffer.h"
#include "ConfigData.h"

//extern CConfigData g_ConfigData;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEventBuffer::CEventBuffer()
{
	m_BufferSize = CConfigData::get().Output.WRITE_EVENT_BUFFER_SIZE;

	SetBufferSize(m_BufferSize);
	m_NoEvents = 0;
	m_Mode = CEventBuffer::ALLEVENTS;
}

CEventBuffer::CEventBuffer(CPyConfigData& pyConfig)
{
	m_BufferSize = pyConfig.Output_WRITE_EVENT_BUFFER_SIZE;

	SetBufferSize(m_BufferSize);
	m_NoEvents = 0;
	m_Mode = CEventBuffer::ALLEVENTS;
}

CEventBuffer::CEventBuffer(std::string OutFile)
{
	m_BufferSize = CConfigData::get().Output.WRITE_EVENT_BUFFER_SIZE;

	m_OutFile.open(OutFile.c_str(), std::ios::out);
	SetBufferSize(m_BufferSize);
	m_NoEvents = 0;
	m_Mode = ALLEVENTS;
}

CEventBuffer::CEventBuffer(std::string OutFile, CPyConfigData& pyConfig)
{
	m_BufferSize = pyConfig.Output_WRITE_EVENT_BUFFER_SIZE;

	m_OutFile.open(OutFile.c_str(), std::ios::out);
	SetBufferSize(m_BufferSize);
	m_NoEvents = 0;
	m_Mode = ALLEVENTS;
}

CEventBuffer::~CEventBuffer()
{
	m_OutFile.close();
}

void CEventBuffer::setMode(bool bFatigue)
{
	if(bFatigue) 
		m_Mode = FATIGUE;
	else
		m_Mode = ALLEVENTS;
}

void CEventBuffer::setOutFile(std::string OutFile)
{
	m_OutFile.open(OutFile.c_str(), std::ios::out);
}

void CEventBuffer::setOutFile(double BridgeLength)
{
	string stem = m_Mode == ALLEVENTS ? "_AllEvents" : "_Fatigue";
	string OutFile;
	m_BridgeLength = BridgeLength;
	OutFile = "BL_" + to_string(m_BridgeLength) + stem + ".txt";
	m_OutFile.open(OutFile.c_str(), std::ios::out);
}

void CEventBuffer::AddEvent(CEvent Ev)
{
	if(m_NoEvents >= m_BufferSize)
		FlushBuffer();
	
	m_NoEvents++;
	m_vEvents.push_back(Ev);
}

void CEventBuffer::SetBufferSize(int size)
{
	m_BufferSize = size;
	m_vEvents.reserve(m_BufferSize);
}

void CEventBuffer::FlushBuffer()
{
	if(m_Mode == FATIGUE)
		FlushFatigueBuff();
	else
		FlushAllEventsBuff();
}


void CEventBuffer::FlushAllEventsBuff()
{
	size_t nEvs = m_vEvents.size();
	CEvent Ev = m_vEvents[nEvs-1];
	std::cout << std::endl << "Bridge " << to_string(m_BridgeLength) << " m: Flushing AllEvents buffer: " << nEvs << " events at " << Ev.getTimeStr() << '\t';
	
	for (size_t i = 0; i < nEvs; i++)
	{	
		CEvent& Ev = m_vEvents[i];

		m_OutFile << Ev.getStartTime() << '\t';
		m_OutFile << Ev.getNoVehicles() << '\t';
		for (size_t j = 0; j < Ev.getNoEffects(); j++)
			m_OutFile << Ev.getMaxEffect(j).getValue() << '\t';
		m_OutFile << '\n';
	}
	
	m_NoEvents = 0;
	m_vEvents.clear();
	m_vEvents.reserve(m_BufferSize);
}

void CEventBuffer::FlushFatigueBuff()
{
	// output structure is two lines per event, max, min in chronological order
	// first column is event start time and no. trucks in lines 1 & 2.

	size_t nEvs = m_vEvents.size();
	CEvent Ev = m_vEvents[nEvs-1];
	std::cout << std::endl << "Bridge " << to_string(m_BridgeLength) << " m: Flushing Fatigue buffer: " << nEvs << " events at " << Ev.getTimeStr() << '\t';
	
	for (size_t i = 0; i < nEvs; i++)
	{	
		CEvent& Ev = m_vEvents[i];
		ostringstream oStr; 

		// Write the first line
		oStr.width(15); oStr << std::left << std::fixed << std::setprecision(2) << Ev.getStartTime() << '\t';
		oStr << std::right;
		for (size_t j = 0; j < Ev.getNoEffects(); j++)
		{
			if( Ev.getMaxEffect(j).getTime() < Ev.getMinEffect(j).getTime() )
			{	
				oStr.width(15);		oStr << std::fixed << std::setprecision(2) << Ev.getMaxEffect(j).getTime() << '\t';
				oStr.width(10);		oStr << std::fixed << std::setprecision(2) << Ev.getMaxEffect(j).getValue() << '\t';
			}
			else
			{	
				oStr.width(15);		oStr << std::fixed << std::setprecision(2) << Ev.getMinEffect(j).getTime() << '\t';
				oStr.width(10); 	oStr << std::fixed << std::setprecision(2) << Ev.getMinEffect(j).getValue() << '\t';
			}
		}
		oStr << '\n';

		// And the second line
		oStr.width(15); oStr << std::fixed << std::setprecision(2) << Ev.getNoVehicles() << '\t';
		for (size_t j = 0; j < Ev.getNoEffects(); j++)
		{
			if( Ev.getMaxEffect(j).getTime() >= Ev.getMinEffect(j).getTime() )
			{	
				oStr.width(15);		oStr << std::fixed << std::setprecision(2) << Ev.getMaxEffect(j).getTime() << '\t';
				oStr.width(10);		oStr << std::fixed << std::setprecision(2) << Ev.getMaxEffect(j).getValue() << '\t';
			}
			else
			{	
				oStr.width(15);		oStr << std::fixed << std::setprecision(2) << Ev.getMinEffect(j).getTime() << '\t';
				oStr.width(10); 	oStr << std::fixed << std::setprecision(2) << Ev.getMinEffect(j).getValue() << '\t';
			}
		}
		m_OutFile << oStr.str() << '\n';
	}
	
	m_NoEvents = 0;
	m_vEvents.clear();
	m_vEvents.reserve(m_BufferSize);
}

template <typename T>
std::string CEventBuffer::to_string(T const& value)
{
    stringstream sstr;
    sstr << value;
    return sstr.str();
}
