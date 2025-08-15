#ifndef PARSER_UTILS_H
#define PARSER_UTILS_H

#include <string>
#include <vector>
#include <optional> // For std::optional (C++17+)
#include <cstdint> // For uint64_t

// Include your general string utilities, as these parsing functions might use them
#include "string_utils.h"

// Function to normalize instruction strings (e.g., convert to uppercase)
std::string normalizeInstruction(const std::string& instr);

// Function to try and parse a string as a 64-bit unsigned integer (supports hex and decimal)
std::optional<uint64_t> parseImmediateValue(const std::string& str);

// Function to determine if a string is a register name (based on your RegisterMaps)
// This function would need access to the RegisterMaps, or take them as arguments.
// For now, let's declare a simple version that just checks if it starts with 'r', 'e', 's', etc.
// A more robust version would check against your actual RegisterMap.
bool isRegisterName(const std::string& operand);
std::vector<std::string> parseArguments(const std::string& argString);
// You might also have functions here for:
// - Parsing memory addresses (e.g., "[rax+0x10]") - more complex
// - Resolving labels to addresses (if your parser needs to do this during an initial pass)

#endif // PARSER_UTILS_H
