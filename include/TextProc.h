#ifndef TEXTPROC_H
#define TEXTPROC_H

// This includes the text type, the list type, the set type for stop words and the dictionary type for normalization
#include <string>
#include <vector>
#include <set>
#include <map>

using namespace std;

// This class prepares text for the metrics
class TextProc {
private:
    set<string> stopWords;              // Stop words that help little in comparing image content
    map<string, string> normalWords;    // Simple synonym and form dictionary
    bool isStopWord(string word);       // Checks whether a word is a stop word

public:
    TextProc();                           // Constructor prepares the dictionaries
    string toLowerText(string text);      // Then converts text to lowercase
    string cleanText(string text);        // Then removes punctuation and unnecessary characters
    string normalizeWord(string word);    // Then normalizes a single word
    vector<string> tokenize(string text); // And finally splits text into words
};

#endif

