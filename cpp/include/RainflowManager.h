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
	void doRainflow(std::vector< std::vector<double> >& signalData);
	void countRainflow(std::vector< std::pair<double, double> >& rainflowOut, size_t i);

	CRainflow m_RainflowAlg;
	std::ofstream m_RainflowOutFile;
	std::vector< std::vector<double> >  m_LoadEffectValues;
	std::vector< std::map<double, double> > m_RainflowOutCount;
	bool m_WriteHeadLine;
	size_t m_EventCount;
	
	int RAINFLOW_DECIMAL;
	double RAINFLOW_CUTOFF;
};

