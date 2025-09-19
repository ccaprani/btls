#pragma once

#include "OutputManagerBase.h"
#include "Rainflow.h"

class CFatigueManager : public COutputManagerBase
{
public:
	CFatigueManager(CConfigDataCore &config);
	virtual ~CFatigueManager(void);

	void Initialize(double bridgeLength, size_t noLoadEffects);
	void Update(CEvent Ev) {};
	void Update();
	void addLoadEffectValues(std::vector<double> vEffs);

private:
	void cleanLoadEffectValues();

	void WriteSummaryFiles() {};
	virtual void CheckBuffer(bool bForceOutput);

	void doRainflow(bool bIsFinal);
	void writeRainflowBuffer();
	void cleanRainflowOutCountValues();

	std::vector<CRainflow> m_vRainflow;
	std::ofstream m_RainflowOutFile;
	std::vector<std::vector<double>> m_vLoadEffectValues;

	size_t m_EventCount;

	bool m_WriteRainflowHeadLine;
	bool DO_FATIGUE_RAINFLOW;
	int RAINFLOW_DECIMAL;
	double RAINFLOW_CUTOFF;
};
