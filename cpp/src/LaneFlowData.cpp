#include "LaneFlowData.h"


CLaneFlowData::CLaneFlowData(CConfigDataCore& config)
	: CModelData(config)
	, m_TruckClassCount(0)
	, m_BlockSize(3600)
	, m_NoBlocks(24) // MAGIC NUMBER - default to older type file format
{

}

CLaneFlowData::~CLaneFlowData()
{
}

CLaneFlowComposition CLaneFlowData::getLaneComp(size_t i) const
{
	return m_vLaneComp.at(i);
}

void CLaneFlowData::ReadDataIn()
{
	std::filesystem::path file = m_Config.Road.LANES_FILE;
	ReadLaneFlow(file.string());
	SetRoadProperties();
}

void CLaneFlowData::ReadLaneFlow(std::string file)
{
	if (!m_CSV.OpenFile(file, ","))
		return;
	std::string line;

	// get first line and check if it the old file type or the new file type
	// distinguished by the values of the second field being > 900 secs (min block size)
	bool ReadingLine1 = true;
	m_CSV.getline(line);
	size_t blockSize = m_CSV.stringToInt(m_CSV.getfield(1));
	if (blockSize > 900) // MAGIC NUMBER - min block size
	{
		m_BlockSize = blockSize;
		m_NoBlocks = m_CSV.stringToInt(m_CSV.getfield(0));
		ReadingLine1 = false; // so that a new line is read
	}	

	// Take advantage of short-circuiting where right experssion only evaluated if left
	// expression is not true
	// while another lane
	while (ReadingLine1 || m_CSV.getline(line))	
	{
		int laneNo = m_CSV.stringToInt(m_CSV.getfield(0)) - 1; // -1: zero-based
		int dir = m_CSV.stringToInt(m_CSV.getfield(1));
		CLaneFlowComposition lfc(laneNo, dir, m_BlockSize);

		for (size_t i = 0; i < m_NoBlocks; ++i)
		{
			if (m_CSV.getline(line))
			{
				lfc.addBlockData(m_CSV.GetVectorFromCurrentLine());
				if (i == 0)
					m_TruckClassCount = lfc.getTruckClassCount();
				else 
					if (m_TruckClassCount != lfc.getTruckClassCount())
					{
						std::cout << "ERROR: inconsistent truck class counts " << lfc.getGlobalLaneNo()
							<< "  - hour/block " << i << std::endl;
					};
			}
			else
				std::cout << "ERROR: not enough hours/blocks in lane " << lfc.getGlobalLaneNo()
					<< "  - no flow data beyond hour/block " << i << std::endl;
		}
		lfc.completeData();
		m_vLaneComp.push_back(lfc);
		// we are no longer reading first line so can get the next line
		ReadingLine1 = false; 
	}
	m_CSV.CloseFile();
}

void CLaneFlowData::SetRoadProperties()
{
	m_NoLanes = m_vLaneComp.size();
	m_NoDir = 0;
	m_NoLanesDir1 = 0;
	m_NoLanesDir2 = 0;
	for (size_t i = 0; i < m_NoLanes; ++i)
	{
		size_t cur_dir = m_vLaneComp.at(i).getDirn();
		if (cur_dir > m_NoDir) m_NoDir = cur_dir;
		if (cur_dir == 1)
			++m_NoLanesDir1;
		else
			++m_NoLanesDir2;
	}
}
