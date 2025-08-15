#include "string_utils.h" // Include your header

// No need to redefine ltrim, rtrim, trim here due to static inline

// Definition of parseLine
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

