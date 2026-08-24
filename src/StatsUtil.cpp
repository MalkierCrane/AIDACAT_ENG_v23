// This includes the StatsUtil class declaration and math functions
#include "StatsUtil.h"
#include <cmath>

using namespace std;

/* This function sorts a list of numbers in ascending order using a simple insertion sort - the lists here
are not too large (the median and top-N only need small lists, not the entire list of 32000 pairs at once) */
vector<double> StatsUtil::sortAscending(vector<double> values) {

    vector<double> result = values;  // Here I work with a copy, the original list remains untouched

    // The outer loop goes through all the elements
    for (int i = 1; i < (int)result.size(); i++) {
        double current = result[i];
        int j = i - 1;

        // The inner loop shifts the larger elements one position forward
        while (j >= 0 && result[j] > current) {
            result[j + 1] = result[j];
            j--;
        }

        result[j + 1] = current;
    }

    return result;
}

// This function calculates the mean value
double StatsUtil::calcMean(vector<double> values) {
    if (values.size() == 0) {
        return 0.0;
    }

    double sum = 0.0;

    for (int i = 0; i < (int)values.size(); i++) {
        sum += values[i];
    }

    return sum / (double)values.size();
}

// This function calculates the standard deviation around the given mean value
double StatsUtil::calcStdDev(vector<double> values, double meanValue) {
    
    // If there are fewer than 2 values, the standard deviation cannot be meaningfully calculated
    if (values.size() < 2) {
        return 0.0;
    }

    double sumSquares = 0.0;

    for (int i = 0; i < (int)values.size(); i++) {
        double diff = values[i] - meanValue;
        sumSquares += diff * diff;
    }

    return sqrt(sumSquares / (double)values.size());
}

// This function calculates all the statistics at once for one list
MetricStats StatsUtil::calcStats(vector<double> values) {

    MetricStats stats;

    if (values.size() == 0) {
        stats.mean = 0.0;
        stats.median = 0.0;
        stats.stdDev = 0.0;
        stats.minVal = 0.0;
        stats.maxVal = 0.0;
        return stats;
    }

    stats.mean = calcMean(values);
    stats.stdDev = calcStdDev(values, stats.mean);

    // Here I sort the list, so that I can find the median and min/max from it
    vector<double> sorted = sortAscending(values);
    stats.minVal = sorted[0];
    stats.maxVal = sorted[sorted.size() - 1];

    // For an even length, the median is the average of the two middle elements, for an odd length - the middle element itself
    int middle = (int)sorted.size() / 2;
    if (sorted.size() % 2 == 0) {
        stats.median = (sorted[middle - 1] + sorted[middle]) / 2.0;
    }
    else {
        stats.median = sorted[middle];
    }

    return stats;
}

/* This function calculates the Pearson correlation between two lists of equal length - it shows to what
extent finalScore agrees with another value (for example, an expert's score). */
double StatsUtil::pearsonCorrelation(vector<double> x, vector<double> y) {

    // If the lists are not equal length or are too small, the correlation cannot be calculated
    if (x.size() != y.size() || x.size() < 2) {
        return 0.0;
    }

    double meanX = calcMean(x);
    double meanY = calcMean(y);

    double numerator = 0.0;
    double sumSquareX = 0.0;
    double sumSquareY = 0.0;

    // This loop accumulates the numerator and both denominator parts for all pairs
    for (int i = 0; i < (int)x.size(); i++) {
        double diffX = x[i] - meanX;
        double diffY = y[i] - meanY;

        numerator += diffX * diffY;
        sumSquareX += diffX * diffX;
        sumSquareY += diffY * diffY;
    }

    double denominator = sqrt(sumSquareX) * sqrt(sumSquareY);

    // If the denominator is 0, one of the lists has no variance, the correlation cannot be calculated
    if (denominator == 0.0) {
        return 0.0;
    }

    return numerator / denominator;
}
