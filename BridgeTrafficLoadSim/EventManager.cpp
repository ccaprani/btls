// EventManager.cpp: implementation of the CEventManager class.
//
//////////////////////////////////////////////////////////////////////

#include "EventManager.h"
#include "ConfigData.h"

extern CConfigData g_ConfigData;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEventManager::CEventManager()
{
	WRITE_TIME_HISTORY		= g_ConfigData.Output.WRITE_TIME_HISTORY;
	WRITE_EACH_EVENT		= g_ConfigData.Output.WRITE_EACH_EVENT;
	WRITE_EVENT_BUFFER_SIZE = g_ConfigData.Output.WRITE_EVENT_BUFFER_SIZE;
	WRITE_FATIGUE_EVENT		= g_ConfigData.Output.WRITE_FATIGUE_EVENT;

	WRITE_BM	= g_ConfigData.Output.BlockMax.WRITE_BM;
	WRITE_POT	= g_ConfigData.Output.POT.WRITE_POT;
	WRITE_STATS = g_ConfigData.Output.Stats.WRITE_STATS;

	m_NoEvents = 0;
	m_CurTime = 0.0;
	m_FatigueEventBuffer.setMode(true);
}

CEventManager::~CEventManager()
{

}

void CEventManager::Initialize(double BridgeLength, std::vector<double> vThresholds)
{
	m_BridgeLength = BridgeLength;
	m_vThresholds = vThresholds;
	m_NoLoadEffects = m_vThresholds.size();

	if(WRITE_EACH_EVENT)
		m_AllEventBuffer.setOutFile(m_BridgeLength);
	if(WRITE_FATIGUE_EVENT)
		m_FatigueEventBuffer.setOutFile(m_BridgeLength);
	
	if(WRITE_BM) m_BlockMaxManager.Initialize(m_BridgeLength,m_NoLoadEffects);
	if(WRITE_POT) m_POTManager.Initialize(m_BridgeLength,m_vThresholds);
	if(WRITE_STATS) m_StatsManager.Initialize(m_BridgeLength,m_NoLoadEffects);

	if(WRITE_TIME_HISTORY)
	{
		std::vector<double> temp(m_NoLoadEffects, 0.0);
		DoTimeHistory(1, temp);
	}
}

// called to commence new event
void CEventManager::AddNewEvent(std::vector<CVehicle> vVehs, double curTime)
{
	m_CurTime = curTime;

	m_vVehicles = vVehs;
	m_NoEvents++;
	m_CurEvent.setID(m_NoEvents);
	m_CurEvent.setNoEffects(m_NoLoadEffects);
	m_CurEvent.setStartTime(m_CurTime);
}

void CEventManager::AddNewEvent(std::vector<CVehicle*> pVehs, double curTime)
{
	// form local copy of vehs
	std::vector<CVehicle> vVehs;
	vVehs.reserve(pVehs.size());
	for(int i = 0; i != pVehs.size(); ++i)
		vVehs.push_back( *pVehs.at(i) );
	// call primary func
	AddNewEvent(vVehs, curTime);
}

// update the load effects found for the current time-step
void CEventManager::UpdateEffects(std::vector<double> vEffs, double position, double time)
{
	m_CurTime = time;
	std::vector<double> temp(m_NoLoadEffects, 0.0);
	
	// Detect maxima as matter of course
	for (size_t k = 0; k < m_NoLoadEffects; k++)
	{
		double curVal = vEffs[k];
		CEffect& Eff = m_CurEvent.getMaxEffect(k);
		double prevVal = Eff.getValue();
		if(fabs(curVal) >= fabs(prevVal))	// >= VI to include zero LEs at start of event
		{
			Eff.setValue(vEffs[k]);			// set the new higher value of LE
			Eff.setTime(m_CurTime);			// the time at which it occurs
			Eff.setPosition(position);		// the position of the first axle on the bridge
			Eff.AddVehicles(m_vVehicles);
		}
		
		if(WRITE_TIME_HISTORY)
			temp[k] = curVal;
	}
	
	// Only detect minima if a fatigue analysis is being done
	if(WRITE_FATIGUE_EVENT)
	{
		for (size_t k = 0; k < m_NoLoadEffects; k++)
		{
			double curVal = vEffs[k];
			CEffect& Eff = m_CurEvent.getMinEffect(k);
			double prevVal = Eff.getValue();
			if(fabs(curVal) <= fabs(prevVal))	// <= VI to include zero LEs at start of event
			{
				Eff.setValue(vEffs[k]);			// set the new value of LE
				Eff.setTime(m_CurTime);
				Eff.setPosition(position);
				Eff.AddVehicles(m_vVehicles);
			}
		}
	}

	if(WRITE_TIME_HISTORY)
		DoTimeHistory(2, temp);
}

// called to end the current event and process results
void CEventManager::EndEvent()
{
	// only if we are outputting every event
	if(WRITE_EACH_EVENT)
		m_AllEventBuffer.AddEvent(m_CurEvent);

	if(WRITE_FATIGUE_EVENT)
		m_FatigueEventBuffer.AddEvent(m_CurEvent);

	if(WRITE_BM) m_BlockMaxManager.Update(m_CurEvent);
	if(WRITE_POT) m_POTManager.Update(m_CurEvent);
	if(WRITE_STATS) m_StatsManager.Update(m_CurEvent);

	// reset for next event - must be last thing done
	CEvent temp;
	m_CurEvent = temp;
}

// called at the end of the simulation
void CEventManager::Finish()
{
	if(WRITE_EACH_EVENT)
		m_AllEventBuffer.FlushBuffer();

	if(WRITE_FATIGUE_EVENT)
		m_FatigueEventBuffer.FlushBuffer();
	
	if(WRITE_BM) m_BlockMaxManager.Finish();
	if(WRITE_POT) m_POTManager.Finish();
	if(WRITE_STATS) m_StatsManager.Finish();
}

void CEventManager::DoTimeHistory(int i, std::vector<double>& vEff)
{
	switch(i)
	{
	case 1:
		{
			string file;
			file = "TH_" + to_string(m_BridgeLength) + ".txt";
			m_TimeHistoryFile.open(file.c_str(),ios::out);
			m_TimeHistoryFile << std::setw(12) << "TIME (s)" << "\t\t" 
					<< "NO. TRUCKS" << "\t" << "EFFECTS" << endl;
			break;
		};
	case 2:
		{
			size_t nVehs = m_CurEvent.getMaxEffect(0).m_NoVehicles;
			m_TimeHistoryFile << std::setw(12) << fixed << setprecision (3) //14-6-19 changed to 3
				<< m_CurTime << "\t\t" << nVehs << "\t\t";
			for (size_t k = 0; k < m_NoLoadEffects; k++)
				m_TimeHistoryFile << vEff[k] << "\t\t";
			m_TimeHistoryFile << '\n';
			break;
		};
	}
}

template <typename T>
std::string CEventManager::to_string(T const& value)
{
    stringstream sstr;
    sstr << value;
    return sstr.str();
}

template <typename T>
std::string CEventManager::to_string(T const& value, int nDigits)
{
    stringstream sstr;
    sstr << std::fixed << std::setprecision(nDigits) << value;
    return sstr.str();
}