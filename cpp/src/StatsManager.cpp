#include "StatsManager.h"
#include "FilePath.h"


CStatsManager::CStatsManager(CConfigDataCore& config) : COutputManagerBase("SS")
{
	m_OutputDir = config.Output.OUTPUT_DIR;
	WRITE_SS_CUMULATIVE		= config.Output.Stats.WRITE_SS_CUMULATIVE;
	WRITE_SS_INTERVALS		= config.Output.Stats.WRITE_SS_INTERVALS;
	WRITE_SS_INTERVAL_SIZE	= config.Output.Stats.WRITE_SS_INTERVAL_SIZE;
	WRITE_BUFFER_SIZE	= config.Output.Stats.WRITE_SS_BUFFER_SIZE;

	// inhereted from base class, just set to false
	WRITE_SUMMARY = false;
	WRITE_VEHICLES = false;

	m_CurIntervalNo = 1;
}

CStatsManager::~CStatsManager(void)
{

}

void CStatsManager::Initialize(double BridgeLength,size_t nLE, double SimStartTime)
{
	m_SimStartTime = SimStartTime;
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

void CStatsManager::Update(CEvent& curEvent)
{
	m_CurTime = curEvent.getStartTime();
	
	// a zero interval size would make the rollover test permanently true
	while( WRITE_SS_INTERVAL_SIZE > 0 && m_CurTime - m_SimStartTime > (double)(m_CurIntervalNo)*WRITE_SS_INTERVAL_SIZE && WRITE_SS_INTERVALS )
		CheckBuffer(false);	// at the end of a block; while, not if: fill in any silent intervals

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

void CStatsManager::FinishAt(double simEndTime)
{
	// fill any silent trailing intervals up to the simulated end time
	while( WRITE_SS_INTERVAL_SIZE > 0 && simEndTime - m_SimStartTime > (double)(m_CurIntervalNo)*WRITE_SS_INTERVAL_SIZE && WRITE_SS_INTERVALS )
		CheckBuffer(false);

	// an event can start after the end of the simulated window (the bridge is
	// run on until it empties), which has already rolled the interval counter
	// past the last interval of the window; fold that interval back so that the
	// window still produces exactly one row per interval
	while( WRITE_SS_INTERVAL_SIZE > 0 && WRITE_SS_INTERVALS && m_CurIntervalNo > 1
		&& !m_vIntStatsBuffer.empty()
		&& simEndTime - m_SimStartTime <= (double)(m_CurIntervalNo-1)*WRITE_SS_INTERVAL_SIZE )
		FoldBackInterval();

	Finish();
}

// Combine two interval accumulators - the parallel form of the online update
// in CEventStatistics::accumulator(), so that merging two intervals gives the
// moments the interval would have had if it had never been split. The merged
// accumulator is written straight out and never update()d again.
static void MergeStats(CEventStatistics& Dest, const CEventStatistics& Src)
{
	if(Src.m_N == 0)
		return;
	if(Dest.m_N == 0)
	{
		size_t ID = Dest.m_ID;
		Dest = Src;
		Dest.m_ID = ID;
		return;
	}

	double na = (double)Dest.m_N;
	double nb = (double)Src.m_N;
	double n  = na + nb;
	double d  = Src.m_Mean - Dest.m_Mean;

	double M2 = Dest.m_M2 + Src.m_M2 + d*d*na*nb/n;
	double M3 = Dest.m_M3 + Src.m_M3 + d*d*d*na*nb*(na-nb)/(n*n)
			+ 3.0*d*(na*Src.m_M2 - nb*Dest.m_M2)/n;
	double M4 = Dest.m_M4 + Src.m_M4 + d*d*d*d*na*nb*(na*na - na*nb + nb*nb)/(n*n*n)
			+ 6.0*d*d*(na*na*Src.m_M2 + nb*nb*Dest.m_M2)/(n*n)
			+ 4.0*d*(na*Src.m_M3 - nb*Dest.m_M3)/n;

	Dest.m_Mean += d*nb/n;
	Dest.m_M2 = M2;
	Dest.m_M3 = M3;
	Dest.m_M4 = M4;
	Dest.m_N += Src.m_N;

	if(Src.m_Max > Dest.m_Max) Dest.m_Max = Src.m_Max;
	if(Src.m_Min < Dest.m_Min) Dest.m_Min = Src.m_Min;

	Dest.m_NoVehicles += Src.m_NoVehicles;
	Dest.m_NoTrucks += Src.m_NoTrucks;
	if(Src.m_vNoTrucksInEvent.size() > Dest.m_vNoTrucksInEvent.size())
		Dest.m_vNoTrucksInEvent.resize(Src.m_vNoTrucksInEvent.size(), 0);
	for(size_t i = 0; i < Src.m_vNoTrucksInEvent.size(); i++)
		Dest.m_vNoTrucksInEvent.at(i) += Src.m_vNoTrucksInEvent.at(i);
}

void CStatsManager::FoldBackInterval()
{
	// take the last completed interval back out of the write buffer and merge
	// the over-run interval into it, so Finish() writes it as the last interval
	std::vector<CEventStatistics> lastInterval = m_vIntStatsBuffer.back();
	m_vIntStatsBuffer.pop_back();

	for(unsigned int i = 0; i < m_NoLoadEffects; i++)
		MergeStats(lastInterval.at(i), m_vIntervalStats.at(i));

	m_vIntervalStats = lastInterval;
	m_CurIntervalNo--;
}

void CStatsManager::CheckBuffer(bool bForceOutput)
{
	if(bForceOutput)
		// store the current (final) interval so it gets written too,
		// otherwise the last interval of every simulation is lost
		m_vIntStatsBuffer.push_back(m_vIntervalStats);

	if(m_vIntStatsBuffer.size() == WRITE_BUFFER_SIZE || bForceOutput)
		WriteBuffer();

	if(bForceOutput && WRITE_SS_CUMULATIVE)
		WriteCumulativeFile();

	if(bForceOutput)
		return;	// end of simulation - no next interval to prepare

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
			// oStr << std::ends;
	
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
	std::string file = btls::outPath(m_OutputDir, m_FileStem + "_C_" + to_string(m_BridgeLength) + ".txt");
	std::ofstream outFile( file.c_str(), std::ios::out );

	CEventStatistics s;
	outFile << "    LE " << s.headingsString() << std::endl;

	for(unsigned int iLE = 0; iLE < m_NoLoadEffects; iLE++)
	{
		s = m_vCumulativeStats.at(iLE);

		std::ostringstream oStr;
		oStr.width(6);		oStr << iLE+1;
	
		outFile << oStr.str() << s.outputString() << std::endl;
	}

	outFile.close();
}