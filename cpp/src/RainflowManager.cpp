#include "RainflowManager.h"

CRainflowManager::CRainflowManager(CConfigDataCore &config) : COutputManagerBase("RC")
{
	RAINFLOW_DECIMAL = config.Output.Fatigue.RAINFLOW_DECIMAL;
	RAINFLOW_CUTOFF = config.Output.Fatigue.RAINFLOW_CUTOFF;
	WRITE_BUFFER_SIZE = config.Output.Fatigue.WRITE_RAINFLOW_BUFFER_SIZE;

	m_WriteHeadLine = true;
	m_EventCount = 0;
}

CRainflowManager::~CRainflowManager(void)
{
}

void CRainflowManager::Initialize(double bridgeLength, size_t noLoadEffects)
{
	m_BridgeLength = bridgeLength;
	m_NoLoadEffects = noLoadEffects;
	m_vLoadEffectValues = std::vector<std::vector<double>>(m_NoLoadEffects);
	m_vReversals = std::vector<std::vector<double>>(m_NoLoadEffects);
	m_vRainflowOutCounts = std::vector<std::map<double, double>>(m_NoLoadEffects);

	m_bFirstEvent = true;
}

void CRainflowManager::addLoadEffectValues(std::vector<double> vEffs)
{
	for (size_t i = 0; i < m_NoLoadEffects; i++)
	{
		m_vLoadEffectValues[i].push_back(vEffs[i]);
	}
}

void CRainflowManager::cleanLoadEffectValues()
{
	m_vLoadEffectValues = std::vector<std::vector<double>>(m_NoLoadEffects);
}

void CRainflowManager::cleanRainflowOutCountValues()
{
	m_vRainflowOutCounts = std::vector<std::map<double, double>>(m_NoLoadEffects);
	m_EventCount = 0;
}

void CRainflowManager::Update()
{
	m_EventCount++;

	extractReversals();
	cleanLoadEffectValues();

	CheckBuffer(false);  // doRainflow() and WriteBuffer() inside
}

// Extract the reversals from each event.
void CRainflowManager::extractReversals()
{
	std::vector<double> tempVec;
	for (size_t i = 0; i < m_NoLoadEffects; i++)
	{
		tempVec = m_RainflowAlg.extractReversals(m_vLoadEffectValues[i]);
		if (m_bFirstEvent)
		{
			m_vReversals[i] = tempVec;
			m_bFirstEvent = false;
		}
		else
		{
			m_vReversals[i].insert(m_vReversals[i].end(), tempVec.begin(), tempVec.end());
		}
	}
}

// The start and the end of each event are always extracted as reversals, so they can be fake reversals. 
void CRainflowManager::doRainflow(bool bIsFinal)
{
	for (size_t i = 0; i < m_NoLoadEffects; i++)
	{
		std::vector<CRainflow::ExtractCycleOut> cycles;

		if (bIsFinal)  // Sim end case
		{
			m_vReversals[i].push_back(0.0);  // Sim should be end at 0.0
			m_vReversals[i] = m_RainflowAlg.extractReversals(m_vReversals[i]);  // To eliminate the fake reversals between events. 

			cycles = m_RainflowAlg.extractCycles(m_vReversals[i]);
		}
		else  // No. event buffer reached case
		{
			m_vReversals[i] = m_RainflowAlg.extractReversals(m_vReversals[i]);  // To eliminate the fake reversals between events. 

			// To avoid the last reversal being fake. 
			double lastReversal = m_vReversals[i].back();
			m_vReversals[i].pop_back();
			double secondLastReversal = m_vReversals[i].back();

			cycles = m_RainflowAlg.extractCycles(m_vReversals[i]);

			m_vReversals[i].clear();
			m_vReversals[i].push_back(secondLastReversal);
			m_vReversals[i].push_back(lastReversal);
		}

		std::vector<std::pair<double, double>> rainflowOut = m_RainflowAlg.countCycles(cycles, RAINFLOW_DECIMAL);
		accumulateRainflow(rainflowOut, i);
	}
}

// Accumulate the rainflow output
void CRainflowManager::accumulateRainflow(std::vector<std::pair<double, double>> &rainflowOut, size_t i)
{
	for (size_t j = 0; j < rainflowOut.size(); j++)
	{
		if (rainflowOut[j].first >= RAINFLOW_CUTOFF)
		{
			m_vRainflowOutCounts[i][rainflowOut[j].first] += rainflowOut[j].second;
		}
	}
}

void CRainflowManager::CheckBuffer(bool bForceOutput)
{
	if (m_EventCount >= WRITE_BUFFER_SIZE || bForceOutput)
	{
		doRainflow(bForceOutput);
		WriteBuffer();
	}
}

void CRainflowManager::WriteBuffer()
{
	for (size_t i = 0; i < m_NoLoadEffects; i++)
	{
		std::map<double, double>::iterator iter;
		std::string file;
		file = "FR_" + to_string(m_BridgeLength) + "_" + to_string(i + 1) + ".txt";
		std::ostringstream oStr;
		if (m_WriteHeadLine)
		{
			m_RainflowOutFile.open(file.c_str(), std::ios::out);
			oStr.width(15);
			oStr << "Amplitude";
			oStr.width(15);
			oStr << "No. Cycles";
			m_RainflowOutFile << oStr.str() << '\n';
			oStr.str(""); // clear the stringstream
		}
		else
		{
			m_RainflowOutFile.open(file.c_str(), std::ios::app);
		}
		for (iter = m_vRainflowOutCounts[i].begin(); iter != m_vRainflowOutCounts[i].end(); iter++)
		{
			oStr.width(15);
			oStr << std::fixed << std::setprecision(RAINFLOW_DECIMAL) << iter->first;
			oStr.width(10);
			oStr << std::fixed << std::setprecision(1) << iter->second;
			m_RainflowOutFile << oStr.str() << '\n';
			oStr.str(""); // clear the stringstream
		}
		m_RainflowOutFile.close();
	}
	m_WriteHeadLine = false;
	cleanRainflowOutCountValues();
}
