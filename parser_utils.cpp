#include "parser_utils.h" // Include your header

#include <algorithm> // For std::transform
#include <charconv> // For std::from_chars (C++17+) for robust string-to-number conversion

// Definition of normalizeInstruction
std::string normalizeInstruction(const std::string& instr) {
    std::string normalized = instr;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::toupper);
    return normalized;
}

// Definition of parseImmediateValue (using C++17 std::from_chars for robustness)
std::optional<uint64_t> parseImmediateValue(const std::string& str) {
    std::string trimmedStr = str;
    trim(trimmedStr); // Use your trim function

    uint64_t value;
    const char* start = trimmedStr.data();
    const char* end = start + trimmedStr.size();
    std::from_chars_result result;

    if (trimmedStr.rfind("0x", 0) == 0 || trimmedStr.rfind("0X", 0) == 0) { // Hexadecimal
        start += 2; // Skip "0x"
        result = std::from_chars(start, end, value, 16);
    } else { // Decimal (default)
        result = std::from_chars(start, end, value, 10);
    }

    if (result.ec == std::errc() && result.ptr == end) {
        return value; // Parsing successful
    } else {
        return std::nullopt; // Parsing failed or partial
    }
}

// Definition of isRegisterName (simplified for demonstration)
bool isRegisterName(const std::string& operand) {
    // A simplified check. For a real simulator, you would ideally pass a
    // reference to your RegisterMap and check against actual known registers.
    std::string normalizedOperand = operand;
    std::transform(normalizedOperand.begin(), normalizedOperand.end(), normalizedOperand.begin(), ::tolower);

    // Common prefixes for x86 registers
    if (normalizedOperand.length() >= 2 &&
        (normalizedOperand[0] == 'r' || normalizedOperand[0] == 'e' ||
         normalizedOperand[0] == 'a' || normalizedOperand[0] == 'b' ||
         normalizedOperand[0] == 'c' || normalizedOperand[0] == 'd' ||
         normalizedOperand[0] == 's' || normalizedOperand[0] == 'i' ||
         normalizedOperand[0] == 'p' || normalizedOperand[0] == 'f' ||
         normalizedOperand[0] == 'g' || normalizedOperand[0] == 'x')) { // Add more if needed
        return true;
    }
    // Very rudimentary check. A lookup in RegisterMap is much better.
    return false;
}

// Example helper to parse arguments like "AX, BX" into {"AX", "BX"}
// You might put this in simulator_utils.h/.cpp
std::vector<std::string> parseArguments(const std::string& argString) {

    std::vector<std::string> args;
    std::stringstream ss(argString);
    std::string token;
    while (std::getline(ss, token, ',')) {
        trim(token); // Use your existing trim function
        if (!token.empty()) {
            args.push_back(token);
        }
    }
    return args;
}
bool parse_label(const std::string& operand_str) {
  if (operand_str.size() >= 2  && operand_str.back() == ':') {
    return true;
  }
    else return false;
  }

