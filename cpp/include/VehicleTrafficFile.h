#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include "Vehicle.h"

class CVehicleTrafficFile
{
public:
	CVehicleTrafficFile(CVehicleClassification_sp pVC, bool UseConstSpeed, bool UseAveSpeed, double ConstSpeed);
	~CVehicleTrafficFile(void);

	void Read(std::filesystem::path file, int filetype);
	void AssignTraffic(std::vector<CVehicle_sp> vVehicles);

	size_t getNoDays();
	size_t getNoLanes();
	size_t getNoDirn();
	size_t getNoLanesDir1();
	size_t getNoLanesDir2();
	size_t getNoVehicles();
	
	CVehicle_sp getNextVehicle();
	std::vector<CVehicle_sp> getVehicles() const { return m_vVehicles; };
	double getStartTime();
	double getEndTime();

private:
	void AnalyseTraffic();
	void UpdateProperties();
	void SetSpeed();

	bool m_UseConstSpeed;
	bool m_UseAveSpeed;
	double m_ConstSpeed;

	CVehicleClassification_sp m_pVehClassification;

	size_t m_NoVehs;
	size_t m_NoDays;
	size_t m_NoLanes;
	size_t m_NoDirn;
	size_t m_NoLanesDir1;
	size_t m_NoLanesDir2;

	double m_Starttime;
	double m_Endtime;

	unsigned int m_iCurVehicle;
	std::vector<CVehicle_sp> m_vVehicles;
};

