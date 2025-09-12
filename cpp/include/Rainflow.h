/*
Implements rainflow cycle counting algorythm for fatigue analysis
according to section 5.4.4 in ASTM E1049-85 (2011).
*/
#pragma once

#include <algorithm>
#include <deque>
#include <map>
#include <math.h>
#include <utility>
#include <vector>


class CRainflow {
public: 
    CRainflow ();
    struct ExtractCycleOut {
        double rng;
        double mean;
        double count;
    };
    std::vector<double> extractReversals (std::vector<double>& series);
    std::vector<CRainflow::ExtractCycleOut> extractCycles (std::vector<double>& reversals);
    std::vector< std::pair<double, double> > countCycles (std::vector<CRainflow::ExtractCycleOut>& cycles, int ndigits);

private:
    double getRoundFunction (double x, int ndigits = -1);
    ExtractCycleOut formatOutput (double point1, double point2, double count);
    template <typename T> std::vector< std::pair<T, T> > mapToVector (const std::map<T, T> &map);
};
