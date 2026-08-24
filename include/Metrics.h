#ifndef METRICS_H
#define METRICS_H

// This includes the common data structures, the text processing class, the text type, the list type and the dictionary type for n-grams
#include "Types.h"
#include "TextProc.h"
#include <string>
#include <vector>
#include <map>

using namespace std;

// This class calculates description similarity metrics
class Metrics {
private:
    TextProc textProc;                                              // Text preparation object
    double safeDivide(double a, double b);                          // Here is safe division with a zero check
    map<string, int> countWords(vector<string> words);              // This counts words
    map<string, int> countNgrams(vector<string> words, int n);      // This counts n-grams
    int countOverlap(map<string, int> a, map<string, int> b);       // This counts matches
    int lcsLength(vector<string> a, vector<string> b);              // This is the longest common subsequence
    double calcPrecision(vector<string> ref, vector<string> cand);  // Precision metric
    double calcRecall(vector<string> ref, vector<string> cand);     // Recall metric
    double calcBleu(vector<string> ref, vector<string> cand);       // Simplified BLEU-4

    // METEOR with real word matching (exact + stem/synonym) and a fragmentation penalty
    double calcMeteor(vector<string> ref, vector<string> cand);
    int countMeteorChunks(vector<int> candMatchRefIndex);        // This counts matched-word chunks

    double calcRougeL(vector<string> ref, vector<string> cand);  // ROUGE-L
    double calcCider(vector<string> ref, vector<string> cand);   // CIDER without corpus (raw count version)

    /* CIDER with corpus TF-IDF weights - needs CorpusStats, which is built by buildCorpusStats.
NOTE ABOUT & HERE: everywhere else in the program I pass parameters by value (copying), but CorpusStats can contain hundreds of thousands of entries (all the n-grams of the 
entire Flickr8k corpus). In the previous versions I also passed by value here, and running the full experiment became absurdly slow (~170ms per pair, just because the entire huge
dictionary was copied every time). With & (reference) here, nothing gets copied, only the address is passed - that solved the problem. 
This is the only exception in the entire program where I deliberately use a reference. */
    double calcCiderCorpus(vector<string> ref, vector<string> cand, const CorpusStats& corpusStats);
    double cosineSimilarity(map<string, double> vecA, map<string, double> vecB);    // This is the shared cosine similarity function

    string makeLevel(double score);     // This converts the score into a category (in English)

public:
    MetricResult evaluate(string referenceText, string candidateText);                                  // This one compares without corpus statistics
    MetricResult evaluate(string referenceText, string candidateText, const CorpusStats& corpusStats);  // This one compares with corpus CIDER (& the same as calcCiderCorpus - see the note there)
    MetricResult evaluateMulti(vector<string> references, string candidate);                            // This one compares a candidate against multiple reference values

    /* Here I add the semantic similarity to the result and recalculate the final score according to the given weight profile
    This is the only place in the entire program where the final weighted score is computed */
    MetricResult attachSemantic(MetricResult result, double semanticScore, WeightProfile profile);

    // This one builds corpus statistics (n-gram document frequency) from a large list of reference descriptions
    CorpusStats buildCorpusStats(vector<string> referenceTexts);
};

#endif

