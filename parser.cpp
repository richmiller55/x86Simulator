#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

// trim functions to strip leading/trailing spaces (in-place)
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
std::vector<std::string> parseLine(const std::string& line) {
    std::vector<std::string> parts;
    std::istringstream iss(line);
    
    // 1) Read the first word
    std::string instr;
    if (!(iss >> instr)) {
        // empty line or all whitespace
        return parts;
    }
    parts.push_back(instr);

    // 2) Read the rest of the line (may include leading space)
    std::string rest;
    std::getline(iss, rest);

    // 3) Trim whitespace and push as args
    trim(rest);
    parts.push_back(rest);

    return parts;
}
