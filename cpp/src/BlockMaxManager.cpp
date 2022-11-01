// BlockMaxManager.cpp: implementation of the CBlockMaxManager class.
//
//////////////////////////////////////////////////////////////////////

#include "BlockMaxManager.h"
#include "ConfigData.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBlockMaxManager::CBlockMaxManager(CConfigDataCore& config) 
	: COutputManagerBase("BM")
	, m_BlockMaxEvent(config.Output.VehicleFile.FILE_FORMAT)
	, m_BMMixedEvent(config.Output.VehicleFile.FILE_FORMAT)
{
	BLOCK_SIZE_DAYS		= config.Output.BlockMax.BLOCK_SIZE_DAYS;
	BLOCK_SIZE_SECS		= config.Output.BlockMax.BLOCK_SIZE_SECS;

	WRITE_BUFFER_SIZE	= config.Output.BlockMax.WRITE_BM_BUFFER_SIZE;
	WRITE_VEHICLES		= config.Output.BlockMax.WRITE_BM_VEHICLES;
	WRITE_SUMMARY		= config.Output.BlockMax.WRITE_BM_SUMMARY;
	WRITE_BM_MIXED		= config.Output.BlockMax.WRITE_BM_MIXED;

	FILE_FORMAT 		= config.Output.VehicleFile.FILE_FORMAT;

	m_BlockSize = 3600 * 24 * BLOCK_SIZE_DAYS + BLOCK_SIZE_SECS;
	m_CurBlockNo = 1;
	m_MaxEvTypesForBridge = 0;
	m_CurEventNoVehicles = 0;

	m_BlockMaxEvent.setID(m_CurBlockNo);
	m_BMMixedEvent.setID(m_CurBlockNo);
}

CBlockMaxManager::~CBlockMaxManager()
{

}

void CBlockMaxManager::Initialize(double BridgeLength, size_t nLE, double SimStartTime)
{
	m_SimStartTime = SimStartTime;
	m_BridgeLength = BridgeLength;
	m_NoLoadEffects = nLE;
	m_BlockMaxEvent.setNoLoadEffects(m_NoLoadEffects);
	m_BMMixedEvent.setNoEffects(m_NoLoadEffects);

	if(WRITE_SUMMARY)
		OpenSummaryFiles();

	if(WRITE_VEHICLES)
		OpenVehicleFiles();

	if(WRITE_BM_MIXED)
	{
		// This is the mixed event output file
		m_MixedEventFile = m_FileStem + "_V_" + to_string(m_BridgeLength) + "_All.txt";
		ofstream outFile( m_MixedEventFile.c_str(), ios::out ); outFile.close();
	}
}

void CBlockMaxManager::Update(CEvent curEvent)
{
	double curTime = curEvent.getStartTime();
	
	if (curTime - m_SimStartTime > (double)(m_CurBlockNo)*m_BlockSize )
		CheckBuffer(false);	// at the end of a block

	m_CurEventNoVehicles = curEvent.getNoVehicles();
	if(m_CurEventNoVehicles > 0)
	{
		if(m_CurEventNoVehicles > m_BlockMaxEvent.getSize())
			AddExtraEvents();

		for(unsigned int k = 0; k < m_NoLoadEffects; k++)
		{	
			CEvent& BMEvent	= m_BlockMaxEvent.getEvent(m_CurEventNoVehicles-1);
			CEffect& curEff	= curEvent.getMaxEffect(k);
			CEffect& BMEff  = BMEvent.getMaxEffect(k);
			double curVal = curEff.getValue();
			double prevVal = BMEff.getValue();
			if(fabs(curVal) >= fabs(prevVal))
				BMEvent.setMaxEffect(curEvent.m_vMaxEffects[k],k);
		}
	}
	else
		std::cout << endl << "*** No trucks: block maxima error at " 
			<< curTime << " s, block no. " << m_CurBlockNo << endl;

	// For mixed event output
	UpdateMixedEvents(curEvent);
}

void CBlockMaxManager::UpdateMixedEvents(CEvent Ev)
{
	for(unsigned int k = 0; k < m_NoLoadEffects; k++)
	{	
		CEffect& curEff	= Ev.getMaxEffect(k);
		CEffect& BMEff  = m_BMMixedEvent.getMaxEffect(k);
		double curVal = curEff.getValue();
		double prevVal = BMEff.getValue();
		if(fabs(curVal) >= fabs(prevVal))
			m_BMMixedEvent.setMaxEffect(Ev.m_vMaxEffects[k],k);
	}
}

void CBlockMaxManager::AddExtraEvents()
{
	if(WRITE_VEHICLES)
		OpenVehicleFiles();
	
	// And now add scope for the extra event types
	m_BlockMaxEvent.AddExtraEvents(m_CurEventNoVehicles);
}

void CBlockMaxManager::OpenVehicleFiles()
{
	size_t curNoEvents = m_BlockMaxEvent.getSize();
	while(curNoEvents < m_CurEventNoVehicles)
	{
		curNoEvents++;
		// prepare the outfile only if one doesn't already exist
		if(curNoEvents > m_MaxEvTypesForBridge)
		{
			OpenVehicleFile(curNoEvents);
			m_MaxEvTypesForBridge = curNoEvents;
		}	
	}
}

void CBlockMaxManager::CheckBuffer(bool bForceOutput)
{
	// finish allows for block max buffers greater than the simulation length

	// store last block & see if we need to write
	m_vBMEventsBuffer.push_back(m_BlockMaxEvent);
	m_vMixedEvents.push_back(m_BMMixedEvent);	// for the mixed output
	if(m_vBMEventsBuffer.size() == WRITE_BUFFER_SIZE || bForceOutput)
		WriteBuffer();
	
	// clear data and update for next block
	m_CurBlockNo++;
	m_BlockMaxEvent.clear();
	m_BlockMaxEvent.setID(m_CurBlockNo);

	CEvent ev(FILE_FORMAT, m_CurBlockNo, m_NoLoadEffects);
	m_BMMixedEvent = ev;	// reset the mixed event
}

void CBlockMaxManager::WriteBuffer()
{
	// call base class implementation first
	COutputManagerBase::WriteBuffer();

	if(WRITE_BM_MIXED)
	{
		for(unsigned int iBlock = 0; iBlock < m_vMixedEvents.size(); iBlock++)
		{
			CEvent& Ev = m_vMixedEvents[iBlock];
			Ev.writeToFile(m_MixedEventFile);
		}
	}

	m_vBMEventsBuffer.clear();
	m_vMixedEvents.clear();
}

void CBlockMaxManager::WriteVehicleFiles()
{
	for(unsigned int iBlock = 0; iBlock < m_vBMEventsBuffer.size(); iBlock++)
	{
		CBlockMaxEvent BMEv = m_vBMEventsBuffer[iBlock];
			
		for(unsigned int iEv = 0; iEv < BMEv.getSize(); iEv++)
		{
			CEvent& Ev = BMEv.getEvent(iEv);
			Ev.writeToFile(m_vOutFiles[iEv]);
		}
	}
}

void CBlockMaxManager::WriteSummaryFiles()
{
	for(unsigned int iLE = 0; iLE < m_NoLoadEffects; iLE++)
	{
		ofstream outFile( m_vSummaryFiles[iLE].c_str(), ios::app );
		
		for(unsigned int iBlock = 0; iBlock < m_vBMEventsBuffer.size(); iBlock++)
		{
			CBlockMaxEvent BMEv = m_vBMEventsBuffer[iBlock];

			outFile << BMEv.getID() << '\t';
		
			for(unsigned int iEv = 0; iEv < BMEv.getSize(); iEv++)
			{
				CEvent& Ev = BMEv.getEvent(iEv);
				string val = to_string(Ev.getMaxEffect(iLE).getValue(),1);
				outFile << val << '\t' << '\t';
			}
			outFile << endl;
		}

		outFile.close();
	}
}