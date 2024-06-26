#include "LaneFlow.h"


CLaneFlow::CLaneFlow(void)
	: m_LaneNo(0)
	, m_Dirn(0)
{
	Initialize();
}

CLaneFlow::CLaneFlow(int lane, int dirn)
{
	m_LaneNo = lane;
	m_Dirn = dirn;
	Initialize();
}


void CLaneFlow::Initialize(void)
{
	Speed tSpeed;
		tSpeed.Mean = 0.0;
		tSpeed.StdDev = 0.0;
	CP tCP;
		tCP.CP_cars = 0.0;
		tCP.CP_2Axle = 0.0;
		tCP.CP_3Axle = 0.0;
		tCP.CP_4Axle = 0.0;
		tCP.CP_5Axle = 0.0;
	HourData tHD;
		tHD.flow = 0.0;
		tHD.hourCP = tCP;
		tHD.hourSpeed = tSpeed;
	m_vHourData.assign(24,tHD);	// 24 hours per day
}


CLaneFlow::~CLaneFlow(void)
{
}

void CLaneFlow::setLaneNo(int lane)
{
	m_LaneNo = lane;
}


void CLaneFlow::setDirn(int dirn)
{
	m_Dirn = dirn;
}


void CLaneFlow::setHourData(std::vector<double> vHrData)
{
	if(vHrData.size() != 9)
		std::cout << "***ERROR: Incorrect flow data for hour " << vHrData.at(0) << " lane " << m_LaneNo << std::endl;
	else
	{
		int iHour								= static_cast<int>( vHrData.at(0) );
		m_vHourData.at(iHour).flow				= vHrData.at(1);
		m_vHourData.at(iHour).hourSpeed.Mean	= vHrData.at(2)/10.0;	// dm/s to m/s
		m_vHourData.at(iHour).hourSpeed.StdDev	= vHrData.at(3)/10.0;	// dm/s to m/s
		m_vHourData.at(iHour).hourCP.CP_cars	= vHrData.at(4)/100.0;	// to change e.g. 80% to 0.80

		if( m_vHourData.at(iHour).hourCP.CP_cars > 1.0 || m_vHourData.at(iHour).hourCP.CP_cars < 0)
			std::cout << "***ERROR: Incorrect car percentage for hour " << vHrData.at(0) << " lane " << m_LaneNo << std::endl;

		m_vHourData.at(iHour).hourCP.CP_2Axle	= vHrData.at(5)/100.0;
		m_vHourData.at(iHour).hourCP.CP_3Axle	= vHrData.at(6)/100.0;
		m_vHourData.at(iHour).hourCP.CP_4Axle	= vHrData.at(7)/100.0;
		m_vHourData.at(iHour).hourCP.CP_5Axle	= vHrData.at(8)/100.0;
		
		double sum = 0.0; 
		for(int i = 5; i <= 8; ++i) sum += vHrData.at(i);
		if(sum < 99.99 || sum > 100.01)
			std::cout << "***ERROR: Truck percentages " << sum << " != 100 for hour " 
				<< vHrData.at(0) << " lane " << m_LaneNo << std::endl;
	}
}

int CLaneFlow::getLaneNo(void) const
{
	return m_LaneNo;
}


int CLaneFlow::getDirn(void) const
{
	return m_Dirn;
}

double CLaneFlow::getFlow(int iHour)
{
	return m_vHourData.at(iHour).flow;
}


double CLaneFlow::getSpeedMean(int iHour)
{
	return m_vHourData.at(iHour).hourSpeed.Mean;
}


double CLaneFlow::getSpeedStDev(int iHour)
{
	return m_vHourData.at(iHour).hourSpeed.StdDev;
}


double CLaneFlow::getCP_cars(int iHour)
{
	return m_vHourData.at(iHour).hourCP.CP_cars;
}


double CLaneFlow::getCP_2Axle(int iHour)
{
	return m_vHourData.at(iHour).hourCP.CP_2Axle;
}


double CLaneFlow::getCP_3Axle(int iHour)
{
	return m_vHourData.at(iHour).hourCP.CP_3Axle;
}


double CLaneFlow::getCP_4Axle(int iHour)
{
	return m_vHourData.at(iHour).hourCP.CP_4Axle;
}


double CLaneFlow::getCP_5Axle(int iHour)
{
	return m_vHourData.at(iHour).hourCP.CP_5Axle;
}

std::vector<double> CLaneFlow::getCP(int iHour)
{
	std::vector<double> vCP;
	vCP.push_back( getCP_2Axle(iHour) );
	vCP.push_back( getCP_3Axle(iHour) );
	vCP.push_back( getCP_4Axle(iHour) );
	vCP.push_back( getCP_5Axle(iHour) );
	return vCP;
}