#include "VehicleTrafficFile.h"


CVehicleTrafficFile::CVehicleTrafficFile(CVehicleClassification_sp pVC, 
	bool UseConstSpeed, bool UseAveSpeed, double ConstSpeed)
{
	m_NoVehs = 0;
	m_NoDays = 0;
	m_NoLanes = 0;
	m_NoDirn = 0;
	m_NoLanesDir1 = 0;
	m_NoLanesDir2 = 0;
	
	m_iCurVehicle = 0;

	m_pVehClassification = pVC;
	m_UseConstSpeed = UseConstSpeed;
	m_UseAveSpeed = UseAveSpeed;
	m_ConstSpeed = ConstSpeed;	
}


CVehicleTrafficFile::~CVehicleTrafficFile(void)
{
}

void CVehicleTrafficFile::Read(std::filesystem::path file, int filetype)
{
	std::ifstream inFile(file, std::ios::in); // Since C++17, can directly use std::filesystem::path
	if (!inFile)	// check to see if file was created
	{
		std::cout << "Input traffic file: " << file << " could not be opened" << std::endl;
		system("PAUSE");
		exit(1);
	}

	std::string str;
	while(std::getline(inFile, str))	// Improved reading loop
	{
		if(!str.empty())
		{
			CVehicle_sp pVeh = std::make_shared<CVehicle>();	//new CVehicle;
			pVeh->create(str, filetype);
			m_pVehClassification->setClassification(pVeh);
			m_vVehicles.push_back(pVeh);
		}
	}

	AnalyseTraffic();
}

void CVehicleTrafficFile::AssignTraffic(std::vector<CVehicle_sp> vVehicles)
{
	m_vVehicles = vVehicles;
	for (CVehicle_sp pVeh : m_vVehicles)
	{
		m_pVehClassification->setClassification(pVeh);
	}
	AnalyseTraffic();
}

void CVehicleTrafficFile::AnalyseTraffic()
{
	UpdateProperties();
	if(m_UseConstSpeed)
		SetSpeed();
}

void CVehicleTrafficFile::SetSpeed()
{
	
	{
		double speed = 0.0;
		if(m_UseAveSpeed)
		{
			for(unsigned int i = 0; i < m_NoVehs; ++i)
				speed += m_vVehicles.at(i)->getVelocity();
			speed /= m_NoVehs;
		}
		else
			speed = m_ConstSpeed/3.6; // km/h to m/s
		
		for(unsigned int i = 0; i < m_NoVehs; ++i)
			m_vVehicles.at(i)->setVelocity(speed);
	}
}

void CVehicleTrafficFile::UpdateProperties()
{
	m_NoVehs = m_vVehicles.size();

	size_t maxLaneNoDir1 = 0;
	size_t maxLaneNoDir2 = 0;

	for(unsigned int i = 0; i < m_NoVehs; ++i)
	{
		CVehicle_sp pVeh = m_vVehicles.at(i);
		size_t dirn = pVeh->getDirection();
		size_t lane = pVeh->getLocalLane();
		
		if(dirn == 1 && lane > maxLaneNoDir1) maxLaneNoDir1 = lane;
		if(dirn == 2 && lane > maxLaneNoDir2) maxLaneNoDir2 = lane;
	}
	
	// If both directions have lanes
	if(maxLaneNoDir1 > 0 && maxLaneNoDir2 > 0)
	{ 
		m_NoLanesDir1 = maxLaneNoDir1;
		// If vehicle lanes are cumulative then subtract dir 1 lanes from dir 2
		//m_NoLanesDir2 = maxLaneNoDir2 -maxLaneNoDir1;
		// Lnes are no longer cumulative
		m_NoLanesDir2 = maxLaneNoDir2;
		m_NoLanes = m_NoLanesDir1 + m_NoLanesDir2;
		m_NoDirn = 2;
	}
	else	// only one direction has lanes
	{
		if(maxLaneNoDir1 > 0)	// it's direction 1
		{
			m_NoLanesDir1 = maxLaneNoDir1;
			m_NoLanes = m_NoLanesDir1;
		}
		else // it's direction 2
		{
			m_NoLanesDir2 = maxLaneNoDir2;
			m_NoLanes = m_NoLanesDir2;
		}
		m_NoDirn = 1;
	}

	if(m_NoVehs > 0)
	{
		m_Starttime = m_vVehicles.front()->getTime();
		m_Endtime = m_vVehicles.back()->getTime();
		m_NoDays = (int)((m_Endtime - m_Starttime)/(3600.0*24)) + 1;
	}
	else
		std::cout << "*** ERROR: No vehicles in traffic file" << std::endl;
}

CVehicle_sp CVehicleTrafficFile::getNextVehicle()
{
	if(m_iCurVehicle > m_NoVehs-1) // m_iCurVehicle is zero-based
		return nullptr;

	CVehicle_sp pVeh = std::make_shared<CVehicle>(); //new CVehicle;
	*pVeh = *m_vVehicles.at(m_iCurVehicle);
	m_iCurVehicle++;
	return pVeh;
}

size_t CVehicleTrafficFile::getNoDays()
{
	return m_NoDays;
}

size_t CVehicleTrafficFile::getNoLanes()
{
	return m_NoLanes;
}

size_t CVehicleTrafficFile::getNoDirn()
{
	return m_NoDirn;
}

size_t CVehicleTrafficFile::getNoLanesDir1()
{
	return m_NoLanesDir1;
}

size_t CVehicleTrafficFile::getNoLanesDir2()
{
	return m_NoLanesDir2;
}

size_t CVehicleTrafficFile::getNoVehicles()
{
	return m_NoVehs;
}

double CVehicleTrafficFile::getStartTime()
{
	// make the start time the start of the first day of the sim
	return (double)((int)(m_Starttime/(3600.0*24))*3600.0*24);
}

double CVehicleTrafficFile::getEndTime()
{
	return m_Endtime;
}

