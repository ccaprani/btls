#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "Vehicle.h"

class CVehicleTrafficFile
{
public:
	CVehicleTrafficFile(bool UseConstSpeed, bool UseAveSpeed, double ConstSpeed);
	~CVehicleTrafficFile(void);

	void Read(std::string file, int filetype);

	unsigned int getNoDays();
	unsigned int getNoLanes();
	unsigned int getNoDirn();
	unsigned int getNoLanesDir1();
	unsigned int getNoLanesDir2();
	size_t getNoVehicles();
	
	CVehicle* getNextVehicle();
	double getStartTime();
	double getEndTime();

private:
	void UpdateProperties();
	void SetSpeed();

	bool m_UseConstSpeed;
	bool m_UseAveSpeed;
	double m_ConstSpeed;

	size_t m_NoVehs;
	unsigned int m_NoDays;
	unsigned int m_NoLanes;
	unsigned int m_NoDirn;
	unsigned int m_NoLanesDir1;
	unsigned int m_NoLanesDir2;

	double m_Starttime;
	double m_Endtime;

	unsigned int m_iCurVehicle;
	std::vector<CVehicle*> m_vVehicles;
};

