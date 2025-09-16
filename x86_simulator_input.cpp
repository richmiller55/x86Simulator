#include "x86_simulator.h"
#include "decoder.h" // For DecodedOperand and helper types if any
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm> // For std::remove_if, std::isspace
#include "CodeGenerator.h"

#include <stdexcept>
#include <string>
#include <vector>
#include <sstream>

namespace fs = std::filesystem;

// Implement the helper functions here if they are not part of a class
// and their declarations were put in X86Simulator.h
// Example:
std::vector<std::string> readLinesFromFile(const std::string& filePath) {
  std::vector<std::string> lines;
  std::ifstream file(filePath);

  if (file.is_open()) {
    std::string line;
    while (std::getline(file, line)) {
      lines.push_back(line);
    }
    file.close();
  } else {
    //    log(0, "Unable to open file: ", "ERROR", 0, __FILE__, __LINE__); 
  }
  return lines;
}



// Assuming your OperandType enum or similar is defined elsewhere
// For example:
// enum class OperandType {
//     REGISTER, IMMEDIATE, MEMORY, LABEL, UNKNOWN
// };

DecodedOperand X86Simulator::parse_operand(const std::string& operand_str) {
    DecodedOperand result;
    std::string trimmed_str = trim(operand_str);

    // 1. Check for Label
    if (!trimmed_str.empty() && trimmed_str.back() == ':') {
        std::string label_text = trimmed_str.substr(0, trimmed_str.size() - 1);
        auto it = symbolTable_.find(label_text);
        if (it != symbolTable_.end()) {
            result.text = label_text;
            result.value = it->second;
            result.type = OperandType::LABEL;
            return result;
        }
        // If the label is not in the symbol table, it's an error.
        throw std::out_of_range("Invalid label reference: " + label_text);
    }

    // 2. Check for Register
    try {
        // Try getting a 64-bit register
        register_map_.get64(trimmed_str);
        result.text = trimmed_str;
        result.type = OperandType::REGISTER;
        return result;
    } catch (const std::out_of_range&) {
        // Not a 64-bit register, try 32-bit
        try {
            register_map_.get32(trimmed_str);
            result.text = trimmed_str;
            result.type = OperandType::REGISTER;
            return result;
        } catch (const std::out_of_range&) {
            // Not a register, continue checking other types
        }
    }

    // 3. Check for Immediate Value (Hex or Decimal)
    try {
        // std::stoull handles decimal, hex (0x), and octal (0)
        result.value = std::stoull(trimmed_str, nullptr, 0);
        result.text = trimmed_str;
        result.type = OperandType::IMMEDIATE;
        return result;
    } catch (const std::exception&) {
        // Not an immediate value, continue checking
    }
    
    // 4. Check for Memory Operand (More complex parsing needed here)
    if (trimmed_str.size() >= 2 && trimmed_str.front() == '[' && trimmed_str.back() == ']') {
        // Add your logic for parsing memory operands here.
        // This is complex and might involve regex or more advanced parsing.
        // For now, let's treat it as a memory operand and set the text.
        result.text = trimmed_str;
        result.type = OperandType::MEMORY;
        return result;
    }

    // If none of the above, it's an unrecognized operand
    std::string output = "Unrecognized operand format: ";
    output += trimmed_str;
    log(session_id_, output, "WARNING", 0, __FILE__, __LINE__);
    result.type = OperandType::UNKNOWN_OPERAND_TYPE;
    return result;
}

bool X86Simulator::loadProgram(const std::string& filename) {
  memory_.reset();
  programLines_ = readLinesFromFile(filename);
  return !programLines_.empty(); // Or a more robust check for successful read.
}

std::vector<std::string> X86Simulator::parseLine(const std::string& line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

bool X86Simulator::firstPass() {
    address_t location_counter = 0;
    for (const std::string& line_raw : programLines_) {
        std::string line = trim(line_raw);
        if (line.empty() || line[0] == ';') {
            continue;
        }

        std::vector<std::string> tokens = parseLine(line);
        if (tokens.empty()) {
            continue;
        }

        std::string& mnemonic = tokens[0];
        std::transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        if (mnemonic.back() == ':') {
            std::string label = mnemonic.substr(0, mnemonic.size() - 1);
            symbolTable_[label] = location_counter;
        } else {
            if (mnemonic == "mov") {
                if (tokens.size() < 3) continue;
                std::string dest = tokens[1];
                std::transform(dest.begin(), dest.end(), dest.begin(),
                               [](unsigned char c){ return std::tolower(c); });
                if (dest.back() == ',') dest.pop_back();
                std::string src = tokens[2];
                std::transform(src.begin(), src.end(), src.begin(),
                               [](unsigned char c){ return std::tolower(c); });

                if (dest == "eax" || dest == "ecx") {
                    try {
                        std::stoul(src);
                        location_counter += 5; // Opcode (1) + Immediate (4)
                    } catch (const std::exception&) {
                        if (src == "eax" || src == "ebx" || src == "ecx" || src == "edx") {
                            location_counter += 2; // Opcode (1) + ModR/M (1)
                        }
                    }
                } else if (dest == "ebx" && src == "eax") {
                    location_counter += 2; // Opcode (1) + ModR/M (1)
                }
            } else if (mnemonic == "add") {
                if (tokens.size() < 3) continue;
                std::string dest = tokens[1];
                std::transform(dest.begin(), dest.end(), dest.begin(),
                               [](unsigned char c){ return std::tolower(c); });
                if (dest.back() == ',') dest.pop_back();
                std::string src = tokens[2];
                std::transform(src.begin(), src.end(), src.begin(),
                               [](unsigned char c){ return std::tolower(c); });
                location_counter += 2; // Opcode (1) + ModR/M (1)
            } else if (mnemonic == "inc") {
                if (tokens.size() < 2) continue;
                std::string dest = tokens[1];
                std::transform(dest.begin(), dest.end(), dest.begin(),
                               [](unsigned char c){ return std::tolower(c); });
                location_counter += 1; // e.g., INC ECX is one byte (0x41)
            } else if (mnemonic == "cmp") {
                if (tokens.size() < 3) continue;
                std::string dest = tokens[1];
                std::transform(dest.begin(), dest.end(), dest.begin(),
                               [](unsigned char c){ return std::tolower(c); });
                if (dest.back() == ',') dest.pop_back();
                std::string src = tokens[2];
                std::transform(src.begin(), src.end(), src.begin(),
                               [](unsigned char c){ return std::tolower(c); });
                location_counter += 3; // Opcode (1) + ModR/M (1) + Immediate (1)
            } else if (mnemonic == "jne") {
                location_counter += 2; // Opcode (1) + Relative Offset (1)
            } else if (mnemonic == "jmp") {
                location_counter += 5; // Opcode (1) + Relative Offset (4)
            }
        }
    }
    return true;
}

bool X86Simulator::secondPass() {
    CodeGenerator code_generator(memory_, symbolTable_);
    code_generator.generate_code(programLines_);
    program_size_in_bytes_ = memory_.text_segment_size;
    return true;
}

// Helper to remove leading/trailing whitespace
std::string X86Simulator::trim(const std::string& str) {
  size_t first = str.find_first_not_of(" \t\n\r");
  if (std::string::npos == first) {
    return "";
  }
  size_t last = str.find_last_not_of(" \t\n\r");
  return str.substr(first, (last - first + 1));
}
void X86Simulator::waitForInput() {
  ui_.waitForInput();
}
