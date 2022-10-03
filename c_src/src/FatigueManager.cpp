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
	m_RainflowOutCount = std::vector< std::map<double, double> >(m_NoLoadEffects);
}

void CFatigueManager::Update(std::vector< std::vector<double> >& vCurEvents)
{
    doRainflow(vCurEvents);
}

// run rainflow for each load event (from all bridges, all load effects)
void CFatigueManager::doRainflow(std::vector< std::vector<double> >& signal_data) {
    for (size_t i = 0; i < m_NoLoadEffects; i++) {
        std::vector< std::pair<double, double> > rainflow_out = rainflow_alg.countCycles(signal_data[i], 3);  // ndigits = 3
        countRainflow(rainflow_out,i);
    }
}

// count the rainflow output from doRainflow() (from all bridges, all load effects)
void CFatigueManager::countRainflow(std::vector< std::pair<double, double> >& rainflow_out, size_t i) {
	for (size_t j = 0; j < rainflow_out.size(); j++) {
			m_RainflowOutCount[i][rainflow_out[j].first] += rainflow_out[j].second;
	}
}

void CFatigueManager::CheckBuffer(bool bForceOutput)
{
    for (size_t i = 0; i < m_NoLoadEffects; i++) {
        std::map<double, double>::iterator iter;
        std::string file;
        file = "RC_" + std::to_string(m_BridgeLength) + "_" + std::to_string(i+1) + ".txt";
        m_RainflowOutFile.open(file.c_str(),std::ios::out);

        m_RainflowOutFile << "Amplitude" << "\t\t" << "No. Cycles" << "\n";
        for (iter = m_RainflowOutCount[i].begin(); iter != m_RainflowOutCount[i].end(); iter++) {
            m_RainflowOutFile << std::fixed << iter->first << "\t\t" << iter->second << "\n";
        }
        m_RainflowOutFile.close();
    }
}
