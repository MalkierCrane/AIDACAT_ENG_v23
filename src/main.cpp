// This includes the main Evaluation class, screen output, and the text type
#include "Evaluation.h"
#include <iostream>
#include <string>

using namespace std;

//This function prints the usage help
void printHelp() {

    cout << "Analysis of Image Descriptions Accuracy and Completeness for Accessibility Testing program" << endl;
    cout << endl;

    // Flickr8k mode example - by default with the full dataset (limit 0 = all images)
    cout << "1) Flickr8k validation (with error analysis):" << endl;
    cout << "   app.exe experiment data\\Flickr8k_text\\Flickr8k.token.txt 0" << endl;

    // Single image mode example
    cout << "2) Single image check:" << endl;
    cout << "   app.exe single sample_images\\test.jpg \"A dog is running outside\" --python C:\\path\\test_env\\Scripts\\python.exe" << endl;

    // Web mode example
    cout << "3) Web page check:" << endl;
    cout << "   app.exe web https://example.com 5 --python C:\\path\\test_env\\Scripts\\python.exe" << endl;

    // Coefficient calibration mode example
    cout << "4) Coefficient calibration against ExpertAnnotations.txt (each row ~5s, default limit 100):" << endl;
    cout << "   app.exe calibrate data\\Flickr8k_text\\ExpertAnnotations.txt data\\Flickr8k_text\\Flickr8k.token.txt --limit 30 --python C:\\path\\test_env\\Scripts\\python.exe" << endl;

    cout << endl;
    cout << "Additional parameters:" << endl;
    cout << "   --model microsoft/Florence-2-base" << endl;
    cout << "   --semantic-model all-MiniLM-L6-v2" << endl;
    cout << "   --classify-complexity     (enables Florence region counting in single/web modes)" << endl;
    cout << "   --limit N                 (in calibrate mode, how many rows to process, 0 = all)" << endl;
    cout << "   --crowdflower             (in calibrate mode, also calibrate against CrowdFlowerAnnotations.txt)" << endl;
    cout << "   --crowdflower-file <path>(in calibrate mode, alternate CrowdFlower file path)" << endl;
    cout << "   --top-n N                 (in experiment mode, how many worst/best pairs to show, default 10)" << endl;
}

// This function looks up a parameter's value in the argument list
string getArgValue(int argc, char* argv[], string name, string defaultValue) {

    for (int i = 1; i < argc - 1; i++) {
        if (string(argv[i]) == name) {
            return string(argv[i + 1]);
        }
    }

    return defaultValue;
}

// This function checks whether the specified flag (without a value) is present in the argument list
bool hasFlag(int argc, char* argv[], string name) {

    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == name) {
            return true;
        }
    }

    return false;
}

// This function is the program's entry point function
int main(int argc, char* argv[]) {

    // If there are not enough arguments, then I print the help and stop
    if (argc < 2) {
        printHelp();
        return 0;
    }

    Evaluation evaluation;

    // Here I read the Python path, and the Florence and semantic similarity model names, if the user specified them
    string pythonPath = getArgValue(argc, argv, "--python", "python");
    string modelName = getArgValue(argc, argv, "--model", "microsoft/Florence-2-base");
    string semanticModelName = getArgValue(argc, argv, "--semantic-model", "all-MiniLM-L6-v2");

    evaluation.setPythonExe(pythonPath);
    evaluation.setModelName(modelName);
    evaluation.setSemanticModelName(semanticModelName);

    bool classifyComplexity = hasFlag(argc, argv, "--classify-complexity");
    string mode = argv[1];

    // Flickr8k validation mode
    if (mode == "experiment") {
        if (argc < 4) {
            printHelp();
            return 0;
        }

        string tokenFile = argv[2];
        int limit = stoi(argv[3]);

        // This is - how many worst/best pairs to show in the error analysis
        string topNText = getArgValue(argc, argv, "--top-n", "10");
        int topN = stoi(topNText);

        evaluation.runExperiment(tokenFile, limit, topN);
        return 0;
    }

    // Single image mode
    if (mode == "single") {
        if (argc < 4) {
            printHelp();
            return 0;
        }

        string imagePath = argv[2];
        string altText = argv[3];

        evaluation.runSingle(imagePath, altText, classifyComplexity);
        return 0;
    }

    // Web page mode
    if (mode == "web") {
        if (argc < 4) {
            printHelp();
            return 0;
        }

        string url = argv[2];
        int limit = stoi(argv[3]);

        evaluation.runWeb(url, limit, classifyComplexity);
        return 0;
    }

    // Coefficient calibration mode
    if (mode == "calibrate") {
        if (argc < 4) {
            printHelp();
            return 0;
        }

        string expertFile = argv[2];
        string tokenFile = argv[3];

        /* Here is the limit for how many annotation rows to process. The default is NOT 0 (unlimited) like other modes - testing showed that each row
takes about 5 seconds (a new Python process for semantic similarity for each row), so running the full 5822 rows without a limit would take several hours.
To avoid accidentally starting such a long process, the default here is 100 rows (about 8-9 minutes) - the full dataset can be run with --limit 0, if it's really needed
and there is time to wait. */
        string limitText = getArgValue(argc, argv, "--limit", "100");
        int limit = stoi(limitText);

        string crowdflowerFile = getArgValue(argc, argv, "--crowdflower-file", "");

        // Here I also calibrate against CrowdFlower data, if the user requested it with a flag or specified a file
        bool useCrowdflower = hasFlag(argc, argv, "--crowdflower") || crowdflowerFile.size() > 0;

        evaluation.runCalibration(expertFile, tokenFile, limit, useCrowdflower, crowdflowerFile);
        return 0;
    }

    // If the mode is not recognized, I print the help
    printHelp();
    return 0;
}
