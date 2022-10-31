#pragma once

#include <iostream>
#include <vector>

class CLaneFlow
{
public:
	CLaneFlow(void);
	CLaneFlow(int lane, int dirn);
	~CLaneFlow(void);

	void setHourData(std::vector<double> vHrData);
	void setLaneNo(int lane);
	void setDirn(int dirn);

	int getLaneNo(void) const;
	int getDirn(void) const;

private:
	struct Speed
	{
		double Mean;
		double StdDev;
	};
	
	struct CP
	{
		double CP_cars;
		double CP_2Axle;
		double CP_3Axle;
		double CP_4Axle;
		double CP_5Axle;
	};

	struct HourData
	{
		double flow;
		Speed hourSpeed;
		CP hourCP;
	};
		
	std::vector<HourData> m_vHourData;
	int m_LaneNo;
	int m_Dirn;
	void Initialize(void);

public:
	double getFlow(int iHour);
	double getSpeedMean(int iHour);
	double getSpeedStDev(int iHour);
	double getCP_cars(int iHour);
	double getCP_2Axle(int iHour);
	double getCP_3Axle(int iHour);
	double getCP_4Axle(int iHour);
	double getCP_5Axle(int iHour);
	std::vector<double> getCP(int iHour);
};

