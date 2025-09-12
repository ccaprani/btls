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

std::vector<double> CRainflow::extractReversals (std::vector<double>& series) {
    std::vector<double> reversals_out;
    // std::cout << series.size() << std::endl;

    double x_last = series[0];
    double x = series[1];
    double d_last = (x-x_last);
    reversals_out.push_back(x_last);
    double x_next;

    for (size_t i = 2; i < series.size(); i++) {
        x_next = series[i];
        if (x_next == x) {
            continue;
        }
        double d_next = x_next - x;
        if (d_last * d_next < 0.0) {
            reversals_out.push_back(x);
        }
        x_last = x;
        x = x_next;
        d_last = d_next;
    }
    reversals_out.push_back(x_next);
    
    return reversals_out;
};

CRainflow::ExtractCycleOut CRainflow::formatOutput (double point1, double point2, double count) {
    ExtractCycleOut format_output_return;
    format_output_return.rng = abs(point1-point2);
    format_output_return.mean = 0.5*(point1+point2);
    format_output_return.count = count;
    return format_output_return;
};

std::vector<CRainflow::ExtractCycleOut> CRainflow::extractCycles (std::vector<double>& reversals) {
    std::deque<double> points;
    std::vector<ExtractCycleOut> format_output_out;
    double x1;
    double x2;
    double x3;
    double X;
    double Y;
    // cout << reversals.size() << endl;
    for (size_t i = 0; i < reversals.size(); i++) {
        points.push_back(reversals[i]);
        while (points.size() >= 3) {
            // Form ranges X and Y from the three most recent points
            x1 = points.at(points.size()-3);
            x2 = points.at(points.size()-2);
            x3 = points.at(points.size()-1);
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
                double last = points.back();
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

std::vector< std::pair<double, double> > CRainflow::countCycles (std::vector<CRainflow::ExtractCycleOut>& cycles, int ndigits) {
    std::map<double, double> counts;
    for (size_t i = 0; i < cycles.size(); i++) {
        counts[getRoundFunction(cycles[i].rng, ndigits)] += cycles[i].count;
    }
    return mapToVector(counts);
};

template <typename T>
std::vector< std::pair<T, T> > CRainflow::mapToVector (const std::map<T, T>& map) {
        return std::vector< std::pair<T, T> >(map.begin(), map.end());
}
