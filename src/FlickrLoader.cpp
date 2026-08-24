// This includes the FlickrLoader class declaration, file reading, and a map for grouping descriptions by image
#include "FlickrLoader.h"
#include <fstream>
#include <map>

using namespace std;

// This function loads the Flickr8k.token.txt file
vector<FlickrItem> FlickrLoader::load(string filePath, int limit) {

    vector<FlickrItem> result;
    ifstream file(filePath);

    // If the file cannot be opened, then I return an empty list
    if (!file.is_open()) {
        return result;
    }

    map<string, vector<string>> grouped;  // These are descriptions, grouped by image name
    string line;

    // This loop reads the file line by line
    while (getline(file, line)) {
        if (line.size() == 0) {
            continue;
        }

        // Here I look for a tab between the image name and the description; if there isn't one, I try a space
        size_t tabPos = line.find('\t');
        if (tabPos == string::npos) {
            tabPos = line.find(' ');
        }

        // If there is no separator at all, the line is not valid
        if (tabPos == string::npos) {
            continue;
        }

        string imagePart = line.substr(0, tabPos);
        string caption = line.substr(tabPos + 1);

        // The image part might look like "image.jpg#0" - I keep only the file name
        size_t hashPos = imagePart.find('#');
        if (hashPos != string::npos) {
            imagePart = imagePart.substr(0, hashPos);
        }

        if (imagePart.size() == 0 || caption.size() == 0) {
            continue;
        }

        grouped[imagePart].push_back(caption);
    }

    file.close();

    // This loop converts the grouped map into a list of FlickrItems, respecting the limit
    for (auto it = grouped.begin(); it != grouped.end(); it++) {
        if (limit > 0 && (int)result.size() >= limit) {
            break;
        }

        FlickrItem item;
        item.imageName = it->first;
        item.captions = it->second;

        result.push_back(item);
    }

    return result;
}
