#ifndef CALIBRATION_H
#define CALIBRATION_H

// This includes the common structures, the metrics calculation class, the semantic similarity class, statistics helper functions, the text type, and the list type
#include "Types.h"
#include "Metrics.h"
#include "SemanticSimilarity.h"
#include "StatsUtil.h"
#include <string>
#include <vector>

using namespace std;

/* The structure stores one row from ExpertAnnotations.txt or CrowdFlowerAnnotations.txt.
IMPORTANT: each row contains TWO image names, not one - judgedImageName is the image being judged, and candidateImageName#candidateIndex is the source of the caption that
experts evaluated - in MOST cases this is a different image, not the same one (see the plan note about the 5664/5822 Expert rows and the 47109/47830 CrowdFlower rows, where these
two images differ - this is the original paper's "ranking task" data structure). */
struct ExpertRow {
    string judgedImageName;       // The image being judged - its own human captions serve as our reference text
    string candidateImageName;    // The image from which the evaluated caption comes
    int candidateIndex;           // The caption index (#0..#4) for the candidateImageName image
    double groundTruthScore;      // The expert's rating, already converted to a 0-100 scale
};

// The structure stores the list of loaded rows along with how many rows had to be skipped (malformed format)
struct LoadedAnnotationRows {
    vector<ExpertRow> rows;  // Valid, loaded rows
    int skippedCount;        // How many rows were skipped due to parsing
};

/* This class compares the scores I calculated myself with real human ratings from ExpertAnnotations.txt / CrowdFlowerAnnotations.txt, and tries to find a weight profile
that better matches the human ratings. This is NOT an automatic adjuster that rewrites Coefficients.cpp - it only writes a suggestion to a report file, which a person can
optionally put into the Coefficients.cpp file themselves. */
class Calibration {
private:
    StatsUtil statsUtil; // Statistics helper object

    LoadedAnnotationRows loadExpertAnnotations(string filePath, int limit);        // Loads ExpertAnnotations.txt
    LoadedAnnotationRows loadCrowdflowerAnnotations(string filePath, int limit);   // Loads CrowdFlowerAnnotations.txt

    // Helper functions that convert a WeightProfile to a simple list of 8 numbers and back - this simplifies the weight-search loop, where you need to go through all 8 weights in order
    vector<double> profileToArray(WeightProfile profile);
    WeightProfile arrayToProfile(vector<double> values, string name);
    vector<double> normalizeWeightArray(vector<double> values); // Makes the 8 numbers sum to 1.0

    // This recalculates final_score for all results with the given profile and returns the correlation with human ratings
    double evaluateProfileCorrelation(vector<MetricResult> ourResults, vector<double> groundTruthScores, WeightProfile profile);

    /* Searches for an improved weight profile using the coordinate ascent / hill-climbing method -
    a full grid search across all 8 dimensions would not be practically feasible (too many combinations) */
    WeightProfile hillClimbSearch(vector<MetricResult> ourResults, vector<double> groundTruthScores, WeightProfile startProfile);

    // Writes the final calibration report
    void writeReport(string outputPath, string sourceLabel, int loadedCount, int skippedRowsCount, int skippedLookupCount,
                      vector<MetricResult> ourResults, vector<double> groundTruthScores,
                      WeightProfile startProfile, WeightProfile discoveredProfile);

public:
    /* Performs a full calibration cycle for one source file (Expert or CrowdFlower). flickrItems must be loaded WITHOUT a limit (limit=0), so that all images referenced by
    the annotation file can be found. metricsEngine and semanticSimilarity are already prepared (the Python path and model name are set) - Calibration only uses them. */
    void runCalibration(string annotationFile, vector<FlickrItem> flickrItems, CorpusStats corpusStats,
                         string outputPath, int limit, bool useCrowdflower,
                         Metrics metricsEngine, SemanticSimilarity semanticSimilarity);
};

#endif
