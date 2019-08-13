
#include "EventStatistics.h"

CEventStatistics::CEventStatistics()	// default constructor
{	
	m_N = 0;
	m_ID = 0;
	
	m_Max = 0.0;
	m_Min = 0.0;
	m_Mean = 0.0; 
	m_Variance = 0.0; 
	m_StdDev = 0.0; 
	m_Skewness = 0.0; 
	m_Kurtosis = 0.0;
	m_M2 = 0.0;
	m_M3 = 0.0;
	m_M4 = 0.0;

	m_NoVehicles = 0;
	m_NoTrucks = 0;

	m_MaxNoTrucksInEvent = 0;
};


CEventStatistics::~CEventStatistics(void)
{
}

void CEventStatistics::update(CEvent Ev, unsigned int iLE)
{
	double x = Ev.getMaxEffect(iLE).getValue();
	accumulator(x);
	m_NoVehicles += Ev.getNoVehicles();
	m_NoTrucks += Ev.getNoTrucks();

	if (m_NoTrucks > m_MaxNoTrucksInEvent)
	{
		while (m_MaxNoTrucksInEvent < m_NoTrucks)
		{
			m_vNoTrucksInEvent.push_back(0);
			m_MaxNoTrucksInEvent++;
		}
	}
	m_vNoTrucksInEvent.at(m_NoTrucks - 1) += 1;
}

void CEventStatistics::accumulator(double x)
{
	// See http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Online_algorithm

	if(m_N == 0 || x > m_Max) m_Max = x;
	if(m_N == 0 || x < m_Min) m_Min = x;

	double n1 = (double)m_N;
	double n = n1+1;
	double delta = x - m_Mean;
	double delta_n = delta/n;
	double delta_n2 = delta_n*delta_n;
	double term1 = delta*delta_n*n1;
	m_Mean += delta_n;
	m_M4 += term1*delta_n2*(n*n - 3*n + 3) + 6*delta_n2*m_M2 - 4*delta_n*m_M3;
	m_M3 += term1*delta_n*(n - 2) - 3*delta_n*m_M2;
	m_M2 += term1;
	m_N = (size_t)n;
};

void CEventStatistics::finalize()	// finalize the statistics from the accumulators
{
	m_Variance = m_M2/(m_N-1);
	m_StdDev = sqrt(m_Variance);
	m_Skewness = (sqrt((double)m_N)*m_M3)/sqrt(m_M2*m_M2*m_M2);
	m_Kurtosis = (m_N*m_M4)/(m_M2*m_M2) - 3;
};

std::string CEventStatistics::outputString()
{
	finalize();	// update the values from the accumulators prior to output
		
	std::ostringstream oStr;
	oStr.width(10);		oStr << m_N;
	oStr.width(10);		oStr << m_NoVehicles;
	oStr.width(10);		oStr << m_NoTrucks;
	oStr.width(10);		oStr << std::fixed << std::setprecision(2) << m_Min;
	oStr.width(10);		oStr << std::fixed << std::setprecision(2) << m_Max;
	oStr.width(10);		oStr << std::fixed << std::setprecision(2) << m_Mean;
	oStr.width(10);		oStr << std::fixed << std::setprecision(2) << m_StdDev;
	oStr.width(15);		oStr << std::fixed << std::setprecision(2) << m_Variance;
	oStr.width(10);		oStr << std::fixed << std::setprecision(2) << m_Skewness;
	oStr.width(10);		oStr << std::fixed << std::setprecision(2) << m_Kurtosis;
	
	for (size_t i = 0; i < m_vNoTrucksInEvent.size(); ++i)
	{
		oStr.width(10);
		oStr << std::fixed << std::setprecision(2) << m_vNoTrucksInEvent.at(i);
	}

	oStr << std::ends;
	return oStr.str();
};

std::string CEventStatistics::headingsString()
{
	std::ostringstream oStr;
	oStr.width(10);		oStr << "No.Events";
	oStr.width(10);		oStr << "No.Vehs";
	oStr.width(10);		oStr << "No.Trucks";
	oStr.width(10);		oStr << "Min";
	oStr.width(10);		oStr << "Max";
	oStr.width(10);		oStr << "Mean";
	oStr.width(10);		oStr << "StdDev";
	oStr.width(15);		oStr << "Variance";
	oStr.width(10);		oStr << "Skewness";
	oStr.width(10);		oStr << "Kurtosis";
	oStr.width(10);		oStr << "Truck Presence Counts";
	oStr << std::ends;
	return oStr.str();
};