#include "BridgeLane.h"

using namespace std;

CBridgeLane::CBridgeLane()
	: m_LaneNo(0)
	, m_Length(0)
	, m_NoLE(0)
	, m_TimeNextVehOff(0.0)
	, m_Index(0)
{
	m_LaneWidth = 3.65;
}

CBridgeLane::CBridgeLane(double length)
	: m_LaneNo(0)
	, m_NoLE(0)
	, m_TimeNextVehOff(0.0)
	, m_Index(0)
{
	m_Length = length;
	m_LaneWidth = 3.65;
}


CBridgeLane::~CBridgeLane(void)
{
}


void CBridgeLane::Update(double curTime)
{
	m_CurTime = curTime;
	for(unsigned int j = 0; j < m_vAxles.size(); j++)
		m_vAxles[j].UpdatePosition(m_CurTime);	// update position for current time
}


void CBridgeLane::setLaneNo(size_t LaneNo)
{
	m_LaneNo = LaneNo;
}


size_t CBridgeLane::getLaneNo(void)
{
	return m_LaneNo;
}


void CBridgeLane::addLoadEffect(CInfluenceLine IL, double weight)
{
	IL.setWeight(weight);
	// Note this assumes that all bridge ISs have the same lane coordinates
	if(IL.getType() == 3)	// Inf Line Type for Inf Surface is 3
		m_LaneWidth = IL.getIS()->getLaneWidth(m_LaneNo);
	
	m_vInfLine.push_back(IL);
	m_NoLE = static_cast<unsigned int>( m_vInfLine.size() );
}


void CBridgeLane::AddVehicle(CVehicle* pVeh)
{
	m_vVehicles.push_back(pVeh);
	setAxleVector();	// set up axle vector
	setTimeNextVehOff();
}


void CBridgeLane::setAxleVector()
{
	m_vAxles.clear();
	size_t iAxle = 0;
	for(unsigned int i = 0; i < m_vVehicles.size(); i++)
	{
		CVehicle& Veh = *m_vVehicles[i];

		double speed = Veh.getVelocity();
		double timeAtRHSDatum = 0.0;
		double timeVehicleOn = Veh.getTime();
		double axleSpacing = 0.0;
		
		for(size_t j = 0; j < Veh.getNoAxles(); j++)
		{
			iAxle++;
			axleSpacing += j == 0 ? 0.0 : double( Veh.getAS(j-1) ); // ASs are cumulative
			timeAtRHSDatum = timeVehicleOn + axleSpacing/speed;

			if(Veh.getDirection() == 2)
				timeAtRHSDatum += m_Length/speed;

			//CAxle curAxle(iAxle,timeAtRHSDatum,speed,0.0,axleWeight,Veh.getDirection());
			CAxle curAxle(iAxle,j,timeAtRHSDatum,0.0,&Veh);
			// Note that generated vehicles have eccentricity but no trans while
			// read-in vehicles have tran but no eccentricity
			curAxle.m_Eccentricity += curAxle.m_TransPos-m_LaneWidth/2.0;
			m_vAxles.push_back(curAxle);
		}

	}
}

/*double CBridgeLane::doAxleLoop(int nLE)
{
	double effVal = 0.0;
	for(unsigned int j = 0; j < m_vAxles.size(); j++)
		effVal += m_vAxles[j].m_AxleWeight * m_vInfLine[nLE].getOrdinate(m_vAxles[j].m_Position);

	return effVal;
}*/

double CBridgeLane::getLength(void)
{
	return m_Length;
}


void CBridgeLane::setLength(double length)
{
	m_Length = length;
}


double CBridgeLane::getTimeNextVehOff(void) const
{
	return m_TimeNextVehOff;
}


double CBridgeLane::setTimeNextVehOff(void)
{
	double TimeOff = 1e300;	// MAGIC NUMBER - very big time
	if(m_vVehicles.size() > 0)
		TimeOff = m_vVehicles[0]->getTimeOffBridge();
	for(unsigned int i = 1; i < m_vVehicles.size(); i++)
	{
		double tOff = m_vVehicles[i]->getTimeOffBridge();
		if(TimeOff > tOff)
			TimeOff = tOff;
	}
	m_TimeNextVehOff = TimeOff;
	return m_TimeNextVehOff;
}


double CBridgeLane::getLoadEffect(size_t nLE)
{
	if(m_vVehicles.size() != 0)
		return m_vInfLine[nLE].getLoadEffect(m_vAxles); //doAxleLoop(nLE);
	else
		return 0.0;
}


double CBridgeLane::getLeadVehPosition(void)
{
	return m_vAxles.at(0).m_Position;
}

bool CBridgeLane::operator<(const CBridgeLane& other)
{
	return m_TimeNextVehOff < other.getTimeNextVehOff();
}


size_t CBridgeLane::getNoVehs(void)
{
	return m_vVehicles.size();
}


size_t CBridgeLane::getIndex(void)
{
	return m_Index;
}


void CBridgeLane::setIndex(size_t indx)
{
	m_Index = indx;
}


size_t CBridgeLane::purgeVehicles(double curTime)
{
	bool reset = false;
	for(unsigned int i = 0; i < m_vVehicles.size(); i++)
	{
		if( !m_vVehicles.at(i)->IsOnBridge(curTime) )
		{
			// delete object and remove pointer from vector
			delete m_vVehicles.at(i);					
			m_vVehicles.erase( m_vVehicles.begin() + i );
			--i;	// take one step back due to deletion
			reset = true;
		}
	}
	if(reset)
	{
		setTimeNextVehOff();
		setAxleVector();
	}
	return m_vVehicles.size();
}


std::vector<CVehicle*> CBridgeLane::getVehicles(void)
{
	return m_vVehicles;
}
