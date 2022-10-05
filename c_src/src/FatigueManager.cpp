#include "FatigueManager.h"


CFatigueManager::CFatigueManager(void)
{
}

CFatigueManager::~CFatigueManager(void)
{
}

void CFatigueManager::Initialize(double bridgeLength, size_t noLoadEffects)
{
    m_BridgeLength = bridgeLength;
    m_NoLoadEffects = noLoadEffects;
    m_LoadEffectValues = std::vector< std::vector<double> >(m_NoLoadEffects);
    m_RainflowOutCount = std::vector< std::map<double, double> >(m_NoLoadEffects);
}

void CFatigueManager::addLoadEffectValues (std::vector<double> vEffs) {
    for (size_t i = 0; i < m_NoLoadEffects; i++) {
        m_LoadEffectValues[i].push_back(vEffs[i]);
    }
}

void CFatigueManager::cleanLoadEffectValues () {
    m_LoadEffectValues = std::vector< std::vector<double> >(m_NoLoadEffects);
}

void CFatigueManager::Update()
{
    doRainflow(m_LoadEffectValues);
    cleanLoadEffectValues();
}

// run rainflow for each load event of the bridge
void CFatigueManager::doRainflow(std::vector< std::vector<double> >& signalData) {
    for (size_t i = 0; i < m_NoLoadEffects; i++) {
        std::vector< std::pair<double, double> > rainflowOut = m_RainflowAlg.countCycles(signalData[i], 3);  // ndigits = 3
        countRainflow(rainflowOut,i);
    }
}

// count the rainflow output from doRainflow()
void CFatigueManager::countRainflow(std::vector< std::pair<double, double> >& rainflowOut, size_t i) {
    for (size_t j = 0; j < rainflowOut.size(); j++) {
        m_RainflowOutCount[i][rainflowOut[j].first] += rainflowOut[j].second;
    }
}

void CFatigueManager::CheckBuffer(bool bForceOutput)
{
    if (bForceOutput) {
        WriteFiles();
    }
}

void CFatigueManager::WriteFiles () {
    for (size_t i = 0; i < m_NoLoadEffects; i++) {
        std::map<double, double>::iterator iter;
        std::string file;
        file = "RC_" + to_string(m_BridgeLength) + "_" + to_string(i+1) + ".txt";
        m_RainflowOutFile.open(file.c_str(),std::ios::out);

        m_RainflowOutFile << "Amplitude" << "\t\t" << "No. Cycles" << "\n";
        for (iter = m_RainflowOutCount[i].begin(); iter != m_RainflowOutCount[i].end(); iter++) {
            m_RainflowOutFile << std::fixed << std::setprecision(3) << iter->first << "\t\t\t" << std::setprecision(1) << iter->second << "\n";
        }
        m_RainflowOutFile.close();
    }
}
