#ifndef STATSUTIL_H
#define STATSUTIL_H

// I include the vector type
#include <vector>

using namespace std;

// This structure stores simple statistics for one list of numbers
struct MetricStats {
    double mean;        // Mean value
    double median;      // Median
    double stdDev;      // Standard deviation
    double minVal;      // Smallest value
    double maxVal;      // Largest value
};

// This class contains simple statistics functions used together by ErrorAnalysis and Calibration
class StatsUtil {
public:
    vector<double> sortAscending(vector<double> values);           // This sorts the list in ascending order (insertion sort)
    double calcMean(vector<double> values);                        // This calculates the mean value
    double calcStdDev(vector<double> values, double meanValue);    // This calculates the standard deviation
    MetricStats calcStats(vector<double> values);                  // This calculates all the statistics at once
    double pearsonCorrelation(vector<double> x, vector<double> y); // This calculates the Pearson correlation between two lists
};

#endif
