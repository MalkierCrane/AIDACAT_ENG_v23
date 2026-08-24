// This includes the Evaluation class declaration, screen output, file writing, filesystem for folder creation, and formatting functions
#include "Evaluation.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <iomanip>

using namespace std;

namespace fs = filesystem;

// The function creates the output folder if it has not been created yet
void Evaluation::makeOutputDir() {

    if (!fs::exists("output")) {
        fs::create_directory("output");
    }

    if (!fs::exists("output/downloaded_images")) {
        fs::create_directories("output/downloaded_images");
    }
}

// The function prepares text for the CSV file
string Evaluation::csvSafe(string value) {

    string result = "\"";

    // This loop copies each character; CSV needs to double up quotes for quotes inside the text
    for (int i = 0; i < (int)value.size(); i++) {
        if (value[i] == '"') {
            result += "\"\"";
        }
        else {
            result += value[i];
        }
    }

    result += "\"";

    return result;
}

// This function joins the folder and file path
string Evaluation::joinPath(string folder, string file) {

    fs::path path = fs::path(folder) / fs::path(file);

    return path.string();
}

// This function creates a local file name for the image
string Evaluation::makeFileName(int index) {

    string name = "image_" + to_string(index) + ".jpg";

    return name;
}

// This function prints the metrics to the screen
void Evaluation::printMetric(MetricResult result) {

    cout << "Precision : " << fixed << setprecision(3) << result.precision << endl;
    cout << "Recall    : " << fixed << setprecision(3) << result.recall << endl;
    cout << "F1        : " << fixed << setprecision(3) << result.f1 << endl;
    cout << "BLEU      : " << fixed << setprecision(3) << result.bleu << endl;
    cout << "METEOR    : " << fixed << setprecision(3) << result.meteor << endl;
    cout << "ROUGE-L   : " << fixed << setprecision(3) << result.rougeL << endl;
    cout << "CIDEr     : " << fixed << setprecision(3) << result.cider << endl;
    cout << "Semantic  : " << fixed << setprecision(3) << result.semantic << endl;
    cout << "Final     : " << fixed << setprecision(2) << result.finalScore << " / 100" << endl;
    cout << "Level     : " << result.level << endl;
    cout << "Purpose   : " << result.purpose << endl;
}

/* This function loads the corpus statistics from the full Flickr8k.token.txt file, if this has not been done yet. It is used for the CIDER metric's TF-IDF weight calculation
in all modes, not only in experiment mode. If the file is not found (for example, if someone runs the program from a different folder), I print a warning
and let the program continue with the simplified, non-corpus CIDER version. */
void Evaluation::ensureCorpusStats() {

    // If I have already tried to load the corpus during this run, I don't do it again
    if (corpusStatsLoaded) {
        return;
    }

    corpusStatsLoaded = true;  // Here I mark that the attempt happened, regardless of the result

    string corpusPath = "data/Flickr8k_text/Flickr8k.token.txt";
    vector<FlickrItem> corpusItems = flickrLoader.load(corpusPath, 0);

    // If the corpus did not load (file not found or empty), print a warning and stop
    if (corpusItems.size() == 0) {
        cout << "Warning: corpus file '" << corpusPath << "' was not found or is empty." << endl;
        cout << "The CIDER metric will use the simplified version without corpus TF-IDF weights." << endl;

        corpusStatsAvailable = false;
        return;
    }

    // Here I collect all the reference captions into one list, so that I can build the corpus statistics
    vector<string> allCaptions;
    for (int i = 0; i < (int)corpusItems.size(); i++) {
        for (int j = 0; j < (int)corpusItems[i].captions.size(); j++) {
            allCaptions.push_back(corpusItems[i].captions[j]);
        }
    }

    corpusStats = metrics.buildCorpusStats(allCaptions);
    corpusStatsAvailable = true;
}

/* This function calculates the metrics for one pair, using the corpus CIDER (if available) and adding the semantic similarity with the given weight profile.
It is used by the single and web modes, where the number of images is small and an extra Python call for semantic similarity is not a problem.
Experiment mode DOES NOT USE THIS - see the note in the runExperiment function. */
MetricResult Evaluation::scoreTexts(string referenceText, string candidateText, WeightProfile profile) {

    ensureCorpusStats();

    MetricResult result;

    // If the corpus is available, I use the corpus CIDER version, otherwise the simplified non-corpus one
    if (corpusStatsAvailable) {
        result = metrics.evaluate(referenceText, candidateText, corpusStats);
    }
    else {
        result = metrics.evaluate(referenceText, candidateText);
    }

    double semanticScore = semanticSimilarity.computeSimilarity(referenceText, candidateText);
    result = metrics.attachSemantic(result, semanticScore, profile);

    return result;
}

// This function sets the Python path for both the Florence and SemanticSimilarity objects, since both use the same Python virtual environment
void Evaluation::setPythonExe(string path) {

    florence.setPythonExe(path);
    semanticSimilarity.setPythonExe(path);
}

// This function sets the Florence model name
void Evaluation::setModelName(string model) {
    florence.setModelName(model);
}

// This function sets the sentence-transformers model name
void Evaluation::setSemanticModelName(string model) {
    semanticSimilarity.setModelName(model);
}

// This function runs the Flickr8k validation mode with error analysis
void Evaluation::runExperiment(string tokenFile, int limit, int topN) {

    makeOutputDir();

    vector<FlickrItem> items = flickrLoader.load(tokenFile, limit);
    ensureCorpusStats();

    ofstream csv("output/flickr_validation_results.csv");
    csv << "image,reference,test_caption,precision,recall,f1,bleu,meteor,rouge_l,cider,semantic,final_score,level" << endl;

    double totalScore = 0;
    int count = 0;
    vector<ScoredPair> allPairs;

    // This loop goes through each image and compares its first caption with the rest
    for (int i = 0; i < (int)items.size(); i++) {
        if (items[i].captions.size() < 2) {
            continue;
        }

        string reference = items[i].captions[0];

        for (int j = 1; j < (int)items[i].captions.size(); j++) {
            string testCaption = items[i].captions[j];

            /* IMPORTANT NOTE: in experiment mode I deliberately DO NOT calculate the semantic metric. The full Flickr8k dataset has about 32000 comparable
            pairs - if a separate Python process were started for each one to calculate semantic similarity, the whole process would take an insanely long time.
            This is the same reason why this mode has never called Florence either, ever since the first version of the program - it remains a purely text metric
            comparison, so that it can be run on the whole dataset in a reasonable amount of time. */
            MetricResult result;
            if (corpusStatsAvailable) {
                result = metrics.evaluate(reference, testCaption, corpusStats);
            }
            else {
                result = metrics.evaluate(reference, testCaption);
            }

            totalScore += result.finalScore;
            count++;

            csv << csvSafe(items[i].imageName) << ",";
            csv << csvSafe(reference) << ",";
            csv << csvSafe(testCaption) << ",";
            csv << result.precision << ",";
            csv << result.recall << ",";
            csv << result.f1 << ",";
            csv << result.bleu << ",";
            csv << result.meteor << ",";
            csv << result.rougeL << ",";
            csv << result.cider << ",";
            csv << result.semantic << ",";
            csv << result.finalScore << ",";
            csv << csvSafe(result.level) << endl;

            ScoredPair pair;
            pair.imageName = items[i].imageName;
            pair.referenceText = reference;
            pair.candidateText = testCaption;
            pair.metrics = result;

            allPairs.push_back(pair);
        }
    }

    csv.close();

    cout << "Flickr8k validation mode completed" << endl;
    cout << "Number of comparisons: " << count << endl;

    if (count > 0) {
        double average = totalScore / count;
        cout << "Average final score: " << fixed << setprecision(2) << average << " / 100" << endl;
    }

    cout << "Results saved: output/flickr_validation_results.csv" << endl;

    errorAnalysis.runAnalysis(allPairs, "output/flickr_error_analysis.txt", topN);
}

// This function checks a single local image and the user's caption
void Evaluation::runSingle(string imagePath, string altText, bool classifyComplexity) {

    makeOutputDir();

    cout << "Image: " << imagePath << endl;
    cout << "Alt text being checked: " << altText << endl;

    string reference = florence.generateCaption(imagePath);
    cout << "Florence reference: " << reference << endl;

    /* If Florence returned an error, I cannot meaningfully continue the evaluation - I write a zero row,
    the same way the web mode already does when an image fails to download. */
    if (reference.size() >= 6 && reference.substr(0, 6) == "ERROR:") {
        cout << "Error: Florence did not generate a description, full evaluation is not performed." << endl;

        ofstream csv("output/single_result.csv");
        csv << "image,reference,alt_text,precision,recall,f1,bleu,meteor,rouge_l,cider,semantic,final_score,level,purpose" << endl;
        csv << csvSafe(imagePath) << ",";
        csv << csvSafe(reference) << ",";
        csv << csvSafe(altText) << ",";
        csv << "0,0,0,0,0,0,0,0,0,";
        csv << csvSafe("insufficient") << ",";
        csv << csvSafe("n/a") << endl;
        csv.close();

        cout << "Results saved: output/single_result.csv" << endl;
        return;
    }

    // If complexity detection is enabled, I call the Florence region detection
    int regionCount = 0;
    if (classifyComplexity) {
        regionCount = florence.countRegions(imagePath);
        if (regionCount < 0) {
            regionCount = 0;
        }
    }

    // Here I determine the image's function. In single mode there is no <a> context, so isWrappedInLink is always false.
    string purpose;
    if (classifyComplexity) {
        purpose = imagePurpose.classifyFromAlt(altText, false, regionCount);
    }
    else {
        purpose = imagePurpose.classifyFromAlt(altText, false);
    }

    WeightProfile profile = coefficients.getProfile(purpose);
    MetricResult result = scoreTexts(reference, altText, profile);
    result.purpose = purpose;

    printMetric(result);

    ofstream csv("output/single_result.csv");
    csv << "image,reference,alt_text,precision,recall,f1,bleu,meteor,rouge_l,cider,semantic,final_score,level,purpose" << endl;
    csv << csvSafe(imagePath) << ",";
    csv << csvSafe(reference) << ",";
    csv << csvSafe(altText) << ",";
    csv << result.precision << ",";
    csv << result.recall << ",";
    csv << result.f1 << ",";
    csv << result.bleu << ",";
    csv << result.meteor << ",";
    csv << result.rougeL << ",";
    csv << result.cider << ",";
    csv << result.semantic << ",";
    csv << result.finalScore << ",";
    csv << csvSafe(result.level) << ",";
    csv << csvSafe(result.purpose) << endl;
    csv.close();

    cout << "Results saved: output/single_result.csv" << endl;
}

// This function runs the real web page mode
void Evaluation::runWeb(string url, int limit, bool classifyComplexity) {

    makeOutputDir();

    cout << "Downloading HTML from: " << url << endl;
    string html = downloader.downloadText(url);

    if (html.size() == 0) {
        cout << "Error: HTML was not downloaded" << endl;
        return;
    }

    vector<ImageItem> images = webParser.parseImages(html, url, limit);
    cout << "Images found: " << images.size() << endl;

    ofstream csv("output/web_accessibility_results.csv");
    csv << "index,image_url,local_path,alt_text,reference,precision,recall,f1,bleu,meteor,rouge_l,cider,semantic,final_score,level,note,purpose,wrapped_in_anchor" << endl;

    double totalScore = 0;
    int scoredCount = 0;

    // This loop processes each found image - downloads it, generates the Florence reference caption, and compares it with the alt text
    for (int i = 0; i < (int)images.size(); i++) {
        cout << endl << "Processing image " << (i + 1) << " of " << images.size() << endl;

        string localPath = joinPath("output/downloaded_images", makeFileName(i + 1));
        images[i].localPath = localPath;

        bool downloaded = downloader.downloadFile(images[i].imageUrl, localPath);

        // If the image cannot be downloaded, I write an error row and continue with the next one
        if (!downloaded) {
            csv << (i + 1) << ",";
            csv << csvSafe(images[i].imageUrl) << ",";
            csv << csvSafe(localPath) << ",";
            csv << csvSafe(images[i].altText) << ",";
            csv << csvSafe("") << ",";
            csv << "0,0,0,0,0,0,0,0,0,";
            csv << csvSafe("insufficient") << ",";
            csv << csvSafe("failed to download the image") << ",";
            csv << csvSafe("n/a") << ",";
            csv << (images[i].wrappedInAnchor ? "true" : "false") << endl;
            continue;
        }

        // If the alt text is empty, that by definition means a decorative image - then I write a row without Florence comparison
        if (images[i].altText.size() == 0) {
            csv << (i + 1) << ",";
            csv << csvSafe(images[i].imageUrl) << ",";
            csv << csvSafe(localPath) << ",";
            csv << csvSafe(images[i].altText) << ",";
            csv << csvSafe("") << ",";
            csv << "0,0,0,0,0,0,0,0,0,";
            csv << csvSafe("insufficient") << ",";
            csv << csvSafe("alt text was not found or is empty") << ",";
            csv << csvSafe("decorative") << ",";
            csv << (images[i].wrappedInAnchor ? "true" : "false") << endl;
            continue;
        }

        string reference = florence.generateCaption(localPath);

        // If Florence returned an error, I write an error row and continue with the next image
        if (reference.size() >= 6 && reference.substr(0, 6) == "ERROR:") {
            csv << (i + 1) << ",";
            csv << csvSafe(images[i].imageUrl) << ",";
            csv << csvSafe(localPath) << ",";
            csv << csvSafe(images[i].altText) << ",";
            csv << csvSafe(reference) << ",";
            csv << "0,0,0,0,0,0,0,0,0,";
            csv << csvSafe("insufficient") << ",";
            csv << csvSafe("Florence generation failed") << ",";
            csv << csvSafe("n/a") << ",";
            csv << (images[i].wrappedInAnchor ? "true" : "false") << endl;
            continue;
        }

        // If complexity detection is enabled, then I call the Florence region detection for this local image
        int regionCount = 0;
        if (classifyComplexity) {
            regionCount = florence.countRegions(localPath);
            if (regionCount < 0) {
                regionCount = 0;
            }
        }

        // Here I determine the image's function, using the alt text, the link context, and, if available, the region count
        string purpose;
        if (classifyComplexity) {
            purpose = imagePurpose.classifyFromAlt(images[i].altText, images[i].wrappedInAnchor, regionCount);
        }
        else {
            purpose = imagePurpose.classifyFromAlt(images[i].altText, images[i].wrappedInAnchor);
        }

        WeightProfile profile = coefficients.getProfile(purpose);
        MetricResult result = scoreTexts(reference, images[i].altText, profile);
        result.purpose = purpose;

        totalScore += result.finalScore;
        scoredCount++;

        csv << (i + 1) << ",";
        csv << csvSafe(images[i].imageUrl) << ",";
        csv << csvSafe(localPath) << ",";
        csv << csvSafe(images[i].altText) << ",";
        csv << csvSafe(reference) << ",";
        csv << result.precision << ",";
        csv << result.recall << ",";
        csv << result.f1 << ",";
        csv << result.bleu << ",";
        csv << result.meteor << ",";
        csv << result.rougeL << ",";
        csv << result.cider << ",";
        csv << result.semantic << ",";
        csv << result.finalScore << ",";
        csv << csvSafe(result.level) << ",";
        csv << csvSafe(images[i].note) << ",";
        csv << csvSafe(result.purpose) << ",";
        csv << (images[i].wrappedInAnchor ? "true" : "false") << endl;

        cout << "Alt: " << images[i].altText << endl;
        cout << "Reference: " << reference << endl;
        cout << "Purpose: " << purpose << endl;
        cout << "Score: " << fixed << setprecision(2) << result.finalScore << " / 100" << endl;
    }

    csv.close();

    cout << endl << "Web mode completed" << endl;
    cout << "Images scored with alt text: " << scoredCount << endl;

    if (scoredCount > 0) {
        double average = totalScore / scoredCount;
        cout << "Average accessibility score: " << fixed << setprecision(2) << average << " / 100" << endl;
    }

    cout << "Results saved: output/web_accessibility_results.csv" << endl;
}

// This function runs the coefficient calibration mode - compares my score with human ratings
void Evaluation::runCalibration(string expertFile, string tokenFile, int limit, bool useCrowdflower, string crowdflowerFile) {

    makeOutputDir();

    // Here I load all the Flickr8k images without a limit - the annotation file can point to any of them
    vector<FlickrItem> flickrItems = flickrLoader.load(tokenFile, 0);

    if (flickrItems.size() == 0) {
        cout << "Error: failed to load Flickr8k data from: " << tokenFile << endl;
        return;
    }

    ensureCorpusStats();

    CorpusStats statsToUse;
    if (corpusStatsAvailable) {
        statsToUse = corpusStats;
    }

    calibration.runCalibration(expertFile, flickrItems, statsToUse, "output/calibration_report.txt", limit, false, metrics, semanticSimilarity);

    // If the user also requested CrowdFlower calibration, I do that as well
    if (useCrowdflower) {
        string cfFile = crowdflowerFile;

        // If the user did not specify a specific path, then I use the Flickr8k dataset's default path
        if (cfFile.size() == 0) {
            cfFile = "data/Flickr8k_text/CrowdFlowerAnnotations.txt";
        }

        calibration.runCalibration(cfFile, flickrItems, statsToUse, "output/calibration_report_crowdflower.txt", limit, true, metrics, semanticSimilarity);
    }
}
