/*
Implements rainflow cycle counting algorythm for CFatigueManager
according to section 5.4.4 in ASTM E1049-85 (2011).
*/
#pragma once

#include <algorithm>
#include <deque>
#include <map>
#include <math.h>
#include <utility>
#include <vector>

class CRainflow
{
public:
	struct ExtractCycleOut
	{
		double range;
		double mean;
		double count;
	};

public:
	CRainflow() {};
	CRainflow(int decimal, double cutoff) : m_Decimal(decimal), m_Cutoff(cutoff) {};
	~CRainflow() {};

	void clearRainflowOutput();

	void processData(const std::vector<double> &series);
	void calcCycles(bool bIsFinal);

	const std::map<double, double>& getRainflowOutput() const { return m_RainflowOutput; }

private:
	double doRoundUp(double x) const;
	ExtractCycleOut formatOutput(double point1, double point2, double count) const;
	template <typename T>
	std::vector<std::pair<T, T>> mapToVector(const std::map<T, T> &inputMap) const;

	std::vector<double> extractReversals(const std::vector<double> &series) const;
	std::vector<CRainflow::ExtractCycleOut> extractCycles() const;
	std::vector<std::pair<double, double>> countCycles(const std::vector<CRainflow::ExtractCycleOut> &cycles) const;

	std::vector<double> m_vReversals;
	std::map<double, double> m_RainflowOutput;

	int m_Decimal;
	double m_Cutoff;
};
