#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <math.h>
#include "Event.h"
#include "Effect.h"
#include "Vehicle.h"

class CEventStatistics
{
public:
	CEventStatistics(void);
	virtual ~CEventStatistics(void);

	void update(CEvent Ev, unsigned int iLE);
	std::string outputString();
	std::string headingsString();
	
	size_t m_N, m_ID;
	// load effect relared statistics
	double m_Max, m_Min;
	double m_Mean, m_Variance, m_StdDev;
	double m_Skewness, m_Kurtosis;
	double m_M2, m_M3, m_M4;
	// Event composition
	size_t m_NoVehicles, m_NoTrucks;	

	std::vector<size_t> m_vNoTrucksInEvent;

private:
	void accumulator(double x);
	void finalize();

	size_t m_MaxNoTrucksInEvent;
};

