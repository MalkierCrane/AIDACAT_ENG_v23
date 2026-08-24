#ifndef TYPES_H
#define TYPES_H

// This includes the text type, list type, and dictionary type for corpus statistics
#include <string>
#include <vector>
#include <map>

using namespace std;

// This structure stores one image found on a web page
struct ImageItem {
    string pageUrl;        // This is the address of the page where the image was found
    string src;            // The image's src attribute from HTML
    string imageUrl;       // The full image address
    string localPath;      // The local path where the image is saved
    string altText;        // The alternative text from the alt attribute
    string note;           // A note, for example if the alt text was not found
    bool wrappedInAnchor;  // True if the img tag is inside an <a> link
    string purpose;        // The image's function: decorative / informative / functional / complex
};

// This structure stores one Flickr8k image and its captions
struct FlickrItem {
    string imageName;          // The image file name
    vector<string> captions;   // Human-made captions for this image, ordered by #0..#4
};

/* This structure stores one weight profile, which determines how much importance each metric has.
Depending on the image's function (decorative/informative/functional/complex) I use a different profile. */
struct WeightProfile {
    string name;          // The profile name, for example "informative"
    double wPrecision;    // Precision weight
    double wRecall;       // Recall weight
    double wF1;           // F1 weight
    double wBleu;         // BLEU weight
    double wMeteor;       // METEOR weight
    double wRougeL;       // ROUGE-L weight
    double wCider;        // CIDER weight
    double wSemantic;     // Semantic similarity weight
};

// This structure stores the corpus statistics used for the CIDER TF-IDF calculation. documentFrequency indicates how many reference captions each n-gram appears in.
struct CorpusStats {
    map<string, int> documentFrequency;   // The n-gram key indicates how many documents it appears in.
    int documentCount = 0;                // The total number of reference captions in the corpus. Starts at 0 while the corpus has not been loaded.
};

// This structure stores the metric result
struct MetricResult {
    double precision;      // Word precision
    double recall;         // Word coverage from the reference caption
    double f1;             // The combined precision and recall score
    double bleu;           // Simplified BLEU-4 metric
    double meteor;         // METEOR metric with real alignment and a fragmentation penalty
    double rougeL;         // ROUGE-L metric
    double cider;          // CIDER metric, with TF-IDF weights if corpus statistics are available
    double semantic;       // Semantic similarity from the sentence-transformers model, 0 to 1
    double finalScore;     // The final score is from 0 to 100
    string level;          // Text category in English: insufficient, weak, average, good, excellent
    string purpose;        // The image's function, if it was determined, otherwise "n/a"
};

#endif
