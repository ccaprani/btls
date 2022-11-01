/*
Implements rainflow cycle counting algorythm for fatigue analysis
according to section 5.4.4 in ASTM E1049-85 (2011).
*/
#pragma once

#include <algorithm>
#include <deque>
#include <iostream>
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
        size_t i1;
        size_t i2;
    };
    std::vector<CRainflow::ExtractCycleOut> extractCycles (std::vector<double>& series);
    std::vector< std::pair<double, double> > countCycles (std::vector<double>& series, int ndigits = -1, int nbins = -1, double binsize = -1.0);

private:
    double getRoundFunction (double x, int ndigits = -1);
    std::vector< std::pair<size_t, double> > reversals (std::vector<double>& series);
    ExtractCycleOut formatOutput (std::pair<size_t, double> point1, std::pair<size_t, double> point2, double count);
    template <typename T> std::vector< std::pair<T, T> > mapToVector (const std::map<T, T> &map);
};
