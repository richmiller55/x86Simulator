#include "x86_simulator.h"


bool X86Simulator::executeInstruction(const std::string& instruction,
                                      const std::string& args) {
  // Convert instruction string to a normalized format (e.g., uppercase)
  std::string normalizedInstr = instruction;
  std::transform(normalizedInstr.begin(), normalizedInstr.end(),
                 normalizedInstr.begin(), ::toupper);

  // This is a simplified example. A real simulator might use
  // more sophisticated decoding or a jump table/map of function pointers.

  if (normalizedInstr == "MOV") {
    // Parse arguments: "AX, BX" -> dest="AX", src="BX"
    std::vector<std::string> argParts = parseArguments(args);
    // A new helper function needed I think we have this
    if (argParts.size() == 2) {
      handleMov(argParts[0], argParts[1]);
      return true;
    }
  } else if (normalizedInstr == "ADD") {
    std::vector<std::string> argParts = parseArguments(args);
    if (argParts.size() == 2) {
      handleAdd(argParts[0], argParts[1]);
      return true;
    }
  } else if (normalizedInstr == "JMP") {
    handleJmp(args); // JMP usually takes one argument: a label or address
    return true;
  } else if (normalizedInstr == "INC") { // <-- Added 'else'
    handleInc(args);
    return true;
  } else if (normalizedInstr == "CMP") { // <-- Added 'else'
    handleCmp(args);
    return true;
  } else if (normalizedInstr == "JNE") { // <-- Added 'else'
    handleJne(args);
    return true;
  } else { // <-- Added 'else' to handle the default case
    std::cerr << "Error: unsupported instruction: " << instruction << std::endl;
    return false;
  }
}

void X86Simulator::runNextInstruction() {
  if (instructionPointer_ < programInstructions_.size()) {
    const auto& instrData = programInstructions_[instructionPointer_];
    if (executeInstruction(instrData[0], instrData[1])) {
      instructionPointer_++; // Increment IP if instruction executed successfully (unless it was a jump)
      // Note: JMP would set the IP itself, so careful here to not double-increment
      // For now, assume a simple sequential flow unless a JMP explicitly changes it
    } else {
      // Handle error, maybe halt simulation
      std::cerr << "Simulation halted due to instruction execution error." << std::endl;
      instructionPointer_ = programInstructions_.size(); // Stop execution
    }
  } else {
    std::cout << "End of program reached." << std::endl;
  }
}

void X86Simulator::runProgram() {
  // Loop until instructionPointer_ goes out of bounds or a HALT instruction is encountered
  while (instructionPointer_ < programInstructions_.size()) {
    runNextInstruction();
    // Potentially add a delay here for step-by-step viewing
    // or call your displayRegistersWithDiff()
    displayRegistersWithDiff(); // Update display after each instruction
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

// You might want a method to fetch and execute the next instruction in sequence
//  void runNextInstruction();


bool X86Simulator::update(const std::string& name, uint64_t val) {
  bool updated = false;
  if (auto it = regs32_.find(name); it != regs32_.end()) {
    if (it->second.getValue() != val) { // Only update if value changed
      it->second.update(val);
      updated = true;
      // Trigger a display update for this specific register (more advanced)
    }
  }
  if (auto it = regs64_.find(name); it != regs64_.end()) {
    if (it->second.getValue() != val) { // Only update if value changed
      it->second.update(val);
      updated = true;
      // Trigger a display update for this specific register (more advanced)
    }
  }
  return updated;
}

