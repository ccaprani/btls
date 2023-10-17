#include "BridgeFile.h"


CBridgeFile::CBridgeFile(CConfigDataCore& config, std::vector<CInfluenceLine> vDiscreteIL, 
	std::vector<CInfluenceLine> vInfSurf) : m_Config(config)
{
	m_CommentString = "//";
	ReadBridges(m_Config.Sim.BRIDGE_FILE, vDiscreteIL, vInfSurf);
}

CBridgeFile::~CBridgeFile(void)
{

}

void CBridgeFile::ReadBridges(std::string file, std::vector<CInfluenceLine> vDiscreteIL, std::vector<CInfluenceLine> vInfSurf)
{

	if( !m_CSV.OpenFile(file, ",") )
	{
		std::cerr << "***ERROR: Bridge file could not be opened" << std::endl;
		system("PAUSE");
		exit( 1 );
	}
	std::string line;

	while (GetNextDataLine(line))	// while another bridge
	{
		CBridge_sp pBridge = std::make_shared<CBridge>(m_Config); //new CBridge;
		pBridge->setIndex( m_CSV.stringToInt( m_CSV.getfield(0) ) );
		pBridge->setLength(m_CSV.stringToDouble( m_CSV.getfield(1) ) );
		int NoLanes = m_CSV.stringToInt( m_CSV.getfield(2) );
		pBridge->InitializeLanes(NoLanes);
		int NoLoadEffects = m_CSV.stringToInt( m_CSV.getfield(3) );
		pBridge->setNoLoadEffects(NoLoadEffects);
		
		//pBridge->InitializeDataMgr();	// this needs to come after thresholds are set
		std::vector<double> vThresholds(NoLoadEffects,0.0);

		for(int i = 0; i < NoLoadEffects; i++)
			vThresholds.at(i) = ReadLoadEffect(pBridge, vDiscreteIL, vInfSurf);
	
		pBridge->setThresholds(vThresholds);

		m_vpBridge.push_back(pBridge);
	} // end of while loop
}

// New bridge file structure
double CBridgeFile::ReadLoadEffect(CBridge_sp pBridge, std::vector<CInfluenceLine> vDiscreteIL, std::vector<CInfluenceLine> vInfSurf)
{
	std::string line;
	GetNextDataLine(line); // m_CSV.getline(line);

	if( m_CSV.getnfield() < 2 )
	{
		std::cerr << "***ERROR: Bridge definition wrong at: " << line << std::endl;
		system("PAUSE");
		exit( 1 );
	}
	int index = m_CSV.stringToInt( m_CSV.getfield(0) );
	int type = m_CSV.stringToInt( m_CSV.getfield(1) );	
	double threshold = 0.0;
	
	if(m_CSV.getnfield() == 3)
		threshold = m_CSV.stringToDouble( m_CSV.getfield(2) );
	else
		std::cerr << "***WARNING: Bridge definition - assumed zero threhold at: " << line << std::endl;
	
	size_t NoLanes = pBridge->getNoLanes();

	switch(type)
	{
	case 2:	// Seperate ILs and weights for each lane
		{
			for (size_t i = 0; i < NoLanes; i++)
			{
				GetNextDataLine(line); // m_CSV.getline(line);
				int ILtype = m_CSV.stringToInt( m_CSV.getfield(0) );
				size_t ILno = m_CSV.stringToInt(m_CSV.getfield(1));
				double ILweight = m_CSV.stringToDouble( m_CSV.getfield(2) );

				CInfluenceLine IL;
				if(ILtype == 2)	// discrete IL
				{
					if(IL.getLength() > pBridge->getLength())
						std::cout << "**WARNING: Inf Line " << index << " longer than bridge " << pBridge->getIndex() << std::endl;
					if(ILno > vDiscreteIL.size())
						std::cout << "**ERROR: IL does not exist. Bridge " << pBridge->getIndex() << " : Load effect " << index << std::endl;
					
					IL = vDiscreteIL.at(ILno-1);
				}
				else			// IL function
					IL.setIL(ILno,pBridge->getLength());

				IL.setIndex(index);

				pBridge->addBridgeLaneLoadEffect(i,IL,ILweight);
			}
		} break;

	case 3:		// Influence surface for bridge
		{
			GetNextDataLine(line); // m_CSV.getline(line);
			size_t ISno = m_CSV.stringToInt(m_CSV.getfield(0));

			if(ISno > vInfSurf.size())
				std::cout << "**ERROR: Influence Surface does not exist. Bridge " << pBridge->getIndex() << " : Load effect " << index << std::endl;
					
			for (size_t i = 0; i < NoLanes; i++)
				pBridge->addBridgeLaneLoadEffect(i,vInfSurf.at(ISno-1),1.0);

		} break;

	case 1:		// lane weights on a single IL
	default:
		{
			GetNextDataLine(line); // m_CSV.getline(line);
			int ILtype = m_CSV.stringToInt( m_CSV.getfield(0) );
			size_t ILno = m_CSV.stringToInt(m_CSV.getfield(1));

			CInfluenceLine IL;
			if(ILtype == 2)	// discrete IL
			{
				if(IL.getLength() > pBridge->getLength())
					std::cout << "**WARNING: Inf Line " << index << " longer than bridge " << pBridge->getIndex() << std::endl;
				if(ILno > vDiscreteIL.size())
					std::cout << "**ERROR: IL does not exist. Bridge " << pBridge->getIndex() << " : Load effect " << index << std::endl;
					
				IL = vDiscreteIL.at(ILno-1);
			}
			else			// IL function
				IL.setIL(ILno,pBridge->getLength());

			IL.setIndex(index);

			for (size_t j = 0; j < NoLanes; j++)
			{
				double weight = m_CSV.stringToDouble(m_CSV.getfield(2 + j));
				pBridge->addBridgeLaneLoadEffect(j,IL,weight);
			}
		}
	}

	return threshold;
}

std::vector<CBridge_sp> CBridgeFile::getBridges()
{
	return m_vpBridge;
}

double CBridgeFile::getMaxBridgeLength(void)
{
	double maxL = 0.0;	// should not be negative!
	for(unsigned int i = 0; i < m_vpBridge.size(); ++i)
	{
		double L = m_vpBridge.at(i)->getLength();
		if(L > maxL) maxL = L;
	}
	return maxL;
}

int CBridgeFile::GetNextDataLine(std::string& str)
{
	int ret;
	ret = m_CSV.getline(str);
	while (str.substr(0, 2) == m_CommentString)
		ret = m_CSV.getline(str);
	return ret;
}