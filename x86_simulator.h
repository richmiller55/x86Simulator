#ifndef X86_SIMULATOR_H
#define X86_SIMULATOR_H

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <iomanip>
#include <cstdint>
#include <filesystem> 
#include <iomanip>    
#include <fstream>
#include <chrono> 
#include <thread> 
#include <regex> 


// Include other necessary headers for class members (like Memory, RegisterEnums, etc.)
#include "x86_registers.h" 
#include "register.h"     
#include "register_map.h" 
#include "string_utils.h" 
#include "parser_utils.h" 
#include "memory.h"
#include "register_enums.h"
#include "operand_types.h"
#include "operand.h"

// Forward declarations for helper functions if they are *not* member functions
// and if X86Simulator.h needs to know about them (e.g., if another part of the class
// directly calls them and doesn't #include the .cpp where they are defined).
// If these are only called *inside* X86Simulator::ReadProgram, they might not need to be here.
// But it's good practice to declare them where they are used.

std::vector<std::string> readLinesFromFile(const std::string& filename);
std::vector<std::string> parseLine(const std::string& line);
std::vector<std::string> parseArguments(const std::string& argument_str);
uint64_t hash_instruction(const std::string& instruction_mnemonic);
ParsedOperand parse_operand(const std::string& operand_str);
std::string trim(const std::string& str); // If trim is a standalone helper function

class X86Simulator {
public:
    // Constructor, other public methods
    X86Simulator();
  
  bool executeInstruction(const std::string& instruction, const std::string& args);
  void runNextInstruction();
  bool ReadProgram(const std::string& filename);
  void runProgram();
  bool update(const std::string& name, uint64_t val);
  void displayRegistersWithDiff();
  void displayRegistersControlled();
  std::string trim(const std::string& str) ;
private:
  RegisterMap regs32_;
  RegisterMap regs64_;
  Memory memory_;
  size_t instructionPointer_ = 0;
  std::vector<std::vector<std::string>> programInstructions_; // Store parsed program


  void handleMov(const std::string& dest, const std::string& src);
  void handleAdd(const std::string& dest, const std::string& src);
  void handleJmp(const std::string& targetLabel);
  
};

#endif // X86SIMULATOR_H
