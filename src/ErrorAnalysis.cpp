// This includes the ErrorAnalysis class declaration, screen output, file writing, text stream for report building, and formatting functions
#include "ErrorAnalysis.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

// The function counts how many pairs fall into each of the 5 level categories, in fixed order: insufficient, weak, average, good, excellent
vector<int> ErrorAnalysis::countLevelBuckets(vector<ScoredPair> pairs) {

    vector<int> counts(5, 0);

    // This loop compares each pair's category against all known ones and increments the matching counter
    for (int i = 0; i < (int)pairs.size(); i++) {
        string level = pairs[i].metrics.level;

        if (level == "insufficient") {
            counts[0]++;
        }
        else if (level == "weak") {
            counts[1]++;
        }
        else if (level == "average") {
            counts[2]++;
        }
        else if (level == "good") {
            counts[3]++;
        }
        else if (level == "excellent") {
            counts[4]++;
        }
    }

    return counts;
}

// This function formats the entire detailed report as text
string ErrorAnalysis::formatReport(vector<ScoredPair> pairs, vector<ScoredPair> worstPairs, vector<ScoredPair> bestPairs, int topN) {

    ostringstream out;

    // Here I collect the values of each metric into separate lists, so that statistics can be calculated
    vector<double> precisionValues;
    vector<double> recallValues;
    vector<double> f1Values;
    vector<double> bleuValues;
    vector<double> meteorValues;
    vector<double> rougeLValues;
    vector<double> ciderValues;
    vector<double> semanticValues;
    vector<double> finalScoreValues;

    for (int i = 0; i < (int)pairs.size(); i++) {
        precisionValues.push_back(pairs[i].metrics.precision);
        recallValues.push_back(pairs[i].metrics.recall);
        f1Values.push_back(pairs[i].metrics.f1);
        bleuValues.push_back(pairs[i].metrics.bleu);
        meteorValues.push_back(pairs[i].metrics.meteor);
        rougeLValues.push_back(pairs[i].metrics.rougeL);
        ciderValues.push_back(pairs[i].metrics.cider);
        semanticValues.push_back(pairs[i].metrics.semantic);
        finalScoreValues.push_back(pairs[i].metrics.finalScore);
    }

    // Here is the statistics and correlation with final_score for each metric separately
    MetricStats precisionStats = statsUtil.calcStats(precisionValues);
    MetricStats recallStats = statsUtil.calcStats(recallValues);
    MetricStats f1Stats = statsUtil.calcStats(f1Values);
    MetricStats bleuStats = statsUtil.calcStats(bleuValues);
    MetricStats meteorStats = statsUtil.calcStats(meteorValues);
    MetricStats rougeLStats = statsUtil.calcStats(rougeLValues);
    MetricStats ciderStats = statsUtil.calcStats(ciderValues);
    MetricStats semanticStats = statsUtil.calcStats(semanticValues);
    MetricStats finalScoreStats = statsUtil.calcStats(finalScoreValues);

    double corrPrecision = statsUtil.pearsonCorrelation(finalScoreValues, precisionValues);
    double corrRecall = statsUtil.pearsonCorrelation(finalScoreValues, recallValues);
    double corrF1 = statsUtil.pearsonCorrelation(finalScoreValues, f1Values);
    double corrBleu = statsUtil.pearsonCorrelation(finalScoreValues, bleuValues);
    double corrMeteor = statsUtil.pearsonCorrelation(finalScoreValues, meteorValues);
    double corrRougeL = statsUtil.pearsonCorrelation(finalScoreValues, rougeLValues);
    double corrCider = statsUtil.pearsonCorrelation(finalScoreValues, ciderValues);
    double corrSemantic = statsUtil.pearsonCorrelation(finalScoreValues, semanticValues);

    vector<int> levelCounts = countLevelBuckets(pairs);

    out << "=== Flickr8k error analysis (error analysis) ===" << endl;
    out << endl;
    out << "Number of processed pairs: " << pairs.size() << endl;
    out << endl;

    // This is the statistics table by metric
    out << "--- Statistics per metric (mean / median / std.deviation / min / max) ---" << endl;
    out << fixed << setprecision(4);
    out << "precision  : " << precisionStats.mean << " / " << precisionStats.median << " / " << precisionStats.stdDev << " / " << precisionStats.minVal << " / " << precisionStats.maxVal << endl;
    out << "recall     : " << recallStats.mean << " / " << recallStats.median << " / " << recallStats.stdDev << " / " << recallStats.minVal << " / " << recallStats.maxVal << endl;
    out << "f1         : " << f1Stats.mean << " / " << f1Stats.median << " / " << f1Stats.stdDev << " / " << f1Stats.minVal << " / " << f1Stats.maxVal << endl;
    out << "bleu       : " << bleuStats.mean << " / " << bleuStats.median << " / " << bleuStats.stdDev << " / " << bleuStats.minVal << " / " << bleuStats.maxVal << endl;
    out << "meteor     : " << meteorStats.mean << " / " << meteorStats.median << " / " << meteorStats.stdDev << " / " << meteorStats.minVal << " / " << meteorStats.maxVal << endl;
    out << "rouge_l    : " << rougeLStats.mean << " / " << rougeLStats.median << " / " << rougeLStats.stdDev << " / " << rougeLStats.minVal << " / " << rougeLStats.maxVal << endl;
    out << "cider      : " << ciderStats.mean << " / " << ciderStats.median << " / " << ciderStats.stdDev << " / " << ciderStats.minVal << " / " << ciderStats.maxVal << endl;
    out << "semantic   : " << semanticStats.mean << " / " << semanticStats.median << " / " << semanticStats.stdDev << " / " << semanticStats.minVal << " / " << semanticStats.maxVal << endl;
    out << setprecision(2);
    out << "final_score: " << finalScoreStats.mean << " / " << finalScoreStats.median << " / " << finalScoreStats.stdDev << " / " << finalScoreStats.minVal << " / " << finalScoreStats.maxVal << endl;
    out << endl;

    // Level distribution
    out << "--- Level distribution ---" << endl;
    out << "insufficient: " << levelCounts[0] << endl;
    out << "weak        : " << levelCounts[1] << endl;
    out << "average     : " << levelCounts[2] << endl;
    out << "good        : " << levelCounts[3] << endl;
    out << "excellent   : " << levelCounts[4] << endl;
    out << endl;

    // Correlation with final_score
    out << "--- Correlation between final_score and each metric (Pearson coefficient, from -1 to 1) ---" << endl;
    out << setprecision(3);
    out << "precision vs final_score: " << corrPrecision << endl;
    out << "recall    vs final_score: " << corrRecall << endl;
    out << "f1        vs final_score: " << corrF1 << endl;
    out << "bleu      vs final_score: " << corrBleu << endl;
    out << "meteor    vs final_score: " << corrMeteor << endl;
    out << "rouge_l   vs final_score: " << corrRougeL << endl;
    out << "cider     vs final_score: " << corrCider << endl;
    out << "semantic  vs final_score: " << corrSemantic << endl;
    out << endl;

    // Worst pairs - useful for understanding where the metrics or Florence "fail"
    out << "--- " << topN << " worst pairs (lowest final_score) ---" << endl;

    // worstPairs is already sorted in ascending order, so the first one is the worst
    for (int i = 0; i < (int)worstPairs.size(); i++) {
        out << (i + 1) << ". image: " << worstPairs[i].imageName << ", final_score: " << fixed << setprecision(2) << worstPairs[i].metrics.finalScore << endl;
        out << "   reference: " << worstPairs[i].referenceText << endl;
        out << "   candidate: " << worstPairs[i].candidateText << endl;
    }
    out << endl;

    // Best pairs - useful as "success stories" for comparison
    out << "--- " << topN << " best pairs (highest final_score) ---" << endl;

    // bestPairs is also sorted in ascending order, so the best one is last - I go from the end to the start
    int rank = 1;
    for (int i = (int)bestPairs.size() - 1; i >= 0; i--) {
        out << rank << ". image: " << bestPairs[i].imageName << ", final_score: " << fixed << setprecision(2) << bestPairs[i].metrics.finalScore << endl;
        out << "   reference: " << bestPairs[i].referenceText << endl;
        out << "   candidate: " << bestPairs[i].candidateText << endl;
        rank++;
    }

    return out.str();
}

// This function performs a full error analysis and writes the result to the specified file
void ErrorAnalysis::runAnalysis(vector<ScoredPair> pairs, string outputPath, int topN) {

    // If there are no pairs, there is nothing to analyze
    if (pairs.size() == 0) {
        cout << "There are no pairs for error analysis, this step is skipped." << endl;
        return;
    }

    /* The worst and best pair lists are kept sorted in ascending order by final_score, with no more than topN elements,
    using bounded insertion instead of fully sorting the entire list of 32000 pairs (which would be slow for large amounts of data). */
    vector<ScoredPair> worstPairs;
    vector<ScoredPair> bestPairs;

    vector<double> finalScoreValues;  // This is for a quick average final_score calculation for console output

    for (int i = 0; i < (int)pairs.size(); i++) {
        double score = pairs[i].metrics.finalScore;
        finalScoreValues.push_back(score);

        // Maintaining the worst list (topN lowest scores, sorted ascending)

        // If the worst list is not yet full, I simply add the pair and shift it into the correct place
        if ((int)worstPairs.size() < topN) {
            worstPairs.push_back(pairs[i]);

            int j = (int)worstPairs.size() - 1;
            while (j > 0 && worstPairs[j - 1].metrics.finalScore > worstPairs[j].metrics.finalScore) {
                ScoredPair temp = worstPairs[j - 1];
                worstPairs[j - 1] = worstPairs[j];
                worstPairs[j] = temp;
                j--;
            }
        }

        /* If the list is already full, I only replace it when this pair is worse than the currently
        stored "worst of the worst" (the last one in the list, since it is sorted ascending) */
        else if (worstPairs.size() > 0 && score < worstPairs[worstPairs.size() - 1].metrics.finalScore) {
            worstPairs[worstPairs.size() - 1] = pairs[i];

            int j = (int)worstPairs.size() - 1;
            while (j > 0 && worstPairs[j - 1].metrics.finalScore > worstPairs[j].metrics.finalScore) {
                ScoredPair temp = worstPairs[j - 1];
                worstPairs[j - 1] = worstPairs[j];
                worstPairs[j] = temp;
                j--;
            }
        }

        // Maintaining the best list (topN highest scores, sorted ascending)

        if ((int)bestPairs.size() < topN) {
            bestPairs.push_back(pairs[i]);

            int j = (int)bestPairs.size() - 1;
            while (j > 0 && bestPairs[j - 1].metrics.finalScore > bestPairs[j].metrics.finalScore) {
                ScoredPair temp = bestPairs[j - 1];
                bestPairs[j - 1] = bestPairs[j];
                bestPairs[j] = temp;
                j--;
            }
        }
        /* If the list is already full, I only replace it when this pair is better than the currently
        stored "worst of the best" (the first one in the list, since it is sorted ascending) */
        else if (bestPairs.size() > 0 && score > bestPairs[0].metrics.finalScore) {
            bestPairs[0] = pairs[i];

            int j = 0;
            while (j < (int)bestPairs.size() - 1 && bestPairs[j].metrics.finalScore > bestPairs[j + 1].metrics.finalScore) {
                ScoredPair temp = bestPairs[j];
                bestPairs[j] = bestPairs[j + 1];
                bestPairs[j + 1] = temp;
                j++;
            }
        }
    }

    string report = formatReport(pairs, worstPairs, bestPairs, topN);

    ofstream outFile(outputPath);
    outFile << report;
    outFile.close();

    double averageScore = statsUtil.calcMean(finalScoreValues);

    cout << endl;
    cout << "Error analysis completed." << endl;
    cout << "Number of processed pairs: " << pairs.size() << endl;
    cout << "Average final_score: " << fixed << setprecision(2) << averageScore << " / 100" << endl;
    cout << "Detailed report saved: " << outputPath << endl;
}
