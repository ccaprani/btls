#include "StatsManager.h"
#include "ConfigData.h"

extern CConfigData g_ConfigData;

CStatsManager::CStatsManager(void)
{
	m_FileStem = "SS";

	WRITE_SS_CUMULATIVE		= g_ConfigData.Output.Stats.WRITE_SS_CUMULATIVE;
	WRITE_SS_INTERVALS		= g_ConfigData.Output.Stats.WRITE_SS_INTERVALS;
	WRITE_SS_INTERVAL_SIZE	= g_ConfigData.Output.Stats.WRITE_SS_INTERVAL_SIZE;
	WRITE_BUFFER_SIZE		= g_ConfigData.Output.Stats.WRITE_SS_BUFFER_SIZE;

	// inhereted from base class, just set to false
	WRITE_SUMMARY = false;
	WRITE_VEHICLES = false;
	
	m_CurIntervalNo = 1;
}


CStatsManager::~CStatsManager(void)
{
}


void CStatsManager::Initialize(double BridgeLength,size_t nLE)
{
	m_BridgeLength = BridgeLength;
	m_NoLoadEffects = nLE;

	CEventStatistics temp; 
	temp.m_ID = m_CurIntervalNo;
	m_vIntervalStats.assign(m_NoLoadEffects, temp);
	m_vCumulativeStats.assign(m_NoLoadEffects, temp);

	// here we assume is stats are being calculated that they are to be ouput
	if(WRITE_SS_INTERVALS)
	{
		OpenSummaryFiles();
		WriteIntervalHeadings();
	}
}

void CStatsManager::Update(CEvent curEvent)
{
	m_CurTime = curEvent.getStartTime();
	
	if(m_CurTime > (double)(m_CurIntervalNo)*WRITE_SS_INTERVAL_SIZE && WRITE_SS_INTERVALS)
		CheckBuffer(false);	// at the end of a block

	if(curEvent.getNoVehicles() > 0)
	{
		for(unsigned int i = 0; i < m_NoLoadEffects; i++)
		{
			m_vIntervalStats.at(i).update(curEvent,i);
			m_vCumulativeStats.at(i).update(curEvent,i);
		}
	}
	else
		std::cout << std::endl << "*** No trucks: summary statistics error at " 
			<< m_CurTime << " s, interval no. " << m_CurIntervalNo << std::endl;

}

void CStatsManager::CheckBuffer(bool bForceOutput)
{
	if(m_vIntStatsBuffer.size() == WRITE_BUFFER_SIZE || bForceOutput)
		WriteBuffer();

	if(bForceOutput && WRITE_SS_CUMULATIVE)
		WriteCumulativeFile();
	
	// store data and update for next interval
	m_vIntStatsBuffer.push_back(m_vIntervalStats);
	
	m_CurIntervalNo++;

	m_vIntervalStats.clear();
	CEventStatistics temp; 
	temp.m_ID = m_CurIntervalNo;
	m_vIntervalStats.assign(m_NoLoadEffects, temp);
}

void CStatsManager::WriteBuffer()
{
	// call base class implementation first
	//COutputManagerBase::WriteBuffer();
	if(WRITE_SS_INTERVALS)
	{
		WriteSummaryFiles();
		// clear the buffer
		m_vIntStatsBuffer.clear();
	}
}

void CStatsManager::WriteSummaryFiles()
{
	for(unsigned int iLE = 0; iLE < m_NoLoadEffects; iLE++)
	{
		std::ofstream outFile( m_vSummaryFiles[iLE].c_str(), std::ios::app );
		
		for(unsigned int i = 0; i < m_vIntStatsBuffer.size(); i++)
		{
			CEventStatistics s = m_vIntStatsBuffer.at(i).at(iLE);
			
			std::ostringstream oStr;
			oStr.width(6);		oStr << s.m_ID;
			oStr.width(15);		oStr << std::fixed << std::setprecision(1) << WRITE_SS_INTERVAL_SIZE*s.m_ID;
			oStr << std::ends;
	
			outFile << oStr.str() << s.outputString() << std::endl;
		}
		outFile.close();
	}
}

void CStatsManager::WriteIntervalHeadings()
{
	CEventStatistics s;
	for(unsigned int iLE = 0; iLE < m_NoLoadEffects; iLE++)
	{
		std::ofstream outFile( m_vSummaryFiles[iLE].c_str(), std::ios::app );
		outFile << "    ID" << "        Time(s) " << s.headingsString() << std::endl;
		outFile.close();
	}
}

void CStatsManager::WriteCumulativeFile()
{
	std::string file = m_FileStem + "_C_" + to_string(m_BridgeLength) + ".txt";
	std::ofstream outFile( file.c_str(), std::ios::out );

	CEventStatistics s;
	outFile << "    LE " << s.headingsString() << std::endl;

	for(unsigned int iLE = 0; iLE < m_NoLoadEffects; iLE++)
	{
		s = m_vCumulativeStats.at(iLE);

		std::ostringstream oStr;
		oStr.width(6);		oStr << iLE+1 << std::ends;
	
		outFile << oStr.str() << s.outputString() << std::endl;
	}

	outFile.close();
}