#include "LaneGenTraffic.h"

#include "ConfigData.h"

extern CConfigData g_ConfigData;

CLaneGenTraffic::CLaneGenTraffic(void)
{
	m_BufferGap = 1.0;	// m	MAGIC NUMBER - internal gap buffer (e.g. tyre diameter)
	m_pPrevVeh = NULL;
	m_CurHour = 0;

	CONGESTED_GAP			= g_ConfigData.Traffic.CONGESTED_GAP;
	CONGESTED_GAP_COEF_VAR	= g_ConfigData.Traffic.CONGESTED_GAP_COEF_VAR;
	CONGESTED_SPEED			= g_ConfigData.Traffic.CONGESTED_SPEED;
	HEADWAY_MODEL			= g_ConfigData.Traffic.HEADWAY_MODEL;

	NO_OVERLAP_LENGTH		= g_ConfigData.Gen.NO_OVERLAP_LENGTH;
}


CLaneGenTraffic::~CLaneGenTraffic(void)
{
}


void CLaneGenTraffic::SetParams(CVehicleGenerator* pGen, std::vector<double> vQ, double CarPerc, int iLane, int iDir)
{
	m_pGenerator = pGen;
	//m_vFlowRate = vQ;
	m_CarPerc = CarPerc;
	m_LaneIndex = iLane;
	m_Direction = iDir;

	m_vNHM = m_pGenerator->GetNHM();
	
	SetGapGeneration();	
	GenNextArrival();	// the first arrival generation
}

void CLaneGenTraffic::setLaneData(CVehicleGenerator* pGen, CLaneFlow lane_flow, double starttime)
{
	m_pGenerator = pGen;
	m_LaneFlow = lane_flow;
	m_NextArrivalTime = starttime;

	m_LaneIndex = m_LaneFlow.getLaneNo();
	m_Direction = m_LaneFlow.getDirn();
	m_vNHM = m_pGenerator->GetNHM();

	UpdateFlowProperties();
	SetGapGeneration();	
	GenNextArrival();	// the first arrival generation
}

CVehicle* CLaneGenTraffic::GetNextVehicle()
{
	CVehicle* pVeh = m_pNextVeh;	// temp store
	GenNextArrival();				// prepare next vehicle	
	return pVeh;					// send out last vehicle
}

void CLaneGenTraffic::GenNextArrival()
{
	GenNextVehicle();
	GenNextTime();
	m_pNextVeh->setTime(m_NextArrivalTime); // update with new arrival time	
}

void CLaneGenTraffic::GenNextVehicle()
{
	m_pPrevVeh = m_pNextVeh;	// assign as previous vehicle
	// m_pNextVeh = m_pGenerator->Generate(m_LaneIndex, m_Direction);
	m_pNextVeh = m_pGenerator->Generate(m_CurHour);
}

void CLaneGenTraffic::GenNextTime()
{
	UpdateHour();	// check if we need to update flow info
	
	double gap = 0.0;
	double minGap =  GetMinGap();	
	
	while(gap < minGap)
	{
		if(HEADWAY_MODEL == 0)
			gap = GenGap_NHM();
		else if(HEADWAY_MODEL == 5)
			gap = GenGap_Cong();
		else
			gap = GenGap_Poisson();
//		if(gap < minGap)
//			std::cout << "Overlap prevented: " << gap << " s < " << minGap << " s" << endl;
	}

	m_NextArrivalTime += gap;	
}

double CLaneGenTraffic::GetMinGap()
{
	double minGap = 0.1; // 0.1 s min gap for first vehicle
	
	if(m_pPrevVeh == NULL)
		return minGap; 

	double Vii = m_pPrevVeh->getVelocity();
	double Vi = m_pNextVeh->getVelocity();
	double Lii = m_pPrevVeh->getLength();	// dm to m
	
	if(Vi <= Vii)
		minGap = (Lii + m_BufferGap)/Vii;	// so no overlap of vehicles
	else
	{
		double deltaV = Vi - Vii;
		double Tno = (NO_OVERLAP_LENGTH + m_BufferGap + Lii)/Vii;	// time of no overlap allowed
		double DistClose = deltaV * Tno;
		double DistStart = DistClose + Lii + m_BufferGap;			// no overlap distance gap

		minGap = DistStart/Vi;	// no overlap time gap
	}
		
	return minGap;
}

void CLaneGenTraffic::SetGapGeneration()
{
	double mean = 0.0;
	double truckFlow = m_vFlowRate; //[m_CurHour];

	// only if cars are to be included should we amplify the flows
	// note the NHM for > 4 s uses Poisson distribution
	mean = 3600.0/truckFlow;
	if(HEADWAY_MODEL == 6)
	{
		double truckPercent = 1.0 - m_CarPerc;
		double totalFlow = truckFlow/truckPercent;
		mean = 3600.0/totalFlow;
	}

	m_RNG.setScale(mean);
	m_RNG.setLocation(0.0);	// for exponential deviates
}

void CLaneGenTraffic::UpdateHour()
{
	int nHour = (int)(m_NextArrivalTime/3600);
	nHour = nHour % 24;	// reduce to one day

	if(nHour-m_CurHour != 0) // if it's not the same hour
	{
		m_CurHour++;
		m_CurHour = m_CurHour % 24;	// make sure under 24
		
		UpdateFlowProperties();
		SetGapGeneration();
	}
}

void CLaneGenTraffic::UpdateFlowProperties(void)
{
	m_vFlowRate = m_LaneFlow.getFlow(m_CurHour);
	m_CarPerc = m_LaneFlow.getCP_cars(m_CurHour);
}

double CLaneGenTraffic::GenGap_Poisson()
{
	double gap = m_RNG.GenerateExponential();
	return gap;
}

double CLaneGenTraffic::GenGap_Cong()
{		
	// randomize congestion: note this is time gap from front to front of truck
	double timeForTruckToPass = 0.0;

	if(m_pPrevVeh != NULL)
	{
		double Vii = m_pPrevVeh->getVelocity();
		double Lii = m_pPrevVeh->getLength();
		timeForTruckToPass = Lii/Vii; // ignores the m_BufferGap on purpose
	}
	else	// there is no truck in front so set to own time to pass
		timeForTruckToPass = m_pNextVeh->getLength()/m_pNextVeh->getVelocity();
	
	// generate congested time gap and add prev vehicle passage time
	double congGap = m_RNG.GenerateNormal(CONGESTED_GAP, CONGESTED_GAP*CONGESTED_GAP_COEF_VAR);
	double gap = timeForTruckToPass + congGap;
	return gap;
}

double CLaneGenTraffic::GenGap_NHM()
{
	double headwayType = m_RNG.GenerateUniform();
	double Q = m_vFlowRate; //[m_CurHour];
	double gap = 0.0;

	// vector - vNHM: ie New Headway Model - as per IStructE paper
	// first line gives no of intervals
	// second line gives the u1s params of the quadratic curve
	// third gives the u1.5s params
	// the rest give the flowrate interval and the flowrate and it's params
	
	int curInterval = 0;	int noIntervals = (int)m_vNHM[0][0];
	
	int i = 3;
	while(i < 3+noIntervals && Q > m_vNHM[i][0])
		i++;
	curInterval = i; // not i+1 because it's a zero-based array

	double u1s		= quad(m_vNHM[1][1],	m_vNHM[1][2],	m_vNHM[1][3],	1.0);
	double u1pt5s	= quad(m_vNHM[2][1],	m_vNHM[2][2],	m_vNHM[2][3],	1.5);
	double u4s		= quad(m_vNHM[curInterval][1],m_vNHM[curInterval][2],	m_vNHM[curInterval][3],	4.0);

	if(headwayType > u4s)
	{
		do{
			gap = GenGap_Poisson();
		}while(gap < 4.0);
	}
	else if(headwayType <= u1s)
	{
		gap = inv_quad(m_vNHM[1][1], m_vNHM[1][2], m_vNHM[1][3], headwayType);
	}
	else if(headwayType > u1s && headwayType <= u1pt5s)
	{
		gap = inv_quad(m_vNHM[2][1], m_vNHM[2][2], m_vNHM[2][3], headwayType);
	}
	else if(headwayType > u1pt5s && headwayType <= u4s)
	{
		gap = inv_quad(m_vNHM[curInterval][1], m_vNHM[curInterval][2], m_vNHM[curInterval][3], headwayType);
	}
	return gap;
}

double CLaneGenTraffic::quad(double a, double b, double c, const double x)
{
	return a*x*x + b*x + c;
}

double CLaneGenTraffic::inv_quad(double a, double b, double c, const double y)
{
	c = c-y;
	double x1 = (-b + sqrt(b*b-4*a*c) )/(2*a);
	double x2 = (-b - sqrt(b*b-4*a*c) )/(2*a);
	double x0 = -b/(2*a); // this is the derivative at the min point

	if(a < 0 && x1 > x0) // ie concave
		return x1;
	else
	{
		c = c+y;
		double y_trial = quad(a,b,c,x1);
		if(y_trial > 0.99*y && y_trial < 1.01*y)
			return x1;
		else
			return x2;
	}
}
