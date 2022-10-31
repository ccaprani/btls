#include "OutputManagerBase.h"


COutputManagerBase::COutputManagerBase(void)
{
}


COutputManagerBase::~COutputManagerBase(void)
{
}

void COutputManagerBase::Finish()
{
	CheckBuffer(true);
}

void COutputManagerBase::OpenSummaryFiles()
{
	for (size_t i = 0; i < m_NoLoadEffects; i++)
	{
		std::string file;
		file = m_FileStem + "_S_" + to_string(m_BridgeLength) + "_Eff_" + to_string(i+1) + ".txt";
		std::ofstream outFile( file.c_str(), std::ios::out ); outFile.close();
		m_vSummaryFiles.push_back(file);
	}
}

void COutputManagerBase::OpenVehicleFile(size_t i)
{
	std::string thefile;
	thefile = m_FileStem + "_V_" + to_string(m_BridgeLength) + "_" + to_string(i) + ".txt";
	// this clears anything already in the file if it exists.
	std::ofstream outFile( thefile.c_str(), std::ios::out ); outFile.close();
	// save the filename for later use.
	m_vOutFiles.push_back(thefile);
}

void COutputManagerBase::WriteBuffer()
{
	if(WRITE_SUMMARY)
		WriteSummaryFiles();
	
	if(WRITE_VEHICLES)
		WriteVehicleFiles();
}