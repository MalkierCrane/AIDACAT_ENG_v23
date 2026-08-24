/* This includes the Metrics class declaration, weight profiles (so that the default profile can be obtained in the evaluate() function),
math functions and algorithm functions. */
#include "Metrics.h"
#include "Coefficients.h"
#include <cmath>
#include <algorithm>

using namespace std;

// This function safely divides two numbers
double Metrics::safeDivide(double a, double b) {
    if (b == 0) {
        return 0;
    }

    return a / b;
}

// This function counts words
map<string, int> Metrics::countWords(vector<string> words) {

    map<string, int> result;

    for (int i = 0; i < (int)words.size(); i++) {
        result[words[i]]++;
    }

    return result;
}

// This function counts n-grams
map<string, int> Metrics::countNgrams(vector<string> words, int n) {

    map<string, int> result;

    if ((int)words.size() < n) {
        return result;
    }

    // This loop builds each n-gram by joining n consecutive words together with a "_" separator
    for (int i = 0; i <= (int)words.size() - n; i++) {
        string key = "";

        for (int j = 0; j < n; j++) {
            if (j > 0) {
                key += "_";
            }
            key += words[i + j];
        }

        result[key]++;
    }

    return result;
}

// This function counts the matches between two dictionaries
int Metrics::countOverlap(map<string, int> a, map<string, int> b) {

    int count = 0;

    // This loop goes through the first dictionary and adds the smaller of the two dictionaries' counts for each common key
    for (auto it = a.begin(); it != a.end(); it++) {
        string key = it->first;

        if (b.find(key) != b.end()) {
            count += min(it->second, b[key]);
        }
    }

    return count;
}

// This function calculates the longest common subsequence (LCS) of words using the standard dynamic programming algorithm
int Metrics::lcsLength(vector<string> a, vector<string> b) {

    vector<vector<int>> dp(a.size() + 1, vector<int>(b.size() + 1, 0));

    for (int i = 1; i <= (int)a.size(); i++) {
        for (int j = 1; j <= (int)b.size(); j++) {
            if (a[i - 1] == b[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
            }
            else {
                dp[i][j] = max(dp[i - 1][j], dp[i][j - 1]);
            }
        }
    }

    return dp[a.size()][b.size()];
}

// This function calculates precision
double Metrics::calcPrecision(vector<string> ref, vector<string> cand) {

    map<string, int> refMap = countWords(ref);
    map<string, int> candMap = countWords(cand);
    int overlap = countOverlap(candMap, refMap);

    return safeDivide(overlap, (double)cand.size());
}

// This function calculates recall
double Metrics::calcRecall(vector<string> ref, vector<string> cand) {

    map<string, int> refMap = countWords(ref);
    map<string, int> candMap = countWords(cand);
    int overlap = countOverlap(refMap, candMap);

    return safeDivide(overlap, (double)ref.size());
}

// This function calculates a simplified BLEU-4
double Metrics::calcBleu(vector<string> ref, vector<string> cand) {
    if (cand.size() == 0) {
        return 0;
    }

    double logSum = 0;

    // This loop calculates precision for 1- to 4-grams and sums their logarithms for the geometric mean
    for (int n = 1; n <= 4; n++) {
        map<string, int> refN = countNgrams(ref, n);
        map<string, int> candN = countNgrams(cand, n);

        int total = 0;
        for (auto it = candN.begin(); it != candN.end(); it++) {
            total += it->second;
        }

        int overlap = countOverlap(candN, refN);

        // Here I apply smoothing (+1/+1) so that short texts don't get an immediate zero
        double p = safeDivide((double)overlap + 1.0, (double)total + 1.0);
        logSum += log(p);
    }

    double geoMean = exp(logSum / 4.0);

    // Brevity penalty - a penalty if the candidate is shorter than the reference text
    double bp = 1.0;
    if (cand.size() < ref.size()) {
        bp = exp(1.0 - safeDivide((double)ref.size(), (double)cand.size()));
    }

    return bp * geoMean;
}

/* This function counts the number of matched-word chunks for the METEOR fragmentation penalty. candMatchRefIndex[i] tells which reference word index the i-th candidate word 
is matched to, or -1 if it is not matched at all. A chunk is a sequence of consecutively matched words, where the reference
indices also proceed sequentially one by one forward - this means that the candidate and reference "say the same thing in the same order" at this point.
For the sake of simplicity, I consider that an unmatched candidate word always breaks the previous chunk. */
int Metrics::countMeteorChunks(vector<int> candMatchRefIndex) {

    int chunkCount = 0;
    int previousRefIndex = -2;  // -2 never matches a real index

    // This loop goes through all candidate words in order and counts where a new chunk starts
    for (int i = 0; i < (int)candMatchRefIndex.size(); i++) {
        int refIndex = candMatchRefIndex[i];

        // If the word is not matched, it breaks the current chunk and does not start a new one
        if (refIndex == -1) {
            previousRefIndex = -2;
            continue;
        }

        // If this match does not continue the previous one sequentially, a new chunk starts
        if (refIndex != previousRefIndex + 1) {
            chunkCount++;
        }

        previousRefIndex = refIndex;
    }

    return chunkCount;
}

/* This function calculates METEOR with real word matching in two stages and a fragmentation penalty. In the previous versions of the program, this was just
the formula 10pr/(r+9p) on plain bag-of-words precision/recall, without real matching - now I first match words (exact, then stem/synonym), and
only then compute the formula from the actual number of matched words. */
double Metrics::calcMeteor(vector<string> ref, vector<string> cand) {
    if (ref.size() == 0 || cand.size() == 0) {
        return 0;
    }

    vector<int> candMatchRefIndex((int)cand.size(), -1);  // Here, for each candidate word - the index of the matched reference word
    vector<bool> refUsed((int)ref.size(), false);         // This marks whether the reference word has already been used in matching

    // Stage 1: exact word matching (exact match), words are already normalized in the tokenize step
    for (int i = 0; i < (int)cand.size(); i++) {
        for (int j = 0; j < (int)ref.size(); j++) {     // Here I search for the first not-yet-used reference word that matches exactly
            if (refUsed[j]) {
                continue;
            }

            if (cand[i] == ref[j]) {
                candMatchRefIndex[i] = j;
                refUsed[j] = true;
                break;
            }
        }
    }

    /* Stage 2: stem/synonym matching for unused words, using the same TextProc::normalizeWord dictionary that tokenize already uses.
This replaces a real WordNet synonym lookup with my small, already-existing synonym table. */
    for (int i = 0; i < (int)cand.size(); i++) {
        if (candMatchRefIndex[i] != -1) {
            continue;
        }

        string candStem = textProc.normalizeWord(cand[i]);

        // Here I search for a not-yet-used reference word with the same normalized form
        for (int j = 0; j < (int)ref.size(); j++) {
            if (refUsed[j]) {
                continue;
            }

            string refStem = textProc.normalizeWord(ref[j]);

            if (candStem == refStem) {
                candMatchRefIndex[i] = j;
                refUsed[j] = true;
                break;
            }
        }
    }

    // Here I count how many candidate words are matched in total
    int matchCount = 0;
    for (int i = 0; i < (int)candMatchRefIndex.size(); i++) {
        if (candMatchRefIndex[i] != -1) {
            matchCount++;
        }
    }

    if (matchCount == 0) {
        return 0;
    }

    // Precision and recall are based on the actual number of matched words, not bag-of-words overlap
    double p = safeDivide((double)matchCount, (double)cand.size());
    double r = safeDivide((double)matchCount, (double)ref.size());

    /* The F-mean formula gives more weight to the recall side - this formula itself was already correct in the previous version,
the problem was only that p and r did not come from real matching.*/
    double fMean = safeDivide(10.0 * p * r, r + 9.0 * p);

    int chunkCount = countMeteorChunks(candMatchRefIndex);

    /* Fragmentation ratio - the more separate short chunks there are compared to the total number matched,
the more the candidate "reorders" words compared to the reference order. */
    double fragmentation = safeDivide((double)chunkCount, (double)matchCount);
    double penalty = 0.5 * fragmentation * fragmentation * fragmentation;  // This is the standard METEOR fragmentation penalty

    double score = fMean * (1.0 - penalty);
    if (score < 0.0) {
        score = 0.0;
    }

    return score;
}

// This function calculates ROUGE-L
double Metrics::calcRougeL(vector<string> ref, vector<string> cand) {
    if (ref.size() == 0 || cand.size() == 0) {
        return 0;
    }

    int lcs = lcsLength(ref, cand);
    double p = safeDivide((double)lcs, (double)cand.size());
    double r = safeDivide((double)lcs, (double)ref.size());

    if (p == 0 && r == 0) {
        return 0;
    }

    return safeDivide(2.0 * p * r, p + r);
}

/* This function calculates the cosine similarity between two weighted vectors. It is used together by both the old, raw-count CIDER version, and the new,
corpus TF-IDF weighted version, so that the math is written in only one place. */
double Metrics::cosineSimilarity(map<string, double> vecA, map<string, double> vecB) {

    double dot = 0.0;
    double lenA = 0.0;
    double lenB = 0.0;

    for (auto it = vecA.begin(); it != vecA.end(); it++) {
        lenA += it->second * it->second;
    }

    // This loop accumulates the length of the second vector and, where the key matches the first vector, also the dot product
    for (auto it = vecB.begin(); it != vecB.end(); it++) {
        lenB += it->second * it->second;

        if (vecA.find(it->first) != vecA.end()) {
            dot += it->second * vecA[it->first];
        }
    }

    return safeDivide(dot, sqrt(lenA) * sqrt(lenB));
}

/* This function calculates CIDER without corpus statistics - the raw n-gram count version. This is used only when the corpus file (Flickr8k.token.txt) cannot
be found and the Evaluation class cannot build CorpusStats - see calcCiderCorpus for the real, TF-IDF weighted version. */
double Metrics::calcCider(vector<string> ref, vector<string> cand) {

    double sum = 0;
    int used = 0;

    // This loop goes through 1- to 4-grams and sums the cosine similarity for each n-gram length
    for (int n = 1; n <= 4; n++) {
        map<string, int> refN = countNgrams(ref, n);
        map<string, int> candN = countNgrams(cand, n);

        if (refN.size() == 0 || candN.size() == 0) {
            continue;
        }

        // Here I convert the count dictionaries into double vectors, so they can be passed to cosineSimilarity
        map<string, double> refVec;
        for (auto it = refN.begin(); it != refN.end(); it++) {
            refVec[it->first] = (double)it->second;
        }

        map<string, double> candVec;
        for (auto it = candN.begin(); it != candN.end(); it++) {
            candVec[it->first] = (double)it->second;
        }

        double cosine = cosineSimilarity(refVec, candVec);
        sum += cosine;
        used++;
    }

    return safeDivide(sum, (double)used);
}

/* This function calculates CIDER with corpus TF-IDF weights. For each n-gram I assign a weight based on its rarity in the corpus (IDF) - the rarer the n-gram is
in the corpus (for example, across all Flickr8k descriptions), the more it "says" about the specific image, and the larger the weight it gets.
Frequently occurring n-grams (for example "a man") get a small weight. */
double Metrics::calcCiderCorpus(vector<string> ref, vector<string> cand, const CorpusStats& corpusStats) {

    double sum = 0.0;
    int used = 0;

    for (int n = 1; n <= 4; n++) {
        map<string, int> refN = countNgrams(ref, n);
        map<string, int> candN = countNgrams(cand, n);

        if (refN.size() == 0 || candN.size() == 0) {
            continue;
        }

        // Here I prepare the TF-IDF weighted reference vector
        map<string, double> refVec;
        for (auto it = refN.begin(); it != refN.end(); it++) {
            int df = 0;

            /* Here I look up the n-gram in the corpus statistics ONCE and keep the found location (iterator).
NOTE: originally I had also written a second lookup here with the [] operator, but that did not compile, because corpusStats here is const&
(see the note at the function declaration in the Metrics.h file) - the [] operator would also be allowed to INSERT a new entry into the dictionary if such a key does not yet exist,
so it is not allowed for a const dictionary. By using the found iterator directly, I avoid a second, unnecessary lookup. */
            auto foundEntry = corpusStats.documentFrequency.find(it->first);
            if (foundEntry != corpusStats.documentFrequency.end()) {
                df = foundEntry->second;
            }

            // IDF - the rarer the n-gram is in the corpus, the higher the IDF. TF here is simply the count of the n-gram in this text
            double idf = log((double)corpusStats.documentCount / (1.0 + (double)df));
            refVec[it->first] = (double)it->second * idf;
        }

        // Here I prepare the TF-IDF weighted candidate vector the same way as the reference vector above
        map<string, double> candVec;
        for (auto it = candN.begin(); it != candN.end(); it++) {
            int df = 0;

            // Here I look up the n-gram ONCE (see the longer note at the reference vector above)
            auto foundEntry = corpusStats.documentFrequency.find(it->first);
            if (foundEntry != corpusStats.documentFrequency.end()) {
                df = foundEntry->second;
            }

            double idf = log((double)corpusStats.documentCount / (1.0 + (double)df));
            candVec[it->first] = (double)it->second * idf;
        }

        double cosine = cosineSimilarity(refVec, candVec);
        sum += cosine;
        used++;
    }

    return safeDivide(sum, (double)used);
}

/* This function builds corpus statistics from a large list of reference descriptions. This is called once by the Evaluation class, when loading the entire Flickr8k.token.txt file
as the corpus, and afterwards reuses the same CorpusStats for all calcCiderCorpus calls. */
CorpusStats Metrics::buildCorpusStats(vector<string> referenceTexts) {

    CorpusStats stats;
    stats.documentCount = 0;

    // This loop tokenizes each reference text (one text = one "document" in the TF-IDF sense) and increments the document frequency of each n-gram found in it
    for (int i = 0; i < (int)referenceTexts.size(); i++) {
        vector<string> tokens = textProc.tokenize(referenceTexts[i]);

        if (tokens.size() == 0) {
            continue;
        }

        stats.documentCount++;

        for (int n = 1; n <= 4; n++) {
            map<string, int> ngrams = countNgrams(tokens, n);  // Only which n-grams appear is needed, not how many times

            for (auto it = ngrams.begin(); it != ngrams.end(); it++) {
                stats.documentFrequency[it->first]++;
            }
        }
    }

    return stats;
}

// This function converts a score into a category. The categories are in English.
string Metrics::makeLevel(double score) {
    if (score < 25) {
        return "insufficient";
    }
    if (score < 50) {
        return "weak";
    }
    if (score < 70) {
        return "average";
    }
    if (score < 85) {
        return "good";
    }

    return "excellent";
}

/* This function adds the semantic similarity to the result and recalculates the final score according to the given weight profile. This is the only place in the entire 
program where the final weighted score is computed - both evaluate() (with semantic=0.0 and the default profile), and the Evaluation class (with a real semantic score and
the correct profile based on the image's purpose) call this function directly. */
MetricResult Metrics::attachSemantic(MetricResult result, double semanticScore, WeightProfile profile) {

    // If the semantic score is negative (for example, a -1.0 error signal from SemanticSimilarity), then I replace it with 0
    if (semanticScore < 0.0) {
        semanticScore = 0.0;
    }

    result.semantic = semanticScore;

    result.finalScore = 100.0 * (
        profile.wPrecision * result.precision +
        profile.wRecall * result.recall +
        profile.wF1 * result.f1 +
        profile.wBleu * result.bleu +
        profile.wMeteor * result.meteor +
        profile.wRougeL * result.rougeL +
        profile.wCider * result.cider +
        profile.wSemantic * result.semantic
    );

    result.level = makeLevel(result.finalScore);

    return result;
}

// This function compares one reference with one candidate, without corpus statistics for the CIDER metric
MetricResult Metrics::evaluate(string referenceText, string candidateText) {

    MetricResult result;

    vector<string> ref = textProc.tokenize(referenceText);
    vector<string> cand = textProc.tokenize(candidateText);

    result.precision = calcPrecision(ref, cand);
    result.recall = calcRecall(ref, cand);
    result.f1 = safeDivide(2.0 * result.precision * result.recall, result.precision + result.recall);
    result.bleu = calcBleu(ref, cand);
    result.meteor = calcMeteor(ref, cand);
    result.rougeL = calcRougeL(ref, cand);
    result.cider = calcCider(ref, cand);
    result.purpose = "n/a";  // The image's purpose is not known for this function

    /* I calculate the final score here without semantic similarity and with the default weight profile - the Evaluation class usually
    calls attachSemantic again itself, with a real semantic score and the correct profile. */
    Coefficients coefficients;
    result = attachSemantic(result, 0.0, coefficients.getDefaultProfile());

    return result;
}

// This function compares one reference with one candidate, using corpus TF-IDF statistics for the CIDER metric
MetricResult Metrics::evaluate(string referenceText, string candidateText, const CorpusStats& corpusStats) {

    MetricResult result;

    vector<string> ref = textProc.tokenize(referenceText);
    vector<string> cand = textProc.tokenize(candidateText);

    result.precision = calcPrecision(ref, cand);
    result.recall = calcRecall(ref, cand);
    result.f1 = safeDivide(2.0 * result.precision * result.recall, result.precision + result.recall);
    result.bleu = calcBleu(ref, cand);
    result.meteor = calcMeteor(ref, cand);
    result.rougeL = calcRougeL(ref, cand);
    result.cider = calcCiderCorpus(ref, cand, corpusStats);
    result.purpose = "n/a";

    Coefficients coefficients;
    result = attachSemantic(result, 0.0, coefficients.getDefaultProfile());

    return result;
}

// This function compares a candidate against multiple reference values
MetricResult Metrics::evaluateMulti(vector<string> references, string candidate) {

    MetricResult best;
    best.finalScore = -1;

    if (references.size() == 0) {
        return evaluate("", candidate);
    }

    // This loop compares the candidate with each reference text and keeps the best result
    for (int i = 0; i < (int)references.size(); i++) {
        MetricResult current = evaluate(references[i], candidate);

        if (current.finalScore > best.finalScore) {
            best = current;
        }
    }

    return best;
}

