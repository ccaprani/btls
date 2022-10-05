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
    void Update ();
    void addLoadEffectValues (std::vector<double> vEffs);

private:
    void WriteSummaryFiles () {};
    virtual void CheckBuffer (bool bForceOutput);
    void WriteFiles ();

    void cleanLoadEffectValues ();
    void doRainflow (std::vector< std::vector<double> >& signalData);
    void countRainflow (std::vector< std::pair<double, double> >& rainflowOut, size_t i);

    std::ofstream m_RainflowOutFile;
    std::vector< std::vector<double> >  m_LoadEffectValues;
    std::vector< std::map<double, double> > m_RainflowOutCount;
    CRainflow m_RainflowAlg;
    int noDecimal;
    double cutOffValue;
};
