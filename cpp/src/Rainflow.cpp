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
	if (series.size() < 2)
		return series;	// guard: a 0/1-point series has no reversals to eliminate

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
	if (bIsFinal)
		m_vReversals.push_back(0.0);  // Sim should be end at 0.0

	if (m_vReversals.size() < 2)
	{
		if (bIsFinal)
			m_vReversals.clear();
		return;
	}

	m_vReversals = extractReversals(m_vReversals);  // To eliminate the fake reversals between events.

	// Closed cycles are counted now; the unclosed residual is carried over
	// to the next buffer (or closed out as half cycles when bIsFinal), so
	// the result does not depend on the buffer cadence.
	std::vector<double> residual;
	std::vector<CRainflow::ExtractCycleOut> cycles = extractCycles(bIsFinal, residual);
	m_vReversals = residual;

	std::vector<std::pair<double, double>> rainflowOut = countCycles(cycles);
	for (size_t i = 0; i < rainflowOut.size(); i++)
	{
		if (rainflowOut[i].first >= m_Cutoff)
		{
			m_RainflowOutput[rainflowOut[i].first] += rainflowOut[i].second;
		}
	}
}

std::vector<CRainflow::ExtractCycleOut> CRainflow::extractCycles(
	bool bCloseResidual, std::vector<double> &residualOut) const
{
	std::deque<double> points;
	std::vector<ExtractCycleOut> cycles;
	double x1;
	double x2;
	double x3;
	double x4;

	for (size_t i = 0; i < m_vReversals.size(); i++)
	{
		points.push_back(m_vReversals[i]);
		// Four-point closure: a cycle (x2,x3) closes when its range is
		// bracketed by its neighbours. This rule is independent of the
		// points in front of the window, so the residual concatenation of
		// consecutive chunks reproduces a whole-series count exactly, and
		// (with the half-cycle closure of the residual at end of data) it
		// is equivalent to the ASTM E1049-85 one-pass count.
		while (points.size() >= 4)
		{
			x1 = points.at(points.size() - 4);
			x2 = points.at(points.size() - 3);
			x3 = points.at(points.size() - 2);
			x4 = points.at(points.size() - 1);

			if (abs(x3 - x2) <= abs(x2 - x1) && abs(x3 - x2) <= abs(x4 - x3))
			{
				// Count (x2,x3) as one cycle and discard its peak and valley
				cycles.push_back(formatOutput(x2, x3, 1.0));
				points.pop_back();
				points.pop_back();
				points.pop_back();
				points.push_back(x4);
			}
			else
			{
				// Read the next point
				break;
			}
		}
	}

	if (bCloseResidual)
	{
		// End of data (ASTM E1049-85 rule 5): each remaining range counts
		// as one-half cycle, front to back.
		while (points.size() > 1)
		{
			cycles.push_back(formatOutput(points[0], points[1], 0.5));
			points.pop_front();
		}
		residualOut.clear();
	}
	else
	{
		residualOut.assign(points.begin(), points.end());
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
