/* This includes the Calibration class declaration, the weight profile class, file reading and writing, text stream for splitting lines and building the report,
cstdlib atof/atoi functions, the dictionary for the image index, screen output, and formatting functions. */
#include "Calibration.h"
#include "Coefficients.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <map>
#include <iostream>
#include <iomanip>

using namespace std;

/* This function loads the ExpertAnnotations.txt file. The format of each row is:
judgedImage.jpg <TAB> candidateImage.jpg#N <TAB> rater1 <TAB> rater2 <TAB> rater3, where the raters are numbers from 1 to 4.
Then I convert the average to a 0-100 scale, so it can be compared with finalScore. */
LoadedAnnotationRows Calibration::loadExpertAnnotations(string filePath, int limit) {

    // Preparing the result
    LoadedAnnotationRows result;
    result.skippedCount = 0;

    ifstream file(filePath);    // Opening the file for reading

    // If the file cannot be opened, return an empty result
    if (!file.is_open()) {
        return result;
    }

    string line;    // Here I prepare the text of one line

    // This loop reads the file line by line
    while (getline(file, line)) {
        if (limit > 0 && (int)result.rows.size() >= limit) {    // If I have reached the limit on the number of valid rows, stop.
            break;  // Exit the loop
        }

        // If the row is empty, skip it (not considered a malformed row)
        if (line.size() == 0) {
            continue;   // Then continue with the next row
        }

        // Here I split the row by tabs
        vector<string> parts;
        stringstream ss(line);
        string part;

        // This loop reads each part between tabs
        while (getline(ss, part, '\t')) {
            parts.push_back(part);  // Here I add the part to the list
        }

        // If there are fewer than 5 parts, the row is malformed - I skip it and count it
        if (parts.size() < 5) {
            result.skippedCount++;      // Then increment the skipped-row counter
            continue;                   // And continue with the next row
        }

        string judgedImage = parts[0];                  // The first part is the name of the image being judged
        string candidateField = parts[1];               // The second part is the candidate image and index, for example "image.jpg#2"
        size_t hashPos = candidateField.find('#');      // Here I look for the # character

        // If # is not found, the row is malformed
        if (hashPos == string::npos) {
            result.skippedCount++;      // Then I increment the skipped-row counter
            continue;                   // And continue with the next row
        }

        string candidateImage = candidateField.substr(0, hashPos);      // Here I cut off the candidate image name
        string indexText = candidateField.substr(hashPos + 1);          // Then I read the index text after #
        int candidateIndex = atoi(indexText.c_str());                   // Then I convert the index into a number

        // Here I convert the three raters' numbers into double-type numbers
        double expert1 = atof(parts[2].c_str());
        double expert2 = atof(parts[3].c_str());
        double expert3 = atof(parts[4].c_str());

        double averageRaw = (expert1 + expert2 + expert3) / 3.0;    // Here I calculate the average of the three raters, scale 1 to 4

        // Here I convert the scale from 1-4 to 0-100, so it can be compared with finalScore
        double scaled = (averageRaw - 1.0) / 3.0 * 100.0;

        // Here the row structure is prepared, so it can be added to the result
        ExpertRow row;
        row.judgedImageName = judgedImage;
        row.candidateImageName = candidateImage;
        row.candidateIndex = candidateIndex;
        row.groundTruthScore = scaled;
        result.rows.push_back(row);
    }

    file.close();   // Closing the file

    return result;  // Returning the loaded rows
}

/* This function loads the CrowdFlowerAnnotations.txt file. The format of each row is:
judgedImage.jpg <TAB> candidateImage.jpg#N <TAB> percentYes <TAB> numYes <TAB> numNo, where percentYes is already a fraction from 0 to 1 - I convert it to a 0-100 scale. */
LoadedAnnotationRows Calibration::loadCrowdflowerAnnotations(string filePath, int limit) {

    // Preparing the result
    LoadedAnnotationRows result;
    result.skippedCount = 0;

    ifstream file(filePath);    // Opening the file for reading

    // If the file cannot be opened, return an empty result
    if (!file.is_open()) {
        return result;
    }

    string line;    // Here I prepare the text of one line

    // This loop reads the file line by line
    while (getline(file, line)) {
        if (limit > 0 && (int)result.rows.size() >= limit) {    // If I have reached the limit, stop
            break;
        }

        // If the row is empty, I skip it
        if (line.size() == 0) {
            continue;   // And continue with the next row
        }

        // Here I split the row by tabs
        vector<string> parts;
        stringstream ss(line);
        string part;

        // This loop reads each part
        while (getline(ss, part, '\t')) {
            parts.push_back(part);  // Here I add the part to the list
        }

        // If there are fewer than 3 parts (image, candidate, percentYes), the row is malformed
        if (parts.size() < 3) {
            result.skippedCount++;      // And need to increment the skipped-row counter
            continue;                   // And then continue with the next row
        }

        string judgedImage = parts[0];              // The first part is the name of the image being judged
        string candidateField = parts[1];           // The second part is the candidate image and index
        size_t hashPos = candidateField.find('#');  // Here I look for the # character

        // If # is not found, the row is malformed
        if (hashPos == string::npos) {
            result.skippedCount++;      // And need to increment the skipped-row counter
            continue;                   // And then continue with the next row
        }

        string candidateImage = candidateField.substr(0, hashPos);  // Here I cut off the candidate image name
        string indexText = candidateField.substr(hashPos + 1);      // Then I read the index text after #
        int candidateIndex = atoi(indexText.c_str());               // Then I convert the index into a number
        double percentYes = atof(parts[2].c_str());                 // Then I convert the percentYes fraction
        double scaled = percentYes * 100.0;                         // And convert to a 0-100 scale

        // Here the row structure is prepared, so it can be added to the result
        ExpertRow row;
        row.judgedImageName = judgedImage;
        row.candidateImageName = candidateImage;
        row.candidateIndex = candidateIndex;
        row.groundTruthScore = scaled;
        result.rows.push_back(row);
    }

    file.close();   // Closing the file

    return result;  // Returning the loaded rows
}

// This function converts a weight profile into a simple list of 8 numbers
vector<double> Calibration::profileToArray(WeightProfile profile) {

    vector<double> values;  // Preparing the list

    // Then I add each weight in a fixed order
    values.push_back(profile.wPrecision);
    values.push_back(profile.wRecall);
    values.push_back(profile.wF1);
    values.push_back(profile.wBleu);
    values.push_back(profile.wMeteor);
    values.push_back(profile.wRougeL);
    values.push_back(profile.wCider);
    values.push_back(profile.wSemantic);

    return values;  // And return the list
}

// This function converts a list of 8 numbers back into a weight profile
WeightProfile Calibration::arrayToProfile(vector<double> values, string name) {

    WeightProfile profile;  // Preparing the profile
    profile.name = name;    // Saving the name

    // Then I save each weight in the same order in which they were placed in profileToArray
    profile.wPrecision = values[0];
    profile.wRecall = values[1];
    profile.wF1 = values[2];
    profile.wBleu = values[3];
    profile.wMeteor = values[4];
    profile.wRougeL = values[5];
    profile.wCider = values[6];
    profile.wSemantic = values[7];

    return profile; // And return the profile
}

// This function makes the 8 weights sum to 1.0, by dividing each by the total sum
vector<double> Calibration::normalizeWeightArray(vector<double> values) {

    double sum = 0.0;   // Preparing the sum

    for (int i = 0; i < (int)values.size(); i++) {      // This loop adds up all the weights
        sum += values[i];                               // And adds the weight to the sum
    }

    // If the sum is 0 or negative (should not happen, but just in case), return unchanged
    if (sum <= 0.0) {
        return values;
    }

    vector<double> result;       // Here the normalized list is being prepared

    for (int i = 0; i < (int)values.size(); i++) {      // This loop divides each weight by the sum
        result.push_back(values[i] / sum);              // And adds the normalized value
    }

    return result;      // Returning the normalized list
}

/* This function recalculates final_score for all results with the given profile and returns the correlation with human ratings.
The semantic value for each result has already been calculated earlier (no need to call Python again) - only the weights with which it is combined with the other metrics change. */
double Calibration::evaluateProfileCorrelation(vector<MetricResult> ourResults, vector<double> groundTruthScores, WeightProfile profile) {

    Metrics metricsEngine;          // Here I create a local Metrics object only for calling attachSemantic
    vector<double> trialScores;     // Then I prepare a list with the recalculated final_score values

    for (int i = 0; i < (int)ourResults.size(); i++) {          // This loop recalculates each result with the new profile

        // Here I recalculate the final score, using the already known semantic value and the new profile
        MetricResult recomputed = metricsEngine.attachSemantic(ourResults[i], ourResults[i].semantic, profile);

        // And add the recalculated score to the list
        trialScores.push_back(recomputed.finalScore);
    }

        // And then return the correlation between the recalculated scores and the human ratings
    return statsUtil.pearsonCorrelation(trialScores, groundTruthScores);
}

/* This function searches for an improved weight profile using the coordinate ascent method. A full grid search across all 8 dimensions (for example, 5 values each) would be 5^8,
which is not practically feasible - so at each step only one weight's value is changed at a time, and the change is kept only if it improves the correlation with human ratings. */
WeightProfile Calibration::hillClimbSearch(vector<MetricResult> ourResults, vector<double> groundTruthScores, WeightProfile startProfile) {

    vector<double> currentWeights = profileToArray(startProfile);   // Here the starting profile is converted into a list of numbers

    // Then the starting correlation is calculated
    double bestCorrelation = evaluateProfileCorrelation(ourResults, groundTruthScores, arrayToProfile(currentWeights, "informative"));

    // Then I prepare the step size and the number of passes
    double step = 0.02;
    int passCount = 20;

    for (int pass = 0; pass < passCount; pass++) {                  // Here the outer loop goes through several passes over all the weights
        for (int fieldIndex = 0; fieldIndex < 8; fieldIndex++) {    // And the inner loop goes through each of the 8 weights in order

            // Here I try to increase this weight by one step
            vector<double> trialUp = currentWeights;
            trialUp[fieldIndex] = trialUp[fieldIndex] + step;
            trialUp = normalizeWeightArray(trialUp);

            // Then calculate the correlation with the increased weight
            double correlationUp = evaluateProfileCorrelation(ourResults, groundTruthScores, arrayToProfile(trialUp, "informative"));

            // If this improves the correlation, keep this change, save the new best correlation and the new weights
            if (correlationUp > bestCorrelation) {
                bestCorrelation = correlationUp;
                currentWeights = trialUp;
                continue;       // Then continue with the next weight, this one has already been improved
            }

            // Here I try to decrease this weight by one step, if it doesn't become negative
            if (currentWeights[fieldIndex] - step > 0.0) {

                // Here I prepare the decreased variant
                vector<double> trialDown = currentWeights;
                trialDown[fieldIndex] = trialDown[fieldIndex] - step;
                trialDown = normalizeWeightArray(trialDown);

                // Then I calculate the correlation with the decreased weight
                double correlationDown = evaluateProfileCorrelation(ourResults, groundTruthScores, arrayToProfile(trialDown, "informative"));

                // If this improves the correlation, keep this change, save the new best correlation and the new weights
                if (correlationDown > bestCorrelation) {
                    bestCorrelation = correlationDown;
                    currentWeights = trialDown;
                }
            }
        }
    }

    // Then return the best profile found
    return arrayToProfile(currentWeights, "informative");
}

// The function writes the final calibration report.
void Calibration::writeReport(string outputPath, string sourceLabel, int loadedCount, int skippedRowsCount, int skippedLookupCount,
                               vector<MetricResult> ourResults, vector<double> groundTruthScores,
                               WeightProfile startProfile, WeightProfile discoveredProfile) {

    ostringstream out;

    double correlationBefore = evaluateProfileCorrelation(ourResults, groundTruthScores, startProfile);
    double correlationAfter = evaluateProfileCorrelation(ourResults, groundTruthScores, discoveredProfile);

    out << "=== Coefficient calibration report (" << sourceLabel << ") ===" << endl;
    out << endl;
    out << "Number of loaded rows: " << loadedCount << endl;
    out << "Number of skipped rows (malformed format): " << skippedRowsCount << endl;
    out << "Number of skipped rows (image not found in Flickr8k data): " << skippedLookupCount << endl;
    out << endl;

    out << "--- Correlation BEFORE calibration (initial 'informative' profile) ---" << endl;
    out << fixed << setprecision(3);
    out << "final_score vs human rating: " << correlationBefore << endl;
    out << endl;

    out << "--- Correlation AFTER calibration (discovered profile) ---" << endl;
    out << "final_score vs human rating: " << correlationAfter << endl;
    out << endl;

    // The proposed weight profile is ready to copy into Coefficients.cpp.
    out << "--- Suggested 'informative' profile (can be copied into Coefficients.cpp) ---" << endl;
    out << setprecision(4);
    out << "precision : " << discoveredProfile.wPrecision << endl;
    out << "recall    : " << discoveredProfile.wRecall << endl;
    out << "f1        : " << discoveredProfile.wF1 << endl;
    out << "bleu      : " << discoveredProfile.wBleu << endl;
    out << "meteor    : " << discoveredProfile.wMeteor << endl;
    out << "rouge_l   : " << discoveredProfile.wRougeL << endl;
    out << "cider     : " << discoveredProfile.wCider << endl;
    out << "semantic  : " << discoveredProfile.wSemantic << endl;
    out << endl;

    out << "--- Note ---" << endl;
    out << "This process calibrates ONLY the 'informative' profile, because neither ExpertAnnotations.txt," << endl;
    out << "nor CrowdFlowerAnnotations.txt contain markings for image function (decorative/" << endl;
    out << "functional/complex). The other 3 profiles remain reasoned, but empirically unverified" << endl;
    out << "initial assumptions." << endl;

    ofstream outFile(outputPath);
    outFile << out.str();
    outFile.close();
}

// This function performs a full calibration cycle for one source file
void Calibration::runCalibration(string annotationFile, vector<FlickrItem> flickrItems, CorpusStats corpusStats,
                                  string outputPath, int limit, bool useCrowdflower,
                                  Metrics metricsEngine, SemanticSimilarity semanticSimilarity) {

    map<string, FlickrItem> flickrIndex;  // Image index for fast access by name

    for (int i = 0; i < (int)flickrItems.size(); i++) {
        flickrIndex[flickrItems[i].imageName] = flickrItems[i];
    }

    // I load the annotation rows - either from the Expert or the CrowdFlower file
    LoadedAnnotationRows loaded;
    if (useCrowdflower) {
        loaded = loadCrowdflowerAnnotations(annotationFile, limit);
    }
    else {
        loaded = loadExpertAnnotations(annotationFile, limit);
    }

    if (loaded.rows.size() == 0) {
        cout << "Could not load any valid row for calibration from: " << annotationFile << endl;
        return;
    }

    // I prepare a Coefficients object to get the starting "informative" profile
    Coefficients coefficients;
    WeightProfile startProfile = coefficients.getDefaultProfile();

    vector<MetricResult> ourResults;
    vector<double> groundTruthScores;
    int skippedLookupCount = 0;  // Rows for which the corresponding images could not be found in the data

    // This loop goes through all the loaded annotation rows
    for (int i = 0; i < (int)loaded.rows.size(); i++) {
        ExpertRow row = loaded.rows[i];

        // I check whether the judged image and the candidate image can be found in the Flickr8k data
        if (flickrIndex.find(row.judgedImageName) == flickrIndex.end()) {
            skippedLookupCount++;
            continue;
        }
        if (flickrIndex.find(row.candidateImageName) == flickrIndex.end()) {
            skippedLookupCount++;
            continue;
        }

        FlickrItem judgedItem = flickrIndex[row.judgedImageName];
        FlickrItem candidateItem = flickrIndex[row.candidateImageName];

        // If the judged image has no captions at all, it cannot be used as reference text
        if (judgedItem.captions.size() == 0) {
            skippedLookupCount++;
            continue;
        }

        // I check whether the candidate index is within a valid range
        if (row.candidateIndex < 0 || row.candidateIndex >= (int)candidateItem.captions.size()) {
            skippedLookupCount++;
            continue;
        }

        /* As the reference text, I use the first human caption of the judged image - the same
        principle already used by experiment mode for Flickr8k validation */
        string referenceText = judgedItem.captions[0];
        string candidateText = candidateItem.captions[row.candidateIndex];  // The caption that the experts evaluated

        MetricResult result = metricsEngine.evaluate(referenceText, candidateText, corpusStats);
        double semanticScore = semanticSimilarity.computeSimilarity(referenceText, candidateText);

        // The semantic similarity is attached with the starting profile - it is recalculated later by hillClimbSearch
        result = metricsEngine.attachSemantic(result, semanticScore, startProfile);

        ourResults.push_back(result);
        groundTruthScores.push_back(row.groundTruthScore);
    }

    if (ourResults.size() == 0) {
        cout << "Could not find any valid pair for calibration in the Flickr8k data." << endl;
        return;
    }

    WeightProfile discoveredProfile = hillClimbSearch(ourResults, groundTruthScores, startProfile);
    string sourceLabel = useCrowdflower ? "CrowdFlowerAnnotations.txt" : "ExpertAnnotations.txt";

    writeReport(outputPath, sourceLabel, (int)ourResults.size(), loaded.skippedCount, skippedLookupCount,
                ourResults, groundTruthScores, startProfile, discoveredProfile);

    cout << endl;
    cout << "Calibration complete (" << sourceLabel << ")." << endl;
    cout << "Number of pairs used: " << ourResults.size() << endl;
    cout << "Report saved: " << outputPath << endl;
}
