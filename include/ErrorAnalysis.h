#ifndef ERRORANALYSIS_H
#define ERRORANALYSIS_H

// This includes the shared structures, statistics helper functions, the string type, and the vector type
#include "Types.h"
#include "StatsUtil.h"
#include <string>
#include <vector>

using namespace std;

/* This structure stores one scored pair - the image, the reference and candidate text, and the metrics
It is used by experiment mode to collect all the results before performing error analysis */
struct ScoredPair {
    string imageName;        // Image name
    string referenceText;    // Reference text
    string candidateText;    // Candidate (tested) text
    MetricResult metrics;    // Calculated metrics for this pair
};

/* This class performs a detailed error analysis after experiment mode has gone through all (or a large part) of the Flickr8k dataset. In the previous versions of the program, experiment
mode only printed the average score - now I additionally calculate the distribution, the worst and best cases, and the correlation between metrics. */
class ErrorAnalysis {
private:
    StatsUtil statsUtil; // Statistics helper object

    // I count how many pairs fall into each of the 5 level categories, in fixed order
    vector<int> countLevelBuckets(vector<ScoredPair> pairs);

    // I format the entire report as text, which I then write to a file
    string formatReport(vector<ScoredPair> pairs, vector<ScoredPair> worstPairs, vector<ScoredPair> bestPairs, int topN);

public:
    // Here I perform a full error analysis and write the result to the specified file
    void runAnalysis(vector<ScoredPair> pairs, string outputPath, int topN);
};

#endif
