#pragma once
#include "lane.h"
#include <vector>

class CLaneFileTraffic : public CLane
{
public:
	CLaneFileTraffic(void);
	~CLaneFileTraffic(void);

	virtual CVehicle_sp GetNextVehicle();

	void setLaneData(int dirn, int laneNo);
	void addVehicle(CVehicle_sp pVeh);
	void setFirstArrivalTime();

	size_t GetNoVehicles() { return m_vVehicles.size(); };

private:
	std::vector<CVehicle_sp> m_vVehicles;
};
typedef std::shared_ptr<CLaneFileTraffic> CLaneFileTraffic_sp;
