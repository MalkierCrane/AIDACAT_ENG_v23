// This includes the ImagePurpose class declaration, a string stream for splitting words, and character functions for removing punctuation
#include "ImagePurpose.h"
#include <sstream>
#include <cctype>

using namespace std;

// This constructor prepares a list of words that indicate a functional image (button, link, icon with an action), rather than simply an informative illustration
ImagePurpose::ImagePurpose() {

    // These are the most common "action" words that tend to be put in the alt text of buttons and links
    functionalKeywords.push_back("click");
    functionalKeywords.push_back("submit");
    functionalKeywords.push_back("search");
    functionalKeywords.push_back("download");
    functionalKeywords.push_back("play");
    functionalKeywords.push_back("menu");
    functionalKeywords.push_back("button");
    functionalKeywords.push_back("link");
    functionalKeywords.push_back("close");
    functionalKeywords.push_back("open");
    functionalKeywords.push_back("login");
    functionalKeywords.push_back("logout");
    functionalKeywords.push_back("sign in");
    functionalKeywords.push_back("sign up");
    functionalKeywords.push_back("next");
    functionalKeywords.push_back("previous");
    functionalKeywords.push_back("cart");
    functionalKeywords.push_back("checkout");
}

/* This function checks whether any of the functional keywords appears in the alt text (already lowercased).
NOTE ON WHOLE-WORD COMPARISON: In the previous versions, I simply searched for the keyword as a substring anywhere within the text (altTextLower.find(...)).
This turned out to be buggy - for example, a completely ordinary description "A child is playing outside" was classified as "functional" just because the word "play" is
found INSIDE the word "playing". The same would have happened with "display", "playground" and similar words. Therefore, single-word keywords are now compared
against EACH individual word of the alt text directly (not as a substring), but phrases (such as "sign in", which contain a space) are still searched for as a substring, since
comparing a whole phrase to a single word doesn't make any sense. */
bool ImagePurpose::containsFunctionalKeyword(string altTextLower) {

    // This loop goes through all the keywords
    for (int i = 0; i < (int)functionalKeywords.size(); i++) {
        string keyword = functionalKeywords[i];

        // If the keyword contains a space, it is a phrase - I search for it as a substring
        if (keyword.find(' ') != string::npos) {
            if (altTextLower.find(keyword) != string::npos) {
                return true;
            }

            continue;
        }

        // If the keyword is a single word, then I split the alt text into separate words and compare each one
        stringstream ss(altTextLower);
        string word;

        while (ss >> word) {

            // Here I remove punctuation from the beginning and end of the word (for example, "play." or "play,")
            while (word.size() > 0 && !isalnum((unsigned char)word[word.size() - 1])) {
                word = word.substr(0, word.size() - 1);
            }
            while (word.size() > 0 && !isalnum((unsigned char)word[0])) {
                word = word.substr(1);
            }

            if (word == keyword) {
                return true;
            }
        }
    }

    return false;
}

// This function determines the image's purpose without the Florence region count. It is used in single mode, where it is not known whether the image is wrapped in a link.
string ImagePurpose::classifyFromAlt(string altText, bool isWrappedInLink) {

    /* Here I call the full version with regionCount=0, which means "information not available", not "the image has no regions at all" - the complex category here
can therefore only be reached through the long alt text rule, not through the region count. */
    return classifyFromAlt(altText, isWrappedInLink, 0);
}

// This function determines the image's purpose using the alt text, the link context, and, if available, the number of regions found in the image by Florence-2
string ImagePurpose::classifyFromAlt(string altText, bool isWrappedInLink, int regionCount) {

    string altLower = textProc.toLowerText(altText);  // Alt text in lowercase, so that the search is not case-sensitive

    // Here I remove extra spaces from the end and the beginning, to check whether the text is really empty
    string trimmed = altLower;
    while (trimmed.size() > 0 && trimmed[trimmed.size() - 1] == ' ') {
        trimmed = trimmed.substr(0, trimmed.size() - 1);
    }
    while (trimmed.size() > 0 && trimmed[0] == ' ') {
        trimmed = trimmed.substr(1);
    }

    // If the alt text is empty after removing spaces, the image is decorative - in accessibility guidelines, an empty alt="" specifically means the image has no content meaning
    if (trimmed.size() == 0) {
        return "decorative";
    }

    // If the image is wrapped in a link, or the alt text contains any action word, it is a functional image
    if (isWrappedInLink || containsFunctionalKeyword(altLower)) {
        return "functional";
    }

    // If the alt text is very long (120 or more characters), I assume the image is complex (for example, a chart or diagram that needs a long description)
    if ((int)altText.size() >= 120) {
        return "complex";
    }

    // If Florence found 5 or more regions in the image, that also indicates a complex image
    if (regionCount >= 5) {
        return "complex";
    }

    // If none of the previous rules apply, I consider the image to be a normal informative image
    return "informative";
}
