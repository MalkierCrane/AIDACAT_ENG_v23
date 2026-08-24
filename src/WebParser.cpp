// This includes the WebParser class declaration and character functions
#include "WebParser.h"
#include <cctype>

using namespace std;

// This function converts text to lowercase
string WebParser::toLowerText(string text) {

    string result = "";

    for (int i = 0; i < (int)text.size(); i++) {
        result += (char)tolower((unsigned char)text[i]);
    }

    return result;
}

// This function reads an attribute value from a single HTML tag
string WebParser::getAttribute(string tag, string name) {

    string lower = toLowerText(tag);
    string search = toLowerText(name) + "=";

    size_t pos = lower.find(search);
    if (pos == string::npos) {
        return "";
    }
    pos = pos + search.size();

    // Here I skip the whitespace between the attribute name and the value
    while (pos < tag.size() && isspace((unsigned char)tag[pos])) {
        pos++;
    }

    if (pos >= tag.size()) {
        return "";
    }

    char quoteChar = tag[pos];

    // If the value is in quotes, I read it up to the next quote of the same kind
    if (quoteChar == '"' || quoteChar == '\'') {
        pos++;

        size_t end = tag.find(quoteChar, pos);
        if (end == string::npos) {
            return "";
        }

        return tag.substr(pos, end - pos);
    }

    // If there are no quotes, then I read until whitespace or the end of the tag
    size_t end = pos;
    while (end < tag.size() && !isspace((unsigned char)tag[end]) && tag[end] != '>') {
        end++;
    }

    return tag.substr(pos, end - pos);
}

// This function takes the first image address from srcset
string WebParser::getFirstFromSrcset(string srcset) {
    if (srcset.size() == 0) {
        return "";
    }

    // If there are multiple images, separated by a comma, I keep only the first one
    size_t comma = srcset.find(',');
    string first = srcset;
    if (comma != string::npos) {
        first = srcset.substr(0, comma);
    }

    // Here I trim off the size part after the space, for example "300w"
    size_t space = first.find(' ');
    if (space != string::npos) {
        first = first.substr(0, space);
    }

    return first;
}

// This function obtains the page's base URL
string WebParser::getBaseUrl(string pageUrl) {

    size_t protocol = pageUrl.find("://");
    if (protocol == string::npos) {
        return pageUrl;
    }

    size_t slash = pageUrl.find('/', protocol + 3);
    if (slash == string::npos) {
        return pageUrl;
    }

    return pageUrl.substr(0, slash);
}

// This function converts a relative image path into a full URL
string WebParser::resolveUrl(string pageUrl, string src) {
    if (src.size() == 0) {
        return "";
    }

    if (src.find("http://") == 0 || src.find("https://") == 0) {
        return src;
    }

    // If the URL starts with //, it is a protocol-relative path - I prepend https
    if (src.find("//") == 0) {
        return "https:" + src;
    }

    string base = getBaseUrl(pageUrl);

    if (src[0] == '/') {
        return base + src;
    }

    return base + "/" + src;
}

/* This function checks whether the image tag is located inside an <a> link. I search backwards from the image's position for the nearest valid "<a" tag 
(not "<article" or "<aside"), and then check whether there is also a "</a" closing tag between this "<a" and the image - if not, the link is not yet closed, so the image 
is inside it. This is not a real HTML tree check, only text searching, just like the rest of the WebParser class. */
bool WebParser::isImageWrappedInAnchor(string html, int imgTagPosition) {

    string lower = toLowerText(html);
    size_t searchPos = (size_t)imgTagPosition;
    size_t openPos = string::npos;

    // This loop searches backwards for a valid <a> tag, skipping "<article"/"<aside" cases
    while (true) {
        size_t candidate = lower.rfind("<a", searchPos);
        if (candidate == string::npos) {
            break;
        }

        // Here I take the character right after "<a", to check whether it really is an <a> tag
        char afterA = ' ';
        if (candidate + 2 < lower.size()) {
            afterA = lower[candidate + 2];
        }

        // If there is a space, ">" or a tab after "<a", this is a real link tag, not "<article" or "<aside"
        if (afterA == ' ' || afterA == '>' || afterA == '\t' || afterA == '\n') {
            openPos = candidate;
            break;
        }

        if (candidate == 0) {
            break;
        }

        searchPos = candidate - 1;
    }

    if (openPos == string::npos) {
        return false;
    }

    // Here I search for the nearest link closing tag "</a" before the image tag
    size_t closePos = lower.rfind("</a", (size_t)imgTagPosition);

    // If there is no closing tag, or it is further back than the opening tag, the link is not yet closed
    if (closePos == string::npos || closePos < openPos) {
        return true;
    }

    return false;
}

// This function finds img tags in the HTML text
vector<ImageItem> WebParser::parseImages(string html, string pageUrl, int limit) {

    vector<ImageItem> result;
    string lower = toLowerText(html);
    size_t pos = 0;

    // This loop searches for all img tags, until the limit is reached
    while (true) {
        if (limit > 0 && (int)result.size() >= limit) {
            break;
        }

        size_t start = lower.find("<img", pos);
        if (start == string::npos) {
            break;
        }

        size_t end = lower.find('>', start);
        if (end == string::npos) {
            break;
        }

        string tag = html.substr(start, end - start + 1);
        string src = getAttribute(tag, "src");

        // If there is no src directly, I try data-src, then srcset
        if (src.size() == 0) {
            src = getAttribute(tag, "data-src");
        }
        if (src.size() == 0) {
            string srcset = getAttribute(tag, "srcset");
            src = getFirstFromSrcset(srcset);
        }

        ImageItem item;
        item.pageUrl = pageUrl;
        item.src = src;
        item.imageUrl = resolveUrl(pageUrl, src);
        item.altText = getAttribute(tag, "alt");

        if (item.altText.size() == 0) {
            item.note = "alt text was not found or is empty";
        }
        else {
            item.note = "ok";
        }

        // This checks whether the image is located inside an <a> link - this is needed for the ImagePurpose class
        item.wrappedInAnchor = isImageWrappedInAnchor(html, (int)start);
        item.purpose = "";  // I do not determine the image's purpose at this step yet, ImagePurpose does that later

        if (item.imageUrl.size() > 0) {
            result.push_back(item);
        }

        pos = end + 1;
    }

    return result;
}
