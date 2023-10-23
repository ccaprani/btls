#include "FatigueManager.h"


CFatigueManager::CFatigueManager(CConfigDataCore& config) : COutputManagerBase("RC")
{
	RAINFLOW_DECIMAL    = config.Output.Fatigue.RAINFLOW_DECIMAL;
	RAINFLOW_CUTOFF     = config.Output.Fatigue.RAINFLOW_CUTOFF;
	WRITE_BUFFER_SIZE   = config.Output.Fatigue.WRITE_RAINFLOW_BUFFER_SIZE;

	m_WriteHeadLine = true;
	m_EventCount = 0;
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

void CFatigueManager::addLoadEffectValues(std::vector<double> vEffs) {
	for (size_t i = 0; i < m_NoLoadEffects; i++) {
		m_LoadEffectValues[i].push_back(vEffs[i]);
	}
}

void CFatigueManager::cleanLoadEffectValues() {
	m_LoadEffectValues = std::vector< std::vector<double> >(m_NoLoadEffects);
}

void CFatigueManager::cleanRainflowOutCountValues() {
	m_RainflowOutCount = std::vector< std::map<double, double> >(m_NoLoadEffects);
	m_EventCount = 0;
}

void CFatigueManager::Update()
{
	doRainflow(m_LoadEffectValues);
	cleanLoadEffectValues();
	CheckBuffer(false);
}

// run rainflow for each load event of the bridge
void CFatigueManager::doRainflow(std::vector< std::vector<double> >& signalData) {
	for (size_t i = 0; i < m_NoLoadEffects; i++) {
		std::vector< std::pair<double, double> > rainflowOut = m_RainflowAlg.countCycles(signalData[i], RAINFLOW_DECIMAL);
		countRainflow(rainflowOut,i);
	}
	m_EventCount++;
}

// count the rainflow output from doRainflow()
void CFatigueManager::countRainflow(std::vector< std::pair<double, double> >& rainflowOut, size_t i) {
	for (size_t j = 0; j < rainflowOut.size(); j++) {
		if (rainflowOut[j].first >= RAINFLOW_CUTOFF) {
			m_RainflowOutCount[i][rainflowOut[j].first] += rainflowOut[j].second;
		}
	}
}

void CFatigueManager::CheckBuffer(bool bForceOutput)
{
	if (m_EventCount >= WRITE_BUFFER_SIZE || bForceOutput) {
		WriteBuffer();
	}
}

void CFatigueManager::WriteBuffer() {
	for (size_t i = 0; i < m_NoLoadEffects; i++) {
		std::map<double, double>::iterator iter;
		std::string file;
		file = "RC_" + to_string(m_BridgeLength) + "_" + to_string(i+1) + ".txt";
		std::ostringstream oStr;
		if (m_WriteHeadLine) {
			m_RainflowOutFile.open(file.c_str(),std::ios::out);
			oStr.width(15);   oStr << "Amplitude";
			oStr.width(15);   oStr << "No. Cycles"; 
			m_RainflowOutFile << oStr.str() << '\n';
			oStr.str("");  // clear the stringstream
		}
		else {
			m_RainflowOutFile.open(file.c_str(),std::ios::app);
		}
		for (iter = m_RainflowOutCount[i].begin(); iter != m_RainflowOutCount[i].end(); iter++) {
			oStr.width(15);   oStr << std::fixed << std::setprecision(RAINFLOW_DECIMAL) << iter->first;
			oStr.width(10);   oStr << std::fixed << std::setprecision(1) << iter->second;
			m_RainflowOutFile << oStr.str() << '\n';
			oStr.str("");  // clear the stringstream
		}
		m_RainflowOutFile.close();
	}
	m_WriteHeadLine = false;
	cleanRainflowOutCountValues();
}
