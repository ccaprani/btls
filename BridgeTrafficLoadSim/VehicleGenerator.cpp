// VehicleGenerator.cpp: implementation of the CVehicleGenerator class.
//
//////////////////////////////////////////////////////////////////////

#include "VehicleGenerator.h"
#include "ConfigData.h"

extern CConfigData g_ConfigData;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVehicleGenerator::CVehicleGenerator(CTrafficData TD, CLaneFlow LF)
{
	CONGESTED_SPEED			= g_ConfigData.Traffic.CONGESTED_SPEED;
	HEADWAY_MODEL			= g_ConfigData.Traffic.HEADWAY_MODEL;
	LANE_ECCENTRICITY_STD	= g_ConfigData.Gen.LANE_ECCENTRICITY_STD/100; // cm to m

	m_TD = TD;
	m_LaneFlow = LF;

	m_CurLane = m_LaneFlow.getLaneNo();
	m_CurDirection = m_LaneFlow.getDirn();
}

CVehicleGenerator::~CVehicleGenerator()
{

}

std::vector< std::vector<double> > CVehicleGenerator::GetNHM()
{
	return m_TD.GetNHM();
}
/*
CVehicle* CVehicleGenerator::Generate(int iLane, int dir)
{
	m_CurLane = iLane;
	m_CurDirection = dir;
//	m_Time = time;

	CVehicle* pVeh = new CVehicle;

	// assign general properties
	pVeh->setLane(m_CurLane+1);
	pVeh->setDirection(m_CurDirection);
//	pVeh->setTime(m_Time);
	pVeh->setTrns(1.80);			// m
	pVeh->setHead(1001);		
	double speed = GenerateSpeed();
	pVeh->setVelocity(speed);		// m/s

	// determine type of vehicle
	if( NextVehicleIsCar() )
		GenerateCar(pVeh);
	else
	{
		int nAxles = TruckType();
		if(nAxles > 3)
			GenerateTruck45(pVeh, nAxles);
		else
			GenerateTruck23(pVeh, nAxles);
	}

	return pVeh;
}
*/
CVehicle* CVehicleGenerator::Generate(int iHour)
{
	CVehicle* pVeh = new CVehicle;
	m_CurHour = iHour;
	
	// assign general properties
	pVeh->setLane(m_CurLane+1);
	pVeh->setDirection(m_CurDirection);
	pVeh->setLaneEccentricity(m_RNG.GenerateNormal(0.0,LANE_ECCENTRICITY_STD));
	pVeh->setTrns(0.0); // m 0 for generated vehicles
	pVeh->setHead(1001);		
	double speed = GenerateSpeed();
	pVeh->setVelocity(speed);		// m/s

	// determine type of vehicle
	if( NextVehicleIsCar() )
		GenerateCar(pVeh);
	else
	{
		int nAxles = TruckType();
		if(nAxles > 3)
			GenerateTruck45(pVeh, nAxles);
		else
			GenerateTruck23(pVeh, nAxles);
	}
	return pVeh;
}

double CVehicleGenerator::GenerateSpeed()
{
	double speed = 0.0;
	if(HEADWAY_MODEL == 5)
		speed = CONGESTED_SPEED; // m/s
	else
	{
		// speed = m_RNG.GenerateTriModalNormal( m_TD.GetSpeed(m_CurDirection) );
		speed = m_RNG.GenerateNormal( m_LaneFlow.getSpeedMean(m_CurHour), 
									  m_LaneFlow.getSpeedStDev(m_CurHour) );
	}
	return speed;
}

bool CVehicleGenerator::NextVehicleIsCar()
{
	// cars are only possible with HW of 5 (congestion) or 6 (FF w/ cars)
	if(HEADWAY_MODEL == 5 || HEADWAY_MODEL == 6)
	{
		double prop = m_RNG.GenerateUniform();
		// if( prop > m_TD.GetPercent_Cars() )
		if( prop > m_LaneFlow.getCP_cars(m_CurHour) )
			return false;	// is not a car
		
		return true;		// is a car
	}
	else
		return false;
}

int CVehicleGenerator::TruckType()
{
	double prop = m_RNG.GenerateUniform();
	// std::vector<double> vCP = m_TD.GetClassPercent(m_CurLane);
	std::vector<double> vCP = m_LaneFlow.getCP(m_CurHour);

	int nAxles = 1;
	double limit = 0.0;

	while(limit < prop)
	{
		nAxles++;
		limit += vCP.at(nAxles-2); // /100; *** NOTE: new LaneFlow format has already /100
	}

	return nAxles;
}

void CVehicleGenerator::GenerateCar(CVehicle *pVeh)
{
	pVeh->setNoAxles(2);
	pVeh->setGVW(20.0);
	pVeh->setAW(0,10.0);
	pVeh->setAW(1,10.0);
	pVeh->setLength(4.0);	// in m
	pVeh->setAS(0, 4.0);	// in m
	pVeh->setAS(1, 0.0);
}

void CVehicleGenerator::GenerateTruck23(CVehicle *pVeh, int nAxles)
{
	GenerateCommonProps(pVeh, nAxles);

	// Generate Axle Weights
	std::vector<double> vAW(nAxles, 0.0);
	for(int i = 0; i < nAxles; ++i)
	{
		double val = -1.0;
		while(val < 15 || val > 500)
			val = m_RNG.GenerateTriModalNormal( m_TD.GetAxleWeightDist(nAxles, i) );
		vAW[i] = val*0.981;	// kg/100 to kN
	}
	
	double sumAW = SumVector(vAW);
	double GVW = pVeh->getGVW();	// since it's already set
	ScaleVector(vAW, GVW/sumAW);	// scale AWs to get correct GVW
	
	// And assign to vehicle
	for(int i = 0; i < nAxles; ++i)
		pVeh->setAW(i, vAW[i]);

}

void CVehicleGenerator::GenerateTruck45(CVehicle *pVeh, int nAxles)
{
	GenerateCommonProps(pVeh, nAxles);

	// Generate Axle Weights
	double GVW = pVeh->getGVW();	// since it's already set
	int iRange = int( (GVW-25.0)/50.0 ) + 1;	// iRange - index of truck weight in 50 kN intervals
	std::vector<double> vAWdist = m_TD.GetGVWRange(nAxles,iRange);
	std::vector<double> vAW(5, 0.0);

	for(int i = 0; i < 3; ++i)
	{
		double val = -1.0;
		while(val < 15 || val > 500)
			val = m_RNG.GenerateNormal(vAWdist[i], vAWdist[i+3]);
		vAW[i] = val*0.981;	// kg/100 to kN
	}
	
	// scale AWs to get correct GVW
	double sumAW = SumVector(vAW);
	ScaleVector(vAW, GVW/sumAW);	
	
	// split the tandem/tridem weight
	double tdemW = vAW[2];
	double indivAW = nAxles == 4 ? tdemW/2 : tdemW/3;
	for(int i = 2; i < nAxles; ++i)
		vAW[i] = indivAW;

	// And assign to vehicle
	for(int i = 0; i < nAxles; ++i)
		pVeh->setAW(i, vAW[i]);
}

void CVehicleGenerator::GenerateCommonProps(CVehicle *pVeh, int nAxles)
{
	pVeh->setNoAxles(nAxles);

	// Generate GVW, AS, length properties
	double GVW = -1.0;
	while(GVW < 35 || GVW > 1000)
		GVW = m_RNG.GenerateTriModalNormal( m_TD.GetGVW(m_CurDirection, nAxles) );
	GVW = GVW*0.981; // kg/100 to kN

	// Gen axle spacings
	std::vector<double> vAS(nAxles, 0.0);
	for(int i = 0; i < nAxles-1; ++i)	// no last-axle spacing
	{
		double val = -1.0;
		while(val < 0.5 || val > 200)
			val = m_RNG.GenerateTriModalNormal( m_TD.GetSpacingDist(nAxles, i) );
		vAS[i] = val/10; // dm to m
	}
	double length = SumVector(vAS);

	// Gen axle track widths
	std::vector<double> vATW(nAxles, 0.0);
	// do first axle and see if to be constant for all other axles
	double val = -1.0;
	while(val < 120.0 || val > 260.0)	// physical bounds
		val = m_RNG.GenerateTriModalNormal( m_TD.GetTrackWidthDist(nAxles, 0) );
	vATW[0] = val/100; // cm to m
	for(int i = 1; i < nAxles; ++i)
	{
		CTriModalNormal tmn = m_TD.GetTrackWidthDist(nAxles, i);
		if(tmn.m_vModes[0].Mean > 1e-6)	// only generate if a new value is required
		{
			val = -1.0;
			while(val < 120.0 || val > 260.0)	// physical bounds
				val = m_RNG.GenerateTriModalNormal( m_TD.GetTrackWidthDist(nAxles, i) );
		}
		vATW[i] = val/100; // cm to m
	}

	// assign these new properties to vehicle
	pVeh->setGVW(GVW);
	for(int i = 0; i < nAxles; ++i)
	{
		pVeh->setAS(i, vAS[i]);
		pVeh->setAT(i, vATW[i]);
	}
	pVeh->setLength(length);
}

double CVehicleGenerator::SumVector(std::vector<double> vec)
{
	double sum = 0.0;
	for(unsigned int i = 0; i < vec.size(); ++i)
		sum += vec[i];

	return sum;
}

void CVehicleGenerator::ScaleVector(std::vector<double>& vec, double scale)
{
	for(unsigned int i = 0; i < vec.size(); ++i)
		vec[i] *= scale;
}
