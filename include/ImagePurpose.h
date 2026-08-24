#ifndef IMAGEPURPOSE_H
#define IMAGEPURPOSE_H

// This includes the text processing class for lowercase conversion, the string type and the vector type
#include "TextProc.h"
#include <string>
#include <vector>

using namespace std;

/* This class determines the image's purpose - decorative / informative / functional / complex.
This is a simplified approximation of the W3C WAI "alt decision tree" idea (https://www.w3.org/WAI/tutorials/images/decision-tree/), not a full implementation of it
Instead of real accessibility expertise, I use a few simple, easily explainable rules that can be compared to the W3C WAI image tutorial pages,
which are divided exactly according to these same categories. */
class ImagePurpose {
private:
    bool containsFunctionalKeyword(string altTextLower); // I check whether the alt text contains any "action" word

public:
    TextProc textProc;                    // Text processing object for lowercase conversion.
    vector<string> functionalKeywords;    // These are words that indicate the image is a button or link, rather than simply an illustration

    ImagePurpose(); // Constructor prepares the keyword list

    // I determine the purpose without the Florence region count - used in single mode, where there is no <a> context
    string classifyFromAlt(string altText, bool isWrappedInLink);

    // I determine the purpose with the Florence region count - used in web mode, if --classify-complexity is enabled
    string classifyFromAlt(string altText, bool isWrappedInLink, int regionCount);
};

#endif
