// Bridge.cpp: implementation of the CBridge class.
//
//////////////////////////////////////////////////////////////////////

#include "Bridge.h"
#include "ConfigData.h"

extern CConfigData g_ConfigData;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBridge::CBridge()
	: m_Index(0)
	, m_NoLanes(0)
	, m_NoLoadEffects(0)
	, m_NoVehs(0)
{
	NO_LANES_DIR1		= g_ConfigData.Road.NO_LANES_DIR1;
	NO_DIRS				= g_ConfigData.Road.NO_DIRS;
	NO_LANES			= g_ConfigData.Road.NO_LANES;
}

CBridge::CBridge(double length, double calcTimeStep, int n, double curTime)
{
	setLength(length);
	setCalcTimeStep(calcTimeStep);
	setCurrentSimTime(curTime);
}

CBridge::~CBridge()
{

}

void CBridge::InitializeDataMgr()
{
	// must be called after length and nLE set
	m_EventMgr.Initialize(m_Length,m_vThresholds);
}

void CBridge::setLength(double length)
{
	m_Length = length;
}

void CBridge::setCalcTimeStep(double calcTimeStep)
{
	m_CalcTimeStep = calcTimeStep;
}

void CBridge::setCurrentSimTime(double curTime)
{
	m_CurTime = curTime;
}

void CBridge::setThresholds(std::vector<double> vThresholds)
{
	m_vThresholds = vThresholds;
}

void CBridge::InitializeLanes(int NoLanes)
{
	m_NoLanes = NoLanes;
	if(m_Length <= 0.0)
		cout << "***ERROR: Bridge " << m_Index << "length of zero" << endl;
	CBridgeLane lane(m_Length);
	m_vLanes.assign(m_NoLanes,lane);
	// set the lane numbers
	for(int i = 0; i < m_NoLanes; i++)
		m_vLanes.at(i).setIndex(i);	
}

//void CBridge::UpdateMT(double NextArrivalTime, double curTime)
//{
//    m_Thread = boost::thread(&CBridge::Update, boost::ref(*this), NextArrivalTime, curTime);
//}
//
//void CBridge::join()
//{
//    m_Thread.join();
//}

void CBridge::Update(double NextArrivalTime, double curTime)
{
	setCurrentSimTime(curTime);

	while(m_CurTime < NextArrivalTime && m_NoVehs > 0)
	{	
		//if(m_CurTime > 29378.40)
		//	cout << "Hi";
		m_EventMgr.AddNewEvent(AssembleVehicles(), m_CurTime);

		sort(m_vLanes.begin(), m_vLanes.end());
		int lead_lane = 0;	// the lane of the leading vehicle
		while(m_vLanes[lead_lane].getNoVehs() == 0) lead_lane++; // ignore lanes with no vehs
		double NextTimeOff = TimeNextVehOffBridge(); //m_vLanes[lead_lane].getTimeNextVehOff();
		double EventEndTime = __min(NextArrivalTime,NextTimeOff);

		if(EventEndTime < m_CurTime)
			std::cout <<"***Error: Repeating event from the past" << endl;
		
		// while loop until a vehicle leaves or comes on
		while(m_CurTime < EventEndTime)
		{
			//if(m_CurTime > 29378.40)
			//	cout << "Hi";
			// Update vehicles
			for(int i = 0; i < m_NoLanes; i++)
				m_vLanes[i].Update(m_CurTime);
			// calculate the load effects
			m_vEffectValues.assign(m_NoLoadEffects,0.0);
			for(int i = 0; i < m_NoLoadEffects; i++)
				for(int j = 0; j < m_NoLanes; j++)
					m_vEffectValues[i] += m_vLanes[j].getLoadEffect(i);
			// update the event
			m_EventMgr.UpdateEffects(m_vEffectValues, m_vLanes[lead_lane].getLeadVehPosition(), m_CurTime);
			m_CurTime += m_CalcTimeStep;
		}

		m_EventMgr.EndEvent();

		// remove any vehicles that have left the bridge
		m_NoVehs = 0;
		for(int i = 0; i < m_NoLanes; i++)
			// IDEA: add a m_CalcTimeStep before purging to make sure they don't leave
			// within next timestep
			m_NoVehs += m_vLanes[i].purgeVehicles(m_CurTime);	// remove pointer from lane

	} // while loop until next vehicle arrives
	
}

void CBridge::Finish()
{
	m_EventMgr.Finish();
}

double CBridge::TimeNextVehOffBridge()
{
	double TimeOff = 1e300; // MAGIC NUMBER - very big time
	for(int i = 0; i < m_NoLanes; i++)
	{
		double temp = m_vLanes.at(i).setTimeNextVehOff();
		temp < TimeOff ? TimeOff = temp : 0.0;
	}
	return TimeOff;
}

bool CBridge::lane_compare(const CBridgeLane* pL1, const CBridgeLane* pL2)
{
	return pL1->getTimeNextVehOff() < pL2->getTimeNextVehOff();
}

void CBridge::AddVehicle(CVehicle* pVeh)
{	
	m_NoVehs++;
	
	// dereference ptr and get a copy, create new Vehicle on heap
	// copy required as vehicle may be on multiple bridges
	CVehicle* pLocalVeh = new CVehicle;
	*pLocalVeh = *pVeh;
	CVehicle& Veh = *pLocalVeh;
	Veh.setBridgeTimes(m_Length);

	// Map global direction lane to zero-based bridge lane - CASTOR type format
	int iLane = Veh.getLane() - 1; // zero based array
	
	// if the lane is local-numbered, something like this is needed:
	// if(NO_DIRS == 2 && Veh.getDirection() == 2)
	//		iLane = NO_LANES - iLane + 1;

	Veh.setBridgeLaneNo(iLane);	// bridge lanes in global lane numbering

	// update pointer to local store and save
	if(iLane > m_NoLanes)
		cout << "***ERROR: Veh - lane: " << Veh.getLane()
			 << " direction: " << Veh.getDirection()
			 << " cannot be added as no. lanes is " << m_vLanes.size() << endl;
	// Lanes are out of order due to sorting above
	for(int i = 0; i < m_NoLanes; i++)
	{
		if( m_vLanes.at(i).getIndex() ==  Veh.getBridgeLaneNo() )
			m_vLanes.at(i).AddVehicle(&Veh);
	}
}

std::vector<CVehicle*> CBridge::AssembleVehicles(void)
{
	std::vector<CVehicle*> pVehs;
	for(int i = 0; i < m_NoLanes; i++)
	{
		std::vector<CVehicle*> pLaneVehs = m_vLanes[i].getVehicles();
		pVehs.insert(pVehs.end(), pLaneVehs.begin(), pLaneVehs.end());
	}
	sort(pVehs.begin(),pVehs.end());
	return pVehs;
}


int CBridge::getIndex(void)
{
	return m_Index;
}


void CBridge::setIndex(int index)
{
	m_Index = index;
}


double CBridge::getLength(void)
{
	return m_Length;
}


CBridgeLane& CBridge::getBridgeLane(int iLane)
{
	CBridgeLane& lane = m_vLanes.at(iLane);
	return lane;
}

size_t CBridge::getNoLanes()
{
	return m_NoLanes;
}

void CBridge::setNoLoadEffects(int nLE)
{
	m_NoLoadEffects = nLE;
}


int CBridge::getNoLoadEffects(void)
{
	return m_NoLoadEffects;
}