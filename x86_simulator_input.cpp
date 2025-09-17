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
    // First pass is now only for building the symbol table.
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
        }
        // Handle assembler directives
        else if (mnemonic == "section" || mnemonic == "global") {
            // These directives do not generate code and do not advance the location counter.
            // We can just ignore them for the purpose of address calculation.
            // A more advanced assembler might use 'section' to switch between .text, .data, etc.
            // and 'global' to adjust symbol visibility, but for now, we can skip.
            continue;
        }
        // The CodeGenerator will determine the actual location counter during the second pass.
        // To keep the symbol table correct, we must simulate the address calculation here.
        else {
            // To get the correct address for labels, we must calculate the size of each instruction.
            // We use a temporary CodeGenerator for this, as it has the sizing logic.
            // This is inefficient but ensures correctness by centralizing the logic.
            Memory temp_mem; // Create a dummy memory object for sizing
            CodeGenerator temp_gen(temp_mem, symbolTable_);
            std::vector<std::string> single_line_vec = {line_raw};
            size_t instruction_size = temp_gen.generate_code(single_line_vec);
            location_counter += instruction_size;
        }
    }
    return true;
}

bool X86Simulator::secondPass() {
    CodeGenerator code_generator(memory_, symbolTable_);
    // The code generator should return the size of the generated code.
    program_size_in_bytes_ = code_generator.generate_code(programLines_);
    memory_.text_segment_size = program_size_in_bytes_;

    // Set the initial instruction pointer (RIP) to the address of the entry point label.
    auto it = symbolTable_.find(entryPointLabel_);
    if (it != symbolTable_.end()) {
        register_map_.set64("rip", it->second);
    } else {
        log(session_id_, "Entry point label '" + entryPointLabel_ + "' not found. Defaulting to start of text segment.", "ERROR", 0, __FILE__, __LINE__);
        // Fallback to the start of the text segment if the label is not found
        register_map_.set64("rip", memory_.text_segment_start);
    }
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
