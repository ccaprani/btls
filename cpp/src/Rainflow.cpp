#include "Rainflow.h"

void CRainflow::clearRainflowOutput()
{
	m_RainflowOutput.clear();
};

double CRainflow::doRoundUp(double x) const
{
	if (m_Decimal < 0)
	{
		return x;
	}
	else
	{
		return round(x * pow(10, m_Decimal)) / pow(10, m_Decimal);
	}
};

void CRainflow::processData(const std::vector<double> &series)
{
	std::vector<double> tempVec;
	tempVec = extractReversals(series);
	m_vReversals.insert(m_vReversals.end(), tempVec.begin(), tempVec.end());
}

std::vector<double> CRainflow::extractReversals(const std::vector<double> &series) const
{
	std::vector<double> reversalsOut;

	double xLast = series[0];
	double x = series[1];
	double dLast = (x - xLast);
	reversalsOut.push_back(xLast);
	double xNext;

	for (size_t i = 2; i < series.size(); i++)
	{
		xNext = series[i];
		if (xNext == x)
		{
			continue;
		}
		double dNext = xNext - x;
		if (dLast * dNext < 0.0)
		{
			reversalsOut.push_back(x);
		}
		xLast = x;
		x = xNext;
		dLast = dNext;
	}

	if (series.size() > 2)
	{
		reversalsOut.push_back(xNext);
	}
	else
	{
		reversalsOut.push_back(x);
	}

	return reversalsOut;
};

CRainflow::ExtractCycleOut CRainflow::formatOutput(double point1, double point2, double count) const
{
	ExtractCycleOut formatOutputReturn;
	formatOutputReturn.range = abs(point1 - point2);
	formatOutputReturn.mean = 0.5 * (point1 + point2);
	formatOutputReturn.count = count;
	return formatOutputReturn;
};

// The start and the end of each event are always extracted as reversals, so they can be fake reversals. 
void CRainflow::calcCycles(bool bIsFinal)
{
	std::vector<CRainflow::ExtractCycleOut> cycles;

	if (bIsFinal)  // Sim end case
	{
		m_vReversals.push_back(0.0);  // Sim should be end at 0.0
		m_vReversals = extractReversals(m_vReversals);  // To eliminate the fake reversals between events. 

		cycles = extractCycles();
	}
	else  // No. event buffer reached case
	{
		m_vReversals = extractReversals(m_vReversals);  // To eliminate the fake reversals between events.

		double lastReversal = m_vReversals.back();
		m_vReversals.pop_back();
		double secondLastReversal = m_vReversals.back();

		cycles = extractCycles();

		m_vReversals.clear();
		m_vReversals.push_back(secondLastReversal);
		m_vReversals.push_back(lastReversal);
	}

	std::vector<std::pair<double, double>> rainflowOut = countCycles(cycles);
	for (size_t i = 0; i < rainflowOut.size(); i++)
	{
		if (rainflowOut[i].first >= m_Cutoff)
		{
			m_RainflowOutput[rainflowOut[i].first] += rainflowOut[i].second;
		}
	}
}

// Have not added the check for 1-reversal case, but that case is impossible considering the buffer_size.
std::vector<CRainflow::ExtractCycleOut> CRainflow::extractCycles() const
{
	std::deque<double> points;
	std::vector<ExtractCycleOut> cycles;
	double x1;
	double x2;
	double x3;
	double X;
	double Y;
	
	for (size_t i = 0; i < m_vReversals.size(); i++)
	{
		points.push_back(m_vReversals[i]);
		while (points.size() >= 3)
		{
			// Form ranges X and Y from the three most recent points
			x1 = points.at(points.size() - 3);
			x2 = points.at(points.size() - 2);
			x3 = points.at(points.size() - 1);
			X = abs(x3 - x2);
			Y = abs(x2 - x1);

			if (X < Y)
			{
				// Read the next point
				break;
			}
			else if (points.size() == 3)
			{
				// Y contains the starting point
				// Count Y as one-half cycle and discard the first point
				cycles.push_back(formatOutput(points[0], points[1], 0.5));
				points.pop_front();
			}
			else
			{
				// Count Y as one cycle and discard the peak and the valley of Y
				cycles.push_back(formatOutput(points.at(points.size() - 3), points.at(points.size() - 2), 1.0));
				double last = points.back();
				points.pop_back();
				points.pop_back();
				points.pop_back();
				points.push_back(last);
			}
		}
	}
	while (points.size() > 1)
	{
		cycles.push_back(formatOutput(points[0], points[1], 0.5));
		points.pop_front();
	}
	return cycles;
};

std::vector<std::pair<double, double>> CRainflow::countCycles(const std::vector<CRainflow::ExtractCycleOut> &cycles) const
{
	std::map<double, double> counts;
	for (size_t i = 0; i < cycles.size(); i++)
	{
		counts[doRoundUp(cycles[i].range)] += cycles[i].count;
	}
	return mapToVector(counts);
};

template <typename T>
std::vector<std::pair<T, T>> CRainflow::mapToVector(const std::map<T, T> &inputMap) const
{
	return std::vector<std::pair<T, T>>(inputMap.begin(), inputMap.end());
}
