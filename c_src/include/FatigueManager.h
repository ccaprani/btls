#pragma once

#include "OutputManagerBase.h"
#include "Rainflow.h"


class CFatigueManager : public COutputManagerBase
{
public:
	CFatigueManager (void);
	virtual ~CFatigueManager (void);

	void Initialize (double bridgeLength, size_t noLoadEffects);
    void Update (CEvent Ev) {};
    void Update (std::vector< std::vector<double> >& vCurEvents);

private:
    void WriteSummaryFiles () {};
	virtual void CheckBuffer (bool bForceOutput);

    void doRainflow (std::vector< std::vector<double> >& signal_data);
    void countRainflow (std::vector< std::pair<double, double> >& rainflow_out, size_t i);

    std::ofstream m_RainflowOutFile;
    std::vector< std::map<double, double> > m_RainflowOutCount;
    CRainflow rainflow_alg;
};

