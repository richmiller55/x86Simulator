#ifndef PARSER_UTILS_H
#define PARSER_UTILS_H

#include <string>
#include <vector>
#include <optional> // For std::optional (C++17+)
#include <cstdint> // For uint64_t
#include "decoder.h" // For DecodedOperand
#include "register_map.h" // For RegisterMap
#include <map> // for std::map

// Include your general string utilities, as these parsing functions might use them
#include "string_utils.h"

// Function to normalize instruction strings (e.g., convert to uppercase)
std::string normalizeInstruction(const std::string& instr);

// Function to try and parse a string as a 64-bit unsigned integer (supports hex and decimal)
std::optional<uint64_t> parseImmediateValue(const std::string& str);

std::vector<std::string> parseArguments(const std::string& argString);
DecodedOperand parse_operand(const std::string& operand_str, const RegisterMap& regs, const std::map<std::string, address_t>& symbols);
// You might also have functions here for:
// - Parsing memory addresses (e.g., "[rax+0x10]") - more complex
// - Resolving labels to addresses (if your parser needs to do this during an initial pass)

std::vector<std::string> parse_line(const std::string& line);

#endif // PARSER_UTILS_H
