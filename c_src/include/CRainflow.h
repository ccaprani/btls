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

using namespace std;


class Rainflow {
public: 
    Rainflow ();
    struct ExtractCycleOut {
        double rng;
        double mean;
        double count;
        size_t i1;
        size_t i2;
    };
    vector<Rainflow::ExtractCycleOut> extract_cycles (vector<double>& series);
    vector< pair<double, double> > count_cycles (vector<double>& series, int ndigits = -1, int nbins = -1, double binsize = -1.0);

private:
    double get_round_function (double x, int ndigits = -1);
    vector< pair<size_t, double> > reversals (vector<double>& series);
    ExtractCycleOut format_output (pair<size_t, double> point1, pair<size_t, double> point2, double count);
    vector< pair<double, double> > map_to_vector (const map<double, double> &map);
};
