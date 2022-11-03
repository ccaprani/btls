#pragma once
#include "ModelData.h"

class CLaneFlowComposition
{
public:
	CLaneFlowComposition(size_t lane, size_t dirn, size_t blockSize);
	~CLaneFlowComposition();

	size_t getGlobalLaneNo() const { return m_GlobalLaneNo; };
	size_t getDirn() const { return m_Dirn; };

	matrix getComposition() const { return m_mComposition; };
	vec getCarPercent() const { return m_vCarP; };
	
	vec getTotalFlow() const { return m_vTotalFlow; };
	vec getTruckFlow() const { return m_vTruckFlow; };
	std::vector<Normal> getSpeed() const { return m_vSpeed; };

	size_t getBlockCount() const { return m_NoBlocks; };
	size_t getBlockSize() const { return m_BlockSize; };

	size_t getTruckClassCount() const { return m_TruckClassCount; };

	void addBlockData(vec vData);
	void completeData();

private:
	matrix m_mComposition;
	vec m_vCarP;
	vec m_vTotalFlow;
	vec m_vTruckFlow;
	std::vector<Normal> m_vSpeed;

	size_t m_GlobalLaneNo;	// Global
	size_t m_Dirn;			// 1-based
	size_t m_BlockSize;		// typically 3600 secs	
	size_t m_NoBlocks;		// typically hours
	size_t m_TruckClassCount;
};

