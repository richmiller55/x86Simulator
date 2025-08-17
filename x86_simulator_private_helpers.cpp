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


  void X86Simulator::handleJmp(const std::string& targetLabel) {
    // This is significantly more complex. You need to:
    // 1. Store labels and their corresponding line numbers during program loading.
    // 2. Look up the 'targetLabel' in your stored label map.
    // 3. Set the 'instructionPointer_' to the line number of the target label.
    std::cerr << "JMP instruction not yet fully implemented for labels." << std::endl;
    // For now, let's just assume it's an absolute line number for simplicity (not realistic for asm)
    // try {
    //     size_t lineNumber = std::stoul(targetLabel);
    //     if (lineNumber < programInstructions_.size()) {
    //         instructionPointer_ = lineNumber;
    //     } else {
    //         std::cerr << "Error: JMP target out of bounds: " << targetLabel << std::endl;
    //     }
    // } catch(...) {
    //      std::cerr << "Error: JMP target must be a label or address." << std::endl;
    // }

    // No instructionPointer_ increment here, as JMP sets the new IP.
  }

  // You'll need an Instruction Pointer (e.g., store the current instruction line number)
//  size_t instructionPointer_ = 0;
//  std::vector<std::vector<std::string>> programInstructions_; // Store parsed program

