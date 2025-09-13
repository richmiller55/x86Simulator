#include "x86_simulator.h"
#include "decoder.h" // For DecodedOperand and helper types if any
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm> // For std::remove_if, std::isspace

#include <stdexcept>
#include <string>

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
  programLines_ = readLinesFromFile(filename);
  return !programLines_.empty(); // Or a more robust check for successful read.
}

bool X86Simulator::firstPass() {
  address_t location_counter = 0;
  for (const std::string& line_raw : programLines_) {
    std::string line = trim(line_raw); // Clean up whitespace

    if (line.empty() || line[0] == ';') { // Skip empty lines and comments
      continue;
    }
    std::vector<std::string> parsedLine = parseLine(line);
    // e.g., ["MOV", "EAX, 0x10"] or ["ADD", "EBX, [0x100]"]

    if (parsedLine.empty()) {
      std::string output = "Skipping malformed (no instruction): ";
      output += line;
      log(session_id_, output, "WARNING", 0, __FILE__, __LINE__); 
      continue;
    }

    std::string instruction_mnemonic = parsedLine[0];
    std::string arguments_str = (parsedLine.size() > 1) ? parsedLine[1] : ""; // Get the argument string

    if (!instruction_mnemonic.empty() && instruction_mnemonic.back() == ':') {
      std::string label_text = instruction_mnemonic.substr(0, instruction_mnemonic.size() - 1);
      symbolTable_.emplace( label_text, location_counter++);
    }
    else  {
      symbolTable_.emplace( instruction_mnemonic, location_counter++);
    }
  }
  return true;
}

bool X86Simulator::secondPass() {

  address_t current_text_address = memory_.text_segment_start;

  for (const std::string& line_raw : programLines_) {
    std::string line = trim(line_raw); // Clean up whitespace

    if (line.empty() || line[0] == ';') { // Skip empty lines and comments
      continue;
    }
    std::vector<std::string> parsedLine = parseLine(line);
    // e.g., ["MOV", "EAX, 0x10"] or ["ADD", "EBX, [0x100]"]

    if (parsedLine.empty()) {
      std::string output = "Skipping malformed (no instruction): ";
      output += line;
      log(session_id_, output, "WARNING", 0, __FILE__, __LINE__); 
      continue;
    }

    std::string instruction_mnemonic = parsedLine[0];
    std::string arguments_str = (parsedLine.size() > 1)
      ? parsedLine[1] : ""; // Get the argument string

    // 1. Encode the instruction ID
    Decoder& decoder = Decoder::getInstance();
    uint64_t instruction_id = decoder.getOpcode(instruction_mnemonic);
    try {
      memory_.write_text(current_text_address, instruction_id);
      current_text_address++;
    } catch (const std::runtime_error& e) {
      std::string output = "Memory error encoding instruction ID: ";
      output += e.what();
      output += " at address ";
      std::string result = std::to_string(current_text_address);
      output += result;
      log(session_id_, output, "WARNING", 0, __FILE__, __LINE__); 
      return false;
    }

    // 2. Parse and encode arguments
    if (!arguments_str.empty()) {
      std::vector<std::string> raw_arguments =
	parseArguments(arguments_str);
      // Splits "EAX, 0x10" -> ["EAX", "0x10"]

      for (const std::string& raw_arg : raw_arguments) {
	DecodedOperand operand = parse_operand(trim(raw_arg));
	// Parse each individual argument

	uint64_t encoded_operand_value = 0; 
	// uint64_t type_encoded = static_cast<uint64_t>(operand.type) << 60;

	auto it = symbolTable_.find(operand.text);
	switch (operand.type) {
	case OperandType::IMMEDIATE:
	  encoded_operand_value = operand.value;
	  break;
	case OperandType::REGISTER:
	  encoded_operand_value = operand.value;
	  // to do see if we need to encode the text to it's value
	  break;
	case OperandType::MEMORY:
	  encoded_operand_value = operand.value;
	  // Or `static_cast<uint64_t>(operand.reg)` if it was `[EAX]`
	  break;
	case OperandType::LABEL:
	  // Look up the label's address from the symbol table

	  // Assuming `ParsedOperand` stores the label's string
	  if (it != symbolTable_.end()) {
	    encoded_operand_value = it->second; // The address
	  } else {
	    // This case should not happen if first pass logic is correct,
	    // but good practice to handle it.
	    std::string output = "Label not found during second pass: ";
	    output += operand.text;
	    log(session_id_, output, "ERROR", 0, __FILE__, __LINE__);
	    return false;
	  }
	  break;
     
	case OperandType::UNKNOWN_OPERAND_TYPE:
	default:
	  std::string output = "Unrecognized operand type for: ";
	  std::string result = "";
	  for (int num : raw_arg) {
	    result += std::to_string(num);
	  }
	  output += result;
	  log(session_id_, output, "WARNING", 0, __FILE__, __LINE__); 
	  return false;
	}

	// Write the encoded operand value to the text segment
	try {
	  memory_.write_text(current_text_address, encoded_operand_value);
	  current_text_address++;
	} catch (const std::runtime_error& e) {
	  std::string output = "Memory error encoding operand: ";
	  output += e.what();
	  output += " at address ";
	  output += current_text_address;
	  log(session_id_, output, "WARNING", 0, __FILE__, __LINE__); 
	  return false;
	}
      }
    }
  }
  program_size_in_bytes_ = current_text_address - memory_.text_segment_start;
  memory_.text_segment_size = program_size_in_bytes_;
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
