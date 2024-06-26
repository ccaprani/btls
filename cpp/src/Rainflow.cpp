# include "Rainflow.h"


CRainflow::CRainflow () {};

double CRainflow::getRoundFunction (double x, int ndigits) {
    if (ndigits < 0) {
        return x;
    }
    else {
        return round(x*pow(10,ndigits))/pow(10,ndigits);
    }
};

std::vector< std::pair<size_t, double> > CRainflow::reversals (std::vector<double>& series) {
    std::vector< std::pair<size_t, double> > reversals_out;
    // cout << series.size() << endl;
    double x_last = series[0];
    double x = series[1];
    double d_last = (x-x_last);
    reversals_out.push_back(std::make_pair(0,x_last));
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
            reversals_out.push_back(std::make_pair(index,x));
        }
        x_last = x;
        x = x_next;
        d_last = d_next;
    }
    if (index != 0) {
        reversals_out.push_back(std::make_pair(index+1,x_next));
    }
    return reversals_out;
};

CRainflow::ExtractCycleOut CRainflow::formatOutput (std::pair<size_t, double> point1, std::pair<size_t, double> point2, double count) {
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

std::vector<CRainflow::ExtractCycleOut> CRainflow::extractCycles (std::vector<double>& series) {
    std::vector< std::pair<size_t, double> > reversals_out = reversals(series);
    std::deque< std::pair<size_t, double> > points;
    std::vector<ExtractCycleOut> format_output_out;
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
                format_output_out.push_back(formatOutput(points[0],points[1],0.5));
                points.pop_front();
            }
            else {
                // Count Y as one cycle and discard the peak and the valley of Y
                format_output_out.push_back(formatOutput(points.at(points.size()-3),points.at(points.size()-2),1.0));
                std::pair<size_t ,double> last = points.back();
                points.pop_back();
                points.pop_back();
                points.pop_back();
                points.push_back(last);
            }
        }
    }
    while (points.size() > 1) {
        format_output_out.push_back(formatOutput(points[0],points[1],0.5));
        points.pop_front();
    }
    return format_output_out;
};

std::vector< std::pair<double, double> > CRainflow::countCycles (std::vector<double>& series, int ndigits, int nbins, double binsize) {
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
    std::map<double, double> counts;
    if (nbins > 0 && binsize > 0.0) {
        std::cout << "Arguments nbins and binsize are mutually exclusive!" << std::endl;
        return mapToVector(counts);
    }
    std::vector<ExtractCycleOut> cycles = extractCycles(series);
    // cout << cycles.size() << endl;
    if (nbins > 0) {
        double maxValue = *max_element(series.begin(),series.end()); 
        double minValue = *min_element(series.begin(),series.end());
        binsize = (maxValue-minValue)/nbins;
    }
    if (binsize > 0) {
        int nmax = 0;
        for (size_t i = 0; i < cycles.size(); i++) {
            double quotient = cycles[i].rng/binsize;
            int n = (int)ceil(quotient);
            if (nbins > 0 && n > nbins) {
                n -= 1;
            }
            counts[(double)n*binsize] += cycles[i].count;
            nmax = (std::max)(n,nmax);
        }
        for (int i = 1; i < nmax; i++) {
            counts[(double)i*binsize] += 0.0;
        }
    }
    else if (ndigits >= 0) {
        for (size_t i = 0; i < cycles.size(); i++) {
            counts[getRoundFunction(cycles[i].rng,ndigits)] += cycles[i].count;
        }
    }
    else {
        for (size_t i = 0; i < cycles.size(); i++) {
            counts[cycles[i].rng] += cycles[i].count;
        }
    }
    return mapToVector(counts);
};

template <typename T>
std::vector< std::pair<T, T> > CRainflow::mapToVector (const std::map<T, T>& map) {
        return std::vector< std::pair<T, T> >(map.begin(), map.end());
}
