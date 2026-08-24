#ifndef DOWNLOADER_H
#define DOWNLOADER_H

// I include the string type
#include <string>

using namespace std;

// This class uses curl to download HTML and images
class Downloader {
private:
    string quote(string value);        // The text is put in quotes
    string runCommand(string command); // This executes a command and reads the output

public:
    string downloadText(string url);             // This downloads the web page's HTML
    bool downloadFile(string url, string path);  // This downloads an image into a file
};

#endif
