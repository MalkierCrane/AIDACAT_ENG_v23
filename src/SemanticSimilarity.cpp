/* This includes the SemanticSimilarity class declaration, a buffer for reading the command result, C functions
for launching a process, character functions, filesystem checking, and the cstdlib atof function. */
#include "SemanticSimilarity.h"
#include <array>
#include <cstdio>
#include <cctype>
#include <filesystem>
#include <cstdlib>

using namespace std;

namespace fs = filesystem;

// On Windows I use the _popen name
#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

// This constructor prepares the default values
SemanticSimilarity::SemanticSimilarity() {

    pythonExe = "python";
    scriptPath = "python/semantic_similarity.py";
    modelName = "all-MiniLM-L6-v2";  // A small, fast sentence-transformers model that is suitable for this prototype
}

// This function sets the Python executable path
void SemanticSimilarity::setPythonExe(string path) {
    if (path.size() > 0) {
        pythonExe = path;
    }
}

// This function sets the model name
void SemanticSimilarity::setModelName(string model) {
    if (model.size() > 0) {
        modelName = model;
    }
}

// This function puts the text in quotes for the command line
string SemanticSimilarity::quote(string value) {

    string result = "\"";

    for (int i = 0; i < (int)value.size(); i++) {
        if (value[i] == '"') {
            continue;
        }
        result += value[i];
    }

    result += "\"";
    return result;
}

// This function executes the command and reads its output
string SemanticSimilarity::runCommand(string command) {

    array<char, 512> buffer;
    string result = "";

    FILE* pipe = popen(command.c_str(), "r");
    if (pipe == nullptr) {
        return "ERROR: could not run Python command";
    }

    while (fgets(buffer.data(), (int)buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }

    pclose(pipe);

    // Here I remove line-ending characters and extra whitespace from the end
    while (result.size() > 0 && isspace((unsigned char)result.back())) {
        result.pop_back();
    }

    return result;
}

// This function finds the Python script in the project folder
string SemanticSimilarity::findScript() {

    fs::path path = fs::path(scriptPath);
    if (fs::exists(path)) {
        return fs::absolute(path).string();
    }

    // Here I try to look for the script from the current folder
    fs::path secondPath = fs::current_path() / path;
    if (fs::exists(secondPath)) {
        return fs::absolute(secondPath).string();
    }

    return scriptPath;
}

// This function checks whether the file exists
static bool fileExists(string path) {
    if (path.size() == 0) {
        return false;
    }

    return fs::exists(fs::path(path));
}

// This function calculates the semantic similarity between two texts
double SemanticSimilarity::computeSimilarity(string text1, string text2) {

    string script = findScript();

    // If the Python path is not simply "python", I check whether the file exists
    if (pythonExe != "python" && pythonExe != "python.exe") {
        if (!fileExists(pythonExe)) {
            return -1.0;
        }
    }

    if (!fileExists(script)) {
        return -1.0;
    }

    // If either of the texts is empty, semantic similarity is not meaningful
    if (text1.size() == 0 || text2.size() == 0) {
        return 0.0;
    }

    // Here I start the command with cmd /C, so that Windows correctly handles paths and quotes
    string command = "cmd /C \"";
    command += quote(pythonExe);
    command += " ";
    command += quote(script);
    command += " ";
    command += quote(text1);
    command += " ";
    command += quote(text2);
    command += " --model ";
    command += quote(modelName);

    // Here I deliberately do NOT add "2>&1" - the Python script writes diagnostics to stderr, and I don't need that on the stdout side, just like in the Florence class
    command += "\"";

    string output = runCommand(command);

    if (output.size() == 0 || output.substr(0, 6) == "ERROR:") {
        return -1.0;
    }

    // Here I convert the text into a floating-point number
    double similarity = atof(output.c_str());

    return similarity;
}
