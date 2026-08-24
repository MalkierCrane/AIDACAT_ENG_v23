// This includes the Downloader class declaration, the buffer and C functions for running commands and reading output, and the filesystem check
#include "Downloader.h"
#include <array>
#include <cstdio>
#include <filesystem>

using namespace std;

namespace fs = filesystem;

// On Windows I use the _popen name
#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

// This function puts text in quotes for the command line
string Downloader::quote(string value) {

    string result = "\"";

    // This loop copies each character, skipping quotes, so they don't break the command-line argument
    for (int i = 0; i < (int)value.size(); i++) {
        if (value[i] == '"') {
            continue;
        }

        result += value[i];
    }

    result += "\"";

    return result;
}

// This function executes a command and reads its output
string Downloader::runCommand(string command) {

    array<char, 512> buffer;
    string result = "";

    FILE* pipe = popen(command.c_str(), "r");

    // If the process cannot be started, I return an empty string
    if (pipe == nullptr) {
        return "";
    }

    // This loop reads the command's output line by line
    while (fgets(buffer.data(), (int)buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }

    pclose(pipe);

    return result;
}

// This function downloads HTML text using curl
string Downloader::downloadText(string url) {

    string command = "curl -L -s ";
    command += quote(url);

    string html = runCommand(command);

    return html;
}

// This function downloads an image file using curl
bool Downloader::downloadFile(string url, string path) {

    string command = "curl -L -s -o ";
    command += quote(path);
    command += " ";
    command += quote(url);

    runCommand(command);

    // I check whether the file actually appeared after the download
    if (fs::exists(path)) {
        return true;
    }

    return false;
}
