#include "x86_simulator.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm> // For std::remove_if, std::isspace

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
    std::cerr << "Error: Unable to open file: " << filePath << std::endl;
  }
  return lines;
}
uint64_t hash_instruction(const std::string& instruction_str) {
  if (instruction_str == "MOV") return 1;
  if (instruction_str == "ADD") return 2;
  if (instruction_str == "JMP") return 3;
  if (instruction_str == "POP") return 4;
  if (instruction_str == "PUSH") return 5;
  if (instruction_str == "OR") return 6;
  if (instruction_str == "XOR") return 7;
  if (instruction_str == "AND") return 8;
  if (instruction_str == "CMP") return 9;
  if (instruction_str == "JNE") return 10;
  if (instruction_str == "INC") return 11;
  // Add more instructions
  return 0; // Unknown instruction

}

ParsedOperand X86Simulator::parse_operand(const std::string& operand_str) {
  ParsedOperand result;
  result.type = OperandType::UNKNOWN_OPERAND_TYPE;
  result.value = 0;
  result.reg = RegisterEnum::UNKNOWN_REG;

    if (!operand_str.empty() && operand_str.back() == ':') {
      std::string label_text = operand_str.substr(0, operand_str.size() - 1);
      auto it = symbolTable_.find(label_text);
      if (it != symbolTable_.end()) {
        // 3. The label was found, so retrieve the address
	result.value = it->second;   // address_pointer
	result.type = OperandType::LABEL;
	return result;
      }
      // invalid label blow up

    }

  RegisterEnum found_reg = stringToRegister(operand_str);
  if (found_reg != RegisterEnum::UNKNOWN_REG) {
    result.type = OperandType::REGISTER;
    result.reg = found_reg;
    return result;
  }


  // 3. Check for Memory Operand (e.g., "[0x100]", "[EAX+4]", "[EBX+ESI*4]")
  // This is the most complex part and will require more sophisticated parsing.
  // A simplified approach for common patterns:
  if (operand_str.size() >= 2 && operand_str.front() == '[' && operand_str.back() == ']') {
    std::string inner_str = operand_str.substr(1, operand_str.size() - 2);

    // Simple direct address (e.g., "[0x100]")
    try {
      result.value = std::stoull(inner_str, nullptr, 0);
      result.type = OperandType::MEMORY;
      return result;
    } catch (const std::exception& e) {
      // Not a direct address, must be a more complex addressing mode
    }
    // labels:


  try {
    result.value = std::stoull(operand_str, nullptr, 0);
    // Handles decimal, hex (0x prefix), octal (0 prefix)
    result.type = OperandType::IMMEDIATE;
    return result;
  } catch (const std::exception& e) {
    // If it's not a simple integer, it might be a memory address or invalid.
    // We'll proceed to check for memory addresses.
  }

    // More complex patterns require more parsing logic
    // This is where regular expressions or manual string splitting will be essential.
    // For instance, parsing "EBX+ESI*4+0x10" would involve:
    // 1. Finding "EBX" -> base register
    // 2. Finding "ESI" -> index register
    // 3. Finding "*4" -> scale factor
    // 4. Finding "0x10" -> displacement

    std::cerr << "Warning: Could not parse complex memory operand: " << operand_str << std::endl;
    result.type = OperandType::UNKNOWN_OPERAND_TYPE; // Failed to parse
    return result;
  }

  // If none of the above, it's an unrecognized operand format
  std::cerr << "Warning: Unrecognized operand format: " << operand_str << std::endl;
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
            std::cerr << "Warning: Skipping malformed (no instruction): " <<
	      line << std::endl;
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
            std::cerr << "Warning: Skipping malformed (no instruction): " <<
	      line << std::endl;
            continue;
        }

        std::string instruction_mnemonic = parsedLine[0];
        std::string arguments_str = (parsedLine.size() > 1)
	  ? parsedLine[1] : ""; // Get the argument string

             // 1. Encode the instruction ID
        uint64_t instruction_id = hash_instruction(instruction_mnemonic);
        try {
            memory_.write_text(current_text_address, instruction_id);
            current_text_address++;
        } catch (const std::runtime_error& e) {
            std::cerr << "Memory error encoding instruction ID: " << e.what() << " at address " << current_text_address << std::endl;
            return false;
        }

        // 2. Parse and encode arguments
        if (!arguments_str.empty()) {
            std::vector<std::string> raw_arguments =
	      parseArguments(arguments_str);
	    // Splits "EAX, 0x10" -> ["EAX", "0x10"]

            for (const std::string& raw_arg : raw_arguments) {
                ParsedOperand operand = parse_operand(trim(raw_arg));
		// Parse each individual argument

                uint64_t encoded_operand_value = 0; // The value to write to memory

                switch (operand.type) {
                    case OperandType::IMMEDIATE:
                        encoded_operand_value = operand.value;

                        break;
                    case OperandType::REGISTER:
                        // Store the enum value for the register.
                        // When executing, the simulator will know to look up this ID in its register file.
                        encoded_operand_value = static_cast<uint64_t>(operand.reg);

                        break;
                    case OperandType::MEMORY:
                        // For a simple memory address, store the address.
                        // For complex modes (like [EAX+4]), you'd need to encode base, index, scale, displacement.
                        // For now, let's assume `value` holds a direct address like [0x100] or the base register ID.
                        encoded_operand_value = operand.value; // Or `static_cast<uint64_t>(operand.reg)` if it was `[EAX]`
                        break;
		case OperandType::LABEL:

		  break;
		  
                    case OperandType::UNKNOWN_OPERAND_TYPE:
                    default:
                        std::cerr << "Error: Unrecognized operand type for: " << raw_arg << std::endl;
                        return false;
                }

                // Write the encoded operand value to the text segment
                try {
                    memory_.write_text(current_text_address, encoded_operand_value);
                    current_text_address++;
                } catch (const std::runtime_error& e) {
                    std::cerr << "Memory error encoding operand: " << e.what() << " at address " << current_text_address << std::endl;
                    return false;
                }
            }
        }
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
