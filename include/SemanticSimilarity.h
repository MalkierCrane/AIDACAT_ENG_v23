#ifndef SEMANTICSIMILARITY_H
#define SEMANTICSIMILARITY_H

// I include the string type.
#include <string>

using namespace std;

/* This class calls a Python script with a sentence-transformers model to calculate the semantic similarity between two texts.
The structure is the same as the Florence class, since both classes do the same thing - run a Python script and read its output - only the model and input data differ. */
class SemanticSimilarity {
private:
    string pythonExe;                   // Python executable path
    string scriptPath;                  // Path to the semantic_similarity.py script
    string modelName;                   // sentence-transformers model name
    string quote(string value);         // I put the text in quotes for the command line
    string runCommand(string command);  // I execute the command and read the output
    string findScript();                // I find the Python script in the project folder

public:
    SemanticSimilarity();               // This is the constructor with default values
    void setPythonExe(string path);     // This sets the Python path
    void setModelName(string model);    // This sets the model name

    /* Here the semantic similarity between two texts is calculated, a value from 0 to 1.
    Returns -1.0 if an error occurred (for example, the Python script was not found). */
    double computeSimilarity(string text1, string text2);
};

#endif
