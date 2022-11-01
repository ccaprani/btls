#include "LaneFlowComposition.h"


CLaneFlowComposition::CLaneFlowComposition(size_t lane, size_t dirn, size_t blockSize)
	: m_GlobalLaneNo(lane), m_Dirn(dirn), m_TruckClassCount(0), m_BlockSize(blockSize)
{
}


CLaneFlowComposition::~CLaneFlowComposition()
{

}

// Takes in a row from the lane flow file and parses
void CLaneFlowComposition::addBlockData(vec vData)
{
	m_TruckClassCount = vData.size() - 5;

	size_t iCol = 0; size_t iBlock = static_cast<size_t>(vData.at(iCol));
	iCol++;		double flow = vData.at(iCol);		m_vTruckFlow.push_back(flow);
	iCol++;		double mean = vData.at(iCol)/10.0;	// dm/s to m/s
	iCol++;		double std = vData.at(iCol)/10.0;	m_vSpeed.push_back(Normal(mean, std)); 
	iCol++;		double carP = vData.at(iCol)/100.0;	m_vCarP.push_back(carP);	// e.g. 80% to 0.80
		
	if (carP > 1.0 || carP < 0)
		cout << "***ERROR: Incorrect car percentage for hour/block " << iBlock << " lane " << m_GlobalLaneNo << endl;
	m_vTotalFlow.push_back(flow/(1.0-carP));

	vec vComposition;	double sum = 0.0;
	for (size_t j = 0; j < m_TruckClassCount; ++j)
	{
		iCol++;
		double perc = vData.at(iCol);
		sum += perc;
		vComposition.push_back(perc/100.0);
	}
	if (sum < 99.99 || sum > 100.01)
		cout << "***ERROR: Truck percentages " << sum << " != 100 for hour/block "
		<< iBlock << " lane " << m_GlobalLaneNo << endl;
	m_mComposition.push_back(vComposition);

}

void CLaneFlowComposition::completeData()
{
	m_NoBlocks = m_vCarP.size();
}
