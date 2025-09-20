#include "parser_utils.h" // Include your header
#include "decoder.h" 
#include "operand_types.h" 

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

DecodedOperand parse_operand(const std::string& operand_str, const RegisterMap& regs, const std::map<std::string, address_t>& symbols) {
    DecodedOperand result;
    std::string trimmed_str = operand_str;
    trim(trimmed_str); // Assumes a global or accessible trim function

    // 1. Check for Label
    if (!trimmed_str.empty() && trimmed_str.back() == ':') {
        std::string label_text = trimmed_str.substr(0, trimmed_str.size() - 1);
        auto it = symbols.find(label_text);
        if (it != symbols.end()) {
            result.text = label_text;
            result.value = it->second;
            result.type = OperandType::LABEL;
            return result;
        }
        throw std::out_of_range("Invalid label reference: " + label_text);
    }

    // 2. Check for Register
    try {
        regs.get64(trimmed_str);
        result.text = trimmed_str;
        result.type = OperandType::REGISTER;
        return result;
    } catch (const std::out_of_range&) {
        try {
            regs.get32(trimmed_str);
            result.text = trimmed_str;
            result.type = OperandType::REGISTER;
            return result;
        } catch (const std::out_of_range&) { /* Not a register */ }
    }

    // 3. Check for Immediate Value
    try {
        result.value = std::stoull(trimmed_str, nullptr, 0);
        result.text = trimmed_str;
        result.type = OperandType::IMMEDIATE;
        return result;
    } catch (const std::exception&) { /* Not an immediate */ }
    
    // 4. Check for Memory Operand
    if (trimmed_str.size() >= 2 && trimmed_str.front() == '[' && trimmed_str.back() == ']') {
        result.text = trimmed_str;
        result.type = OperandType::MEMORY;
        return result;
    }

    result.type = OperandType::UNKNOWN_OPERAND_TYPE;
    return result;
}
