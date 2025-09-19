#include "FatigueManager.h"

CFatigueManager::CFatigueManager(CConfigDataCore &config) : COutputManagerBase("F")
{
	DO_FATIGUE_RAINFLOW = config.Output.Fatigue.DO_FATIGUE_RAINFLOW;
	RAINFLOW_DECIMAL = config.Output.Fatigue.RAINFLOW_DECIMAL;
	RAINFLOW_CUTOFF = config.Output.Fatigue.RAINFLOW_CUTOFF;

	WRITE_BUFFER_SIZE = config.Output.Fatigue.WRITE_FATIGUE_BUFFER_SIZE;

	m_WriteRainflowHeadLine = true;
	m_EventCount = 0;
}

CFatigueManager::~CFatigueManager(void)
{
}

void CFatigueManager::Initialize(double bridgeLength, size_t noLoadEffects)
{
	m_BridgeLength = bridgeLength;
	m_NoLoadEffects = noLoadEffects;
	m_vLoadEffectValues = std::vector<std::vector<double>>(m_NoLoadEffects);

	if (DO_FATIGUE_RAINFLOW) m_vRainflow = std::vector<CRainflow>(m_NoLoadEffects, CRainflow{RAINFLOW_DECIMAL, RAINFLOW_CUTOFF});
}

void CFatigueManager::addLoadEffectValues(std::vector<double> vEffs)
{
	for (size_t i = 0; i < m_NoLoadEffects; i++)
	{
		m_vLoadEffectValues[i].push_back(vEffs[i]);
	}
}

void CFatigueManager::cleanLoadEffectValues()
{
	m_vLoadEffectValues = std::vector<std::vector<double>>(m_NoLoadEffects);
}

void CFatigueManager::Update()
{
	m_EventCount++;

	for (size_t i = 0; i < m_NoLoadEffects; i++)
	{
		if (DO_FATIGUE_RAINFLOW) m_vRainflow[i].processData(m_vLoadEffectValues[i]);
	}
	cleanLoadEffectValues();

	CheckBuffer(false);
}

void CFatigueManager::CheckBuffer(bool bForceOutput)
{
	if (m_EventCount >= WRITE_BUFFER_SIZE || bForceOutput)
	{
		if (DO_FATIGUE_RAINFLOW) 
		{
			doRainflow(bForceOutput);
			writeRainflowBuffer();
		}

		m_EventCount = 0;
	}
}

void CFatigueManager::doRainflow(bool bIsFinal)
{
	for (size_t i = 0; i < m_NoLoadEffects; i++)
	{
		m_vRainflow[i].calcCycles(bIsFinal);
	}
}

void CFatigueManager::writeRainflowBuffer()
{
	for (size_t i = 0; i < m_NoLoadEffects; i++)
	{
		std::string file;
		file = m_FileStem + "R_" + to_string(m_BridgeLength) + "_" + to_string(i + 1) + ".txt";  // "FR_*.txt"
		std::ostringstream oStr;

		if (m_WriteRainflowHeadLine)
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

		const std::map<double, double>& rainflowOut = m_vRainflow[i].getRainflowOutput();
		for (const auto& [range, count] : rainflowOut)
		{
			oStr.width(15);
			oStr << std::fixed << std::setprecision(RAINFLOW_DECIMAL) << range;
			oStr.width(10);
			oStr << std::fixed << std::setprecision(1) << count;
			m_RainflowOutFile << oStr.str() << '\n';
			oStr.str(""); // clear the stringstream
		}
		m_RainflowOutFile.close();
	}
	m_WriteRainflowHeadLine = false;
	cleanRainflowOutCountValues();
}

void CFatigueManager::cleanRainflowOutCountValues()
{
	for (size_t i = 0; i < m_NoLoadEffects; i++)
	{
		m_vRainflow[i].clearRainflowOutput();
	}
}
