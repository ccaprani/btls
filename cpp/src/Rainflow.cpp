#include "Rainflow.h"

CRainflow::CRainflow() {};

double CRainflow::getRoundFunction(double x, int nDigits)
{
	if (nDigits < 0)
	{
		return x;
	}
	else
	{
		return round(x * pow(10, nDigits)) / pow(10, nDigits);
	}
};

std::vector<double> CRainflow::extractReversals(std::vector<double> &series)
{
	std::vector<double> reversalsOut;
	// std::cout << series.size() << std::endl;

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
	reversalsOut.push_back(xNext);

	return reversalsOut;
};

CRainflow::ExtractCycleOut CRainflow::formatOutput(double point1, double point2, double count)
{
	ExtractCycleOut formatOutputReturn;
	formatOutputReturn.range = abs(point1 - point2);
	formatOutputReturn.mean = 0.5 * (point1 + point2);
	formatOutputReturn.count = count;
	return formatOutputReturn;
};

std::vector<CRainflow::ExtractCycleOut> CRainflow::extractCycles(std::vector<double> &reversals)
{
	std::deque<double> points;
	std::vector<ExtractCycleOut> cycles;
	double x1;
	double x2;
	double x3;
	double X;
	double Y;
	// cout << reversals.size() << endl;
	for (size_t i = 0; i < reversals.size(); i++)
	{
		points.push_back(reversals[i]);
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

std::vector<std::pair<double, double>> CRainflow::countCycles(std::vector<CRainflow::ExtractCycleOut> &cycles, int nDigits)
{
	std::map<double, double> counts;
	for (size_t i = 0; i < cycles.size(); i++)
	{
		counts[getRoundFunction(cycles[i].range, nDigits)] += cycles[i].count;
	}
	return mapToVector(counts);
};

template <typename T>
std::vector<std::pair<T, T>> CRainflow::mapToVector(const std::map<T, T> &inputMap)
{
	return std::vector<std::pair<T, T>>(inputMap.begin(), inputMap.end());
}
