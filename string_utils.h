#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>  // For std::string
#include <vector>  // For std::vector
#include <sstream> // For std::istringstream
#include <algorithm> // For std::find_if
#include <cctype>    // For std::isspace

// trim functions to strip leading/trailing spaces (in-place)
// These are declared static inline, so their definition can be in the header.
static inline void ltrim(std::string &s) {
    s.erase(s.begin(),
            std::find_if(s.begin(), s.end(),
                         [](unsigned char ch){ return !std::isspace(ch); }));
}

static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](unsigned char ch){ return !std::isspace(ch); }).base(),
            s.end());
}

static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

// Splits a line into { instruction, args }
std::vector<std::string> parseLine(const std::string& line);



#endif // STRING_UTILS_H
