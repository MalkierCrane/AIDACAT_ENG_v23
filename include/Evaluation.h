#ifndef EVALUATION_H
#define EVALUATION_H

/* This includes the metrics, Flickr loader, Florence calling class, downloader, web parser, semantic similarity class, weight profile class,
image function classifier, error analysis class, calibration class, and text type */
#include "Metrics.h"
#include "FlickrLoader.h"
#include "Florence.h"
#include "Downloader.h"
#include "WebParser.h"
#include "SemanticSimilarity.h"
#include "Coefficients.h"
#include "ImagePurpose.h"
#include "ErrorAnalysis.h"
#include "Calibration.h"
#include <string>

using namespace std;

// This is the main class that connects the program's modes
class Evaluation {
private:
    Metrics metrics;                       // The metrics calculation object
    FlickrLoader flickrLoader;             // The Flickr8k file loader
    Florence florence;                     // The Florence-2 calling object
    Downloader downloader;                 // The HTML and image downloader
    WebParser webParser;                   // A simple HTML parser
    SemanticSimilarity semanticSimilarity; // The semantic similarity object
    Coefficients coefficients;             // The weight profile object
    ImagePurpose imagePurpose;             // The image function classifier
    ErrorAnalysis errorAnalysis;           // The error analysis object
    Calibration calibration;               // The calibration object

    CorpusStats corpusStats;             // Cached corpus statistics for the CIDER metric
    bool corpusStatsLoaded = false;      // True if I attempted to load the corpus (regardless of the result)
    bool corpusStatsAvailable = false;   // True if the corpus was loaded successfully and the statistics are valid

    void makeOutputDir();                           // I create the output folder
    string csvSafe(string value);                   // I prepare text for the CSV file
    string joinPath(string folder, string file);    // I join the folder and file path
    string makeFileName(int index);                 // I create a local image file name
    void printMetric(MetricResult result);          // I print the result to the screen

    /* Here I load the corpus statistics from data/Flickr8k_text/Flickr8k.token.txt, if this has not been done yet.
    If the file is not found, a warning is printed and I let the program continue without corpus CIDER. */
    void ensureCorpusStats();

    /* Here I calculate the metrics for one pair - I choose the corpus or non-corpus CIDER version depending
    on whether ensureCorpusStats succeeded, and I add the semantic similarity with the given profile */
    MetricResult scoreTexts(string referenceText, string candidateText, WeightProfile profile);

public:
    void setPythonExe(string path);             // Sets the Python path for both the Florence and SemanticSimilarity objects
    void setModelName(string model);            // Sets the Florence model name
    void setSemanticModelName(string model);    // Sets the sentence-transformers model name

    void runExperiment(string tokenFile, int limit, int topN = 10);                    // Flickr8k validation mode with error analysis
    void runSingle(string imagePath, string altText, bool classifyComplexity = false); // Practical check of a single image
    void runWeb(string url, int limit, bool classifyComplexity = false);               // Real web page mode

    // this is the coefficient calibration mode - compares my score with ExpertAnnotations.txt and, if specified, also with CrowdFlowerAnnotations.txt
    void runCalibration(string expertFile, string tokenFile, int limit, bool useCrowdflower, string crowdflowerFile);
};

#endif
