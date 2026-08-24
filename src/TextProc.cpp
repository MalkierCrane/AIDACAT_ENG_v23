// This includes the TextProc class declaration, character functions and the text stream for splitting words
#include "TextProc.h"
#include <cctype>
#include <sstream>

using namespace std;

// This constructor prepares the stop words and a simple normalization dictionary
TextProc::TextProc() {

    // These are the most common English stop words, which help little in comparing image content
    stopWords.insert("a");
    stopWords.insert("an");
    stopWords.insert("the");
    stopWords.insert("is");
    stopWords.insert("are");
    stopWords.insert("was");
    stopWords.insert("were");
    stopWords.insert("in");
    stopWords.insert("on");
    stopWords.insert("at");
    stopWords.insert("of");
    stopWords.insert("to");
    stopWords.insert("with");
    stopWords.insert("and");
    stopWords.insert("or");
    stopWords.insert("by");
    stopWords.insert("for");
    stopWords.insert("into");
    stopWords.insert("from");
    stopWords.insert("near");
    stopWords.insert("next");
    stopWords.insert("front");
    stopWords.insert("while");

    // Simple plural and verb forms that I normalize to the singular or base form
    normalWords["dogs"] = "dog";
    normalWords["puppy"] = "dog";
    normalWords["puppies"] = "dog";
    normalWords["men"] = "man";
    normalWords["women"] = "woman";
    normalWords["boys"] = "boy";
    normalWords["girls"] = "girl";
    normalWords["children"] = "child";
    normalWords["kids"] = "child";
    normalWords["kid"] = "child";
    normalWords["boy"] = "child";
    normalWords["girl"] = "child";
    normalWords["running"] = "run";
    normalWords["runs"] = "run";
    normalWords["ran"] = "run";
    normalWords["playing"] = "play";
    normalWords["plays"] = "play";
    normalWords["played"] = "play";
    normalWords["jumping"] = "jump";
    normalWords["jumps"] = "jump";
    normalWords["jumped"] = "jump";
    normalWords["leaping"] = "jump";
    normalWords["leaps"] = "jump";
    normalWords["climbing"] = "climb";
    normalWords["climbs"] = "climb";
    normalWords["walking"] = "walk";
    normalWords["walks"] = "walk";
    normalWords["standing"] = "stand";
    normalWords["stands"] = "stand";
    normalWords["sitting"] = "sit";
    normalWords["sits"] = "sit";
    normalWords["bicycle"] = "bike";
    normalWords["bicycles"] = "bike";
    normalWords["motorcycle"] = "bike";
    normalWords["football"] = "ball";
    normalWords["soccer"] = "ball";
}

// This function checks whether a word is a stop word
bool TextProc::isStopWord(string word) {
    return stopWords.find(word) != stopWords.end();
}

// This function converts text to lowercase
string TextProc::toLowerText(string text) {

    string result = "";

    // This loop converts each character to lowercase
    for (int i = 0; i < (int)text.size(); i++) {
        result += (char)tolower((unsigned char)text[i]);
    }

    return result;
}

// This function removes punctuation and unnecessary characters
string TextProc::cleanText(string text) {

    string result = "";
    text = toLowerText(text);

    // This loop goes through all characters, keeping letters and digits, replacing everything else with a space
    for (int i = 0; i < (int)text.size(); i++) {
        char c = text[i];

        if (isalnum((unsigned char)c)) {
            result += c;
        }
        else {
            result += ' ';
        }
    }

    return result;
}

// This function normalizes a single word
string TextProc::normalizeWord(string word) {

    // If the word is in the dictionary, then I return its replacement
    if (normalWords.find(word) != normalWords.end()) {
        return normalWords[word];
    }

    // If the word ends with s and is longer than 3 characters, I remove the trailing s
    if (word.size() > 3 && word[word.size() - 1] == 's') {
        return word.substr(0, word.size() - 1);
    }

    return word;
}

// This function splits text into words
vector<string> TextProc::tokenize(string text) {

    vector<string> words;
    string clean = cleanText(text);
    stringstream ss(clean);
    string word;

    // This loop reads each word, normalizes it, and skips words that are too short and stop words
    while (ss >> word) {
        word = normalizeWord(word);

        if (word.size() < 2) {
            continue;
        }

        if (isStopWord(word)) {
            continue;
        }

        words.push_back(word);
    }

    return words;
}

