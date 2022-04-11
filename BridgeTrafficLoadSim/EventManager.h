// EventManager.h: interface for the CEventManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EVENTMANAGER_H__3ED9F26C_A94D_4EA8_A87C_4DB2819160E5__INCLUDED_)
#define AFX_EVENTMANAGER_H__3ED9F26C_A94D_4EA8_A87C_4DB2819160E5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "math.h"
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include "Event.h"
#include "Effect.h"
#include "Vehicle.h"
#include "EventBuffer.h"
#include "BlockMaxManager.h"
#include "POTManager.h"
#include "StatsManager.h"

class CEventManager  
{
public:
	CEventManager();
	virtual ~CEventManager();

	void Initialize(double BridgeLength,std::vector<double> vThresholds, double SimStartTime);
	void EndEvent();
	void Finish();
	void UpdateEffects(std::vector<double> vEffs, double position, double time);
	void AddNewEvent(std::vector<CVehicle> vVehs, double curTime);
	void AddNewEvent(const std::vector<CVehicle_sp> vVehs, double curTime);
	void setEventOutputFile(double BridgeLength);

private:
	void WriteEventBuffer();
	CEventBuffer		m_AllEventBuffer;
	CEventBuffer		m_FatigueEventBuffer;
	CBlockMaxManager	m_BlockMaxManager;
	CPOTManager			m_POTManager;
	CStatsManager		m_StatsManager;
	
	std::vector<CVehicle> m_vVehicles;
	
	std::ofstream		m_TimeHistoryFile;
	CEvent				m_CurEvent;
	long				m_BlockSize;
	int					m_MaxNoVehsInEvent;
	int					m_CurEventType;	
	int					m_NoEvents;
	int					m_CurBlockNo;	
	double				m_CurTime;
	double				m_BridgeLength;
	size_t				m_NoLoadEffects;

	void	UpdateEvent();
	void	DoTimeHistory(int i, std::vector<double>& vEff);
	
	std::vector<double> m_vThresholds;

	bool	WRITE_TIME_HISTORY;
	bool	WRITE_EACH_EVENT;
	int		WRITE_EVENT_BUFFER_SIZE;
	bool	WRITE_FATIGUE_EVENT;

	bool	WRITE_BM;
	bool	WRITE_POT;
	bool	WRITE_STATS;

	template <typename T> std::string to_string(T const& value);
	template <typename T> std::string to_string(T const& value, int nDigits);
};

#endif // !defined(AFX_EVENTMANAGER_H__3ED9F26C_A94D_4EA8_A87C_4DB2819160E5__INCLUDED_)
