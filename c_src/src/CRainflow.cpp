# include "CRainflow.h"


Rainflow::Rainflow () {};

double Rainflow::get_round_function (double x, int ndigits) {
    if (ndigits < 0) {
        return x;
    }
    else {
        return round(x*pow(10,ndigits))/pow(10,ndigits);
    }
};

vector< pair<size_t, double> > Rainflow::reversals (vector<double>& series) {
    /*Iterate reversal points in the series.

    A reversal point is a point in the series at which the first derivative
    changes sign. Reversal is undefined at the first (last) point because the
    derivative before (after) this point is undefined. The first and the last
    points are treated as reversals.

    Parameters
    ----------
    series : iterable sequence of numbers

    Yields
    ------
    Reversal points as tuples (index, value).
    */
    vector< pair<size_t, double> > reversals_out;
    // cout << series.size() << endl;
    double x_last = series[0];
    double x = series[1];
    double d_last = (x-x_last);
    reversals_out.push_back(make_pair(0,x_last));
    size_t index = 0;
    double x_next;
    for (size_t i = 2; i < series.size(); i++) {
        index = i-1;
        x_next = series[i];
        if (x_next == x) {
            continue;
        }
        double d_next = x_next - x;
        if (d_last * d_next < 0) {
            reversals_out.push_back(make_pair(index,x));
        }
        x_last = x;
        x = x_next;
        d_last = d_next;
    }
    if (index != 0) {
        reversals_out.push_back(make_pair(index+1,x_next));
    }
    return reversals_out;
};

Rainflow::ExtractCycleOut Rainflow::format_output (pair<size_t, double> point1, pair<size_t, double> point2, double count) {
    ExtractCycleOut format_output_return;
    double x1 = point1.second;
    double x2 = point2.second;
    format_output_return.rng = abs(x1-x2);
    format_output_return.mean = 0.5*(x1+x2);
    format_output_return.count = count;
    format_output_return.i1 = point1.first;
    format_output_return.i2 = point2.first;
    return format_output_return;
};

vector<Rainflow::ExtractCycleOut> Rainflow::extract_cycles (vector<double>& series) {
    /*Iterate cycles in the series.

    Parameters
    ----------
    series : iterable sequence of numbers

    Yields
    ------
    cycle : tuple
        Each tuple contains (range, mean, count, start index, end index).
        Count equals to 1.0 for full cycles and 0.5 for half cycles.
    */
    vector< pair<size_t, double> > reversals_out = reversals(series);
    deque< pair<size_t, double> > points;
    vector<ExtractCycleOut> format_output_out;
    double x1;
    double x2;
    double x3;
    double X;
    double Y;
    // cout << reversals_out.size() << endl;
    for (size_t i = 0; i < reversals_out.size(); i++) {
        points.push_back(reversals_out[i]);
        while (points.size() >= 3) {
            // Form ranges X and Y from the three most recent points
            x1 = points.at(points.size()-3).second;
            x2 = points.at(points.size()-2).second;
            x3 = points.at(points.size()-1).second;
            X = abs(x3 - x2);
            Y = abs(x2 - x1);

            if (X < Y) {
                // Read the next point
                break;
            }
            else if (points.size() == 3) {
                // Y contains the starting point
                // Count Y as one-half cycle and discard the first point
                format_output_out.push_back(format_output(points[0],points[1],0.5));
                points.pop_front();
            }
            else {
                // Count Y as one cycle and discard the peak and the valley of Y
                format_output_out.push_back(format_output(points.at(points.size()-3),points.at(points.size()-2),1.0));
                pair<size_t ,double> last = points.back();
                points.pop_back();
                points.pop_back();
                points.pop_back();
                points.push_back(last);
            }
        }
    }
    while (points.size() > 1) {
        format_output_out.push_back(format_output(points[0],points[1],0.5));
        points.pop_front();
    }
    return format_output_out;
};

vector< pair<double, double> > Rainflow::count_cycles (vector<double>& series, int ndigits) {
    /*Count cycles in the series.

    Parameters
    ----------
    series : iterable sequence of numbers
    ndigits : int, optional
        Round cycle magnitudes to the given number of digits before counting.
        Use a negative value to round to tens, hundreds, etc.
    nbins : int, optional
        Specifies the number of cycle-counting bins.
    binsize : int, optional
        Specifies the width of each cycle-counting bin

    Arguments ndigits, nbins and binsize are mutually exclusive.

    Returns
    -------
    A sorted list containing pairs of range and cycle count.
    The counts may not be whole numbers because the rainflow counting
    algorithm may produce half-cycles. If binning is used then ranges
    correspond to the right (high) edge of a bin.
    */
    map<double, double> counts;
    vector<ExtractCycleOut> cycles = extract_cycles(series);
    // cout << cycles.size() << endl;
    if (ndigits != -1) {
        for (size_t i = 0; i < cycles.size(); i++) {
            counts[get_round_function(cycles[i].rng,ndigits)] += cycles[i].count;
        }
    }
    else {
        for (size_t i = 0; i < cycles.size(); i++) {
            counts[cycles[i].rng] += cycles[i].count;
        }
    }
    return map_to_vector(counts);
};

vector< pair<double, double> > Rainflow::map_to_vector (const map<double, double>& map) {
        return vector< pair<double, double> >(map.begin(), map.end());
}

