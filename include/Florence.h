#ifndef FLORENCE_H
#define FLORENCE_H

// Include the string type
#include <string>

using namespace std;

// This class calls the Python script with the Florence-2 model
class Florence {
private:
    string pythonExe;                  // Python executable path
    string scriptPath;                 // Florence Python script path
    string modelName;                  // Florence model name
    string quote(string value);        // Puts text in quotes for the command line
    string runCommand(string command); // The command is executed and the output is read
    string findScript();               // The Python script is found in the project folder

    /* Here the diagnostic line (e.g. "Florence device: cpu") is removed, in case it somehow still ended up in the output as before.
    This is an extra safety step - the main fix is that I no longer add 2>&1 to the command, so stderr diagnostics are normally not read at all. */
    string stripDiagnosticLine(string text);

public:
    Florence();                               // Constructor with default values
    void setPythonExe(string path);           // This sets the Python path
    void setModelName(string model);          // This sets the model name
    string generateCaption(string imagePath); // And this generates a caption for one image

    /* Here the Florence-2 object detection task is called and the number of regions found is returned
    It is used by the ImagePurpose class to assess image complexity (complex category). Returns -1 if an error occurred. */
    int countRegions(string imagePath);
};

#endif
