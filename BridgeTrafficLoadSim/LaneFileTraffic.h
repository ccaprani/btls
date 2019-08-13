#pragma once
#include "lane.h"
#include <vector>

class CLaneFileTraffic :
	public CLane
{
public:
	CLaneFileTraffic(void);
	~CLaneFileTraffic(void);

	virtual CVehicle* GetNextVehicle();

	void setLaneData(int dirn, int laneNo);
	void addVehicle(CVehicle* pVeh);
	void setFirstArrivalTime();

	unsigned int GetNoVehicles() {return m_vVehicles.size();};

private:
	std::vector<CVehicle*> m_vVehicles;
};

