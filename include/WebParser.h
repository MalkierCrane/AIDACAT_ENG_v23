#ifndef WEBPARSER_H
#define WEBPARSER_H

// This includes the shared structures, the string type, and the vector type
#include "Types.h"
#include <string>
#include <vector>

using namespace std;

// This is a simple HTML parser that looks only for <img> tags
class WebParser {
private:
    string toLowerText(string text);                   // This converts text to lowercase
    string getAttribute(string tag, string name);      // Then reads an attribute from the img tag
    string getFirstFromSrcset(string srcset);          // Then takes the first image from srcset
    string getBaseUrl(string pageUrl);                 // Then obtains the page's base URL
    string resolveUrl(string pageUrl, string src);     // And then converts a relative path into a full URL

    /* Here it is checked whether the image tag is located inside an <a> link - I search backwards from the image's position.
This is a simple, not fully reliable heuristic (not a real HTML/DOM tree), because the parser as a whole is text scanning, not a real browser - the same limitation 
already applies to the rest of the WebParser functions. */
    bool isImageWrappedInAnchor(string html, int imgTagPosition);

public:
    vector<ImageItem> parseImages(string html, string pageUrl, int limit); // I find img tags
};

#endif
