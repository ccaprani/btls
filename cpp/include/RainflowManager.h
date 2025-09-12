#pragma once

#include "OutputManagerBase.h"
#include "Rainflow.h"


class CRainflowManager : public COutputManagerBase
{
public:
	CRainflowManager(CConfigDataCore& config);
	virtual ~CRainflowManager(void);

	void Initialize(double bridgeLength, size_t noLoadEffects);
	void Update(CEvent Ev) {};
	void Update();
	void addLoadEffectValues (std::vector<double> vEffs);

private:
	void WriteSummaryFiles() {};
	virtual void CheckBuffer(bool bForceOutput);
	virtual void WriteBuffer();

	void cleanLoadEffectValues();
	void cleanRainflowOutCountValues();
	void extractReversals();
	void doRainflow(bool bIsFinal);
	void countRainflow(std::vector< std::pair<double, double> >& rainflowOut, size_t i);

	CRainflow m_RainflowAlg;
	std::ofstream m_RainflowOutFile;
	std::vector< std::vector<double> > m_vLoadEffectValues;
	std::vector< std::vector<double> > m_vReversals;
	std::vector< std::map<double, double> > m_vRainflowOutCounts;
	bool m_WriteHeadLine;
	size_t m_EventCount;

	bool m_bFirstEvent;
	
	int RAINFLOW_DECIMAL;
	double RAINFLOW_CUTOFF;
};

