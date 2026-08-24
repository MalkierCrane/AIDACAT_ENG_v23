/* This includes the Florence class declaration, a buffer for reading command output, C functions
for launching a process, character functions, filesystem checking, a text stream for building error text, and the cstdlib atoi function. */
#include "Florence.h"
#include <array>
#include <cstdio>
#include <cctype>
#include <filesystem>
#include <sstream>
#include <cstdlib>

using namespace std;

namespace fs = filesystem;

// On Windows I use the _popen name
#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

// This constructor prepares the default values
Florence::Florence() {

    pythonExe = "python";
    scriptPath = "python/florence_caption.py";
    modelName = "microsoft/Florence-2-base";  // For this work prototype I use the lightest base model
}

// This function sets the Python executable path
void Florence::setPythonExe(string path) {
    if (path.size() > 0) {
        pythonExe = path;
    }
}

//This function sets the Florence model name
void Florence::setModelName(string model) {
    if (model.size() > 0) {
        modelName = model;
    }
}

//This function puts text in quotes for the command line
string Florence::quote(string value) {

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
string Florence::runCommand(string command) {

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

    // Here I remove trailing newline characters and extra whitespace from the end
    while (result.size() > 0 && isspace((unsigned char)result.back())) {
        result.pop_back();
    }

    return result;
}

/*This function removes the diagnostic line from the Python output, if one has somehow ended up there. The main fix for this problem is that I no longer add "2>&1" to the
generateCaption and countRegions commands - so the Python side's stderr output (e.g. "Florence device: cpu") is not read through popen at all.
This function is just an extra safeguard in case I ever again add a new diagnostic print() without thinking about where it ends up. */
string Florence::stripDiagnosticLine(string text) {

    size_t newlinePos = text.find('\n');
    if (newlinePos == string::npos) {
        return text;
    }

    string firstLine = text.substr(0, newlinePos);

    // If the first line starts with the known diagnostic phrase, then return the text without it
    if (firstLine.find("Florence device") == 0) {
        return text.substr(newlinePos + 1);
    }

    return text;
}

//This function finds the Python script in the project folder
string Florence::findScript() {

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

//This function checks whether the file exists
static bool fileExists(string path) {
    if (path.size() == 0) {
        return false;
    }

    return fs::exists(fs::path(path));
}

//This function generates a caption for one image
string Florence::generateCaption(string imagePath) {

    string script = findScript();

    // If the Python path is not simply "python", check whether the file exists
    if (pythonExe != "python" && pythonExe != "python.exe") {
        if (!fileExists(pythonExe)) {
            return "ERROR: Python exe not found: " + pythonExe;
        }
    }

    if (!fileExists(script)) {
        return "ERROR: Florence script not found: " + script;
    }

    if (!fileExists(imagePath)) {
        return "ERROR: image file not found: " + imagePath;
    }

    // Here I start the command with cmd /C so Windows correctly handles paths in quotes
    string command = "cmd /C \"";
    command += quote(pythonExe);
    command += " ";
    command += quote(script);
    command += " ";
    command += quote(imagePath);
    command += " --model ";
    command += quote(modelName);

    /* IMPORTANT: here I no longer add "2>&1". In the previous version this line redirected the Python script's stderr output (e.g. "Florence device: cpu")
back onto stdout, which popen reads - this meant the diagnostic line always contaminated the measured caption. The Python script already writes diagnostics
to stderr precisely so that stdout stays clean - I simply didn't need to merge it back again. */
    command += "\"";

    string caption = runCommand(command);
    caption = stripDiagnosticLine(caption);  // Extra safeguard in case the diagnostic line is somehow still present >:(

    if (caption.size() == 0) {
        return "ERROR: Florence did not return caption";
    }

    return caption;
}

/*This function calls the Florence-2 object detection task and returns the number of regions found. It uses the <DENSE_REGION_CAPTION> task, which tells the Florence-2
model to find areas of the image. The region count serves as a simple complexity indicator for the ImagePurpose class. */
int Florence::countRegions(string imagePath) {

    string script = findScript();

    // If the Python path is not simply "python", check whether the file exists
    if (pythonExe != "python" && pythonExe != "python.exe") {
        if (!fileExists(pythonExe)) {
            return -1;
        }
    }

    if (!fileExists(script)) {
        return -1;
    }

    if (!fileExists(imagePath)) {
        return -1;
    }

    string command = "cmd /C \"";
    command += quote(pythonExe);
    command += " ";
    command += quote(script);
    command += " ";
    command += quote(imagePath);
    command += " --model ";
    command += quote(modelName);

    /* IMPORTANT: here I use the simple word "regions", NOT the real Florence-2 token "<DENSE_REGION_CAPTION>" - In the previous version I put
"<DENSE_REGION_CAPTION>" directly here, and Windows cmd.exe interpreted the "<" and ">" characters as file redirection signs even inside quotes, so the command always failed
with "The system cannot find the file specified". The Python script (florence_caption.py) now converts "regions" into the real Florence-2 token itself. */
    command += " --task ";
    command += quote("regions");

    // Here too I don't add 2>&1, so stderr diagnostics don't interfere with the result
    command += "\"";

    string output = runCommand(command);
    output = stripDiagnosticLine(output);

    if (output.size() == 0 || output.substr(0, 6) == "ERROR:") {
        return -1;
    }

    // Here atoi returns 0 if the text is not a valid number, which in this case is an acceptable default (zero regions)
    int regionCount = atoi(output.c_str());

    return regionCount;
}
