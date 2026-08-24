#ifndef FLICKRLOADER_H
#define FLICKRLOADER_H

// This includes the common structures, the string type and the vector type
#include "Types.h"
#include <string>
#include <vector>

using namespace std;

// This class reads the Flickr8k.token.txt file
class FlickrLoader {
public:
    vector<FlickrItem> load(string filePath, int limit); // Here I load descriptions per image
};

#endif
