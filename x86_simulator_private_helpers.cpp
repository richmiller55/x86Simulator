#include "x86_simulator.h"

  // Helper functions for individual instructions
  void X86Simulator::handleMov(const std::string& dest, const std::string& src) {
    uint64_t sourceValue = 0;

    // 1. Determine source value (from register or immediate value)
    // This is where it gets complex: differentiating between register names and immediate numbers
    if (auto it = regs64_.find(src); it != regs64_.end()) {
      sourceValue = it->second.getValue();
    } else if (auto it = regs32_.find(src); it != regs32_.end()) {
      sourceValue = it->second.getValue();
    } else {
      // Try to parse as a number (immediate value)
      try {
	// Support hex (0x...) and decimal
	if (src.rfind("0x", 0) == 0 || src.rfind("0X", 0) == 0) { // C++20 starts_with
	  sourceValue = std::stoull(src.substr(2), nullptr, 16);
	} else {
	  sourceValue = std::stoull(src);
	}
      } catch (const std::invalid_argument& e) {
	std::cerr << "Error: Invalid source operand in MOV: " << src << std::endl;
	return;
      } catch (const std::out_of_range& e) {
	std::cerr << "Error: Source operand out of range in MOV: " << src << std::endl;
	return;
      }
    }

    // 2. Update destination register
    if (!update(dest, sourceValue)) { // Use the existing update method
      std::cerr << "Error: Invalid destination register in MOV: " << dest << std::endl;
    }
    // Increment instruction pointer (this happens after each instruction, usually)
  }
  void X86Simulator::handleAdd(const std::string& dest, const std::string& src) {
    uint64_t destValue = 0;
    uint64_t sourceValue = 0;

    // Get destination register value
    if (auto it = regs64_.find(dest); it != regs64_.end()) {
      destValue = it->second.getValue();
    } else if (auto it = regs32_.find(dest); it != regs32_.end()) {
      destValue = it->second.getValue();
    } else {
      std::cerr << "Error: Invalid destination operand in ADD: " << dest << std::endl;
      return;
    }

    // Determine source value (from register or immediate value)
    if (auto it = regs64_.find(src); it != regs64_.end()) {
      sourceValue = it->second.getValue();
    } else if (auto it = regs32_.find(src); it != regs32_.end()) {
      sourceValue = it->second.getValue();
    } else {
      // Try to parse as a number (immediate value)
      try {
	if (src.rfind("0x", 0) == 0 || src.rfind("0X", 0) == 0) {
	  sourceValue = std::stoull(src.substr(2), nullptr, 16);
	} else {
	  sourceValue = std::stoull(src);
	}
      } catch (const std::invalid_argument& e) {
	std::cerr << "Error: Invalid source operand in ADD: " << src << std::endl;
	return;
      } catch (const std::out_of_range& e) {
	std::cerr << "Error: Source operand out of range in ADD: " << src << std::endl;
	return;
      }
    }

    // Perform addition and update destination
    update(dest, destValue + sourceValue);
    // Increment instruction pointer
  }

void X86Simulator::handleJne(const std::string& targetLabel) { }
void X86Simulator::handleInc(const std::string& targetLabel) { }
void X86Simulator::handleCmp(const std::string& targetLabel) { }

void X86Simulator::handleJmp(const std::string& targetLabel) {
    // Look up the label explicitly using find() to avoid unintended insertion.
    auto it = symbolTable_.find(targetLabel);

    // Check if the label was found.
    if (it != symbolTable_.end()) {
        address_t lineNumber = it->second;
        // Optional: Add a check for a valid address if necessary.
        // For example, if 0 is an invalid address.
        if (lineNumber == 0) {
            std::cerr << "Error: JMP target label '" << targetLabel << "' resolves to an invalid address." << std::endl;
        } else {
            instructionPointer_ = lineNumber;
        }
    } else {
        // The label was not found in the symbol table.
        std::cerr << "Error: JMP target must be a valid label or address. Label '" << targetLabel << "' not found." << std::endl;
    }
}


