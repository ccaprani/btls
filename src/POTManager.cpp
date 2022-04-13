#include "POTManager.h"
#include "ConfigData.h"

//extern CConfigData g_ConfigData;

CPOTManager::CPOTManager(void)
{
	m_FileStem = "PT";

	WRITE_BUFFER_SIZE	= CConfigData::get().Output.POT.WRITE_POT_BUFFER_SIZE;
	WRITE_VEHICLES		= CConfigData::get().Output.POT.WRITE_POT_VEHICLES;
	WRITE_SUMMARY		= CConfigData::get().Output.POT.WRITE_POT_SUMMARY;

	WRITE_POT_COUNTER	= CConfigData::get().Output.POT.WRITE_POT_COUNTER;
	POT_COUNT_SIZE_DAYS	= CConfigData::get().Output.POT.POT_COUNT_SIZE_DAYS;
	POT_COUNT_SIZE_SECS	= CConfigData::get().Output.POT.POT_COUNT_SIZE_SECS;

	m_BlockSize = 3600 * 24 * POT_COUNT_SIZE_DAYS + POT_COUNT_SIZE_SECS;
	m_CurBlockNo = 0;
}

CPOTManager::~CPOTManager(void)
{
}

void CPOTManager::Initialize(double BridgeLength, std::vector<double> vThreshold, double SimStartTime)
{
	m_SimStartTime = SimStartTime;
	m_BridgeLength = BridgeLength;
	m_vThreshold = vThreshold;
	m_NoLoadEffects = m_vThreshold.size();

	std::vector<CEvent> vEv;
	m_vEvents.assign(m_NoLoadEffects,vEv);

	UpdateCounter();

	if(WRITE_SUMMARY)
		OpenSummaryFiles();

	if(WRITE_VEHICLES)
		OpenVehicleFiles();

	if(WRITE_POT_COUNTER)
		OpenCounterFile();
}

void CPOTManager::Update(CEvent curEvent)
{
	double curTime = curEvent.getStartTime();
	
	if( curTime - m_SimStartTime > (double)(m_CurBlockNo)*m_BlockSize )
		UpdateCounter();

	size_t nEventVehs = curEvent.getNoVehicles();
	if(nEventVehs > 0)
	{
		for (size_t i = 0; i < m_NoLoadEffects; i++)
		{	
			if(curEvent.getMaxEffect(i).getValue() > m_vThreshold.at(i))
			{
				m_vEvents.at(i).push_back(curEvent);
				m_vCounter.back().at(i)++;
			}
		}
	}
	else
		std::cout << endl << "*** No trucks: POT error at " << curTime << " s" << endl;

	CheckBuffer(false);
}

void CPOTManager::CheckBuffer(bool bForceOutput)
{
	// finish allows for block max buffers greater than the simulation length

	size_t i = 0;
	while(bForceOutput == false && i < m_NoLoadEffects)
	{	
		if(m_vEvents.at(i).size() >= WRITE_BUFFER_SIZE)
			bForceOutput = true;
		i++;
	}

	if(bForceOutput)
	{
		WriteBuffer();
		// clear data
		for (size_t i = 0; i < m_NoLoadEffects; i++)
			m_vEvents.at(i).clear();
		
		m_vCounter.clear();
		UpdateCounter();
		m_CurBlockNo--; // remove the increment just done in UpdateCounter()
	}
}

void CPOTManager::UpdateCounter()
{
	m_CurBlockNo++;
	std::vector<unsigned int> temp(m_NoLoadEffects,0);
	m_vCounter.push_back(temp);
}

void CPOTManager::WriteBuffer()
{
	COutputManagerBase::WriteBuffer(); // call base class first

	if(WRITE_POT_COUNTER)
		WriteCounter();
}

void CPOTManager::OpenVehicleFiles()
{
	for (size_t i = 0; i < m_NoLoadEffects; i++)
		OpenVehicleFile(i+1);
}

void CPOTManager::OpenCounterFile()
{
	m_CounterFile = m_FileStem + "_C_" + to_string(m_BridgeLength) + ".txt";
	
	// this clears anything already in the file if it exists.
	std::ofstream outFile( m_CounterFile.c_str(), std::ios::out );
	
	outFile << "Block" << '\t';
	for (size_t iLE = 0; iLE < m_NoLoadEffects; iLE++)
		outFile << "LE " << iLE+1 << '\t';
	outFile << std::endl;
	
	outFile.close();
}

void CPOTManager::WriteCounter()
{
	std::ofstream outFile( m_CounterFile.c_str(), std::ios::app ); 

	size_t nBlocks = m_vCounter.size();
	size_t index = m_CurBlockNo - nBlocks + 1;
	for (size_t iBlock = 0; iBlock < nBlocks; iBlock++)
	{
		outFile << index + iBlock << '\t';
		for (size_t iLE = 0; iLE < m_NoLoadEffects; iLE++)
			outFile << m_vCounter.at(iBlock).at(iLE) << '\t';
		outFile << std::endl;
	}
	outFile.close();
}

void CPOTManager::WriteVehicleFiles()
{
	for (size_t i = 0; i < m_NoLoadEffects; i++)
	{
		for (size_t iEv = 0; iEv < m_vEvents.at(i).size(); iEv++)
		{
			CEvent& Ev = m_vEvents.at(i).at(iEv);
			Ev.setID(iEv+1);
			Ev.writeToFile(m_vOutFiles[i]);
		}
	}
}

void CPOTManager::WriteSummaryFiles()
{
	for (size_t iLE = 0; iLE < m_NoLoadEffects; iLE++)
	{
		ofstream outFile( m_vSummaryFiles[iLE].c_str(), ios::app );
		
		for (size_t iEv = 0; iEv < m_vEvents.at(iLE).size(); iEv++)
		{
			CEvent& Ev = m_vEvents.at(iLE).at(iEv);
			Ev.setCurEffect(iLE);
			
			std::ostringstream oStr;
			oStr.width(6);		oStr << iEv+1;
			oStr.width(15);		oStr << std::fixed << std::setprecision(1) << Ev.getMaxEffectTime();
			oStr.width(4);		oStr << Ev.getMaxEffect(iLE).m_NoVehicles;
			oStr.width(10);		oStr << std::fixed << std::setprecision(1) << Ev.getMaxEffect(iLE).getValue();	
			oStr << ends;
	
			outFile << oStr.str() << endl;
		}
		outFile.close();
	}
}