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
#include "DatabaseManager.h"

// Forward declarations for helper functions if they are *not* member functions



std::vector<std::string> readLinesFromFile(const std::string& filename);
std::vector<std::string> parseLine(const std::string& line);
std::vector<std::string> parseArguments(const std::string& argument_str);
uint64_t hash_instruction(const std::string& instruction_mnemonic);

bool parse_label(const std::string& line);

std::string trim(const std::string& str); // If trim is a standalone helper function
const uint64_t RFLAGS_CF_BIT = 0;   // Carry Flag
const uint64_t RFLAGS_PF_BIT = 2;   // Parity Flag
const uint64_t RFLAGS_AF_BIT = 4;   // Auxiliary Carry Flag
const uint64_t RFLAGS_ZF_BIT = 6;   // Zero Flag
const uint64_t RFLAGS_SF_BIT = 7;   // Sign Flag
const uint64_t RFLAGS_TF_BIT = 8;   // Trap Flag (for single-stepping)
const uint64_t RFLAGS_IF_BIT = 9;   // Interrupt Enable Flag
const uint64_t RFLAGS_DF_BIT = 10;  // Direction Flag
const uint64_t RFLAGS_OF_BIT = 11;  // Overflow Flag

// Reserved bits (always set or unset)
const uint64_t RFLAGS_ALWAYS_SET_BIT_1 = 1; // Reserved, always set to 1
const uint64_t RFLAGS_ALWAYS_UNSET_BIT_3 = 3; // Reserved, always unset
const uint64_t RFLAGS_ALWAYS_UNSET_BIT_5 = 5; // Reserved, always unset

class X86Simulator {
public:
    // Constructor, other public methods
    X86Simulator(DatabaseManager& dbManager);
  
  bool executeInstruction(const std::string& instruction, const std::string& args);
  void runNextInstruction();

  bool loadProgram(const std::string& filename);
  bool firstPass();
  bool secondPass();
  void runProgram();
  bool update(const std::string& name, uint64_t val);
  void displayRegistersWithDiff();
  void displayRegistersControlled();
  std::string trim(const std::string& str) ;
  ParsedOperand parse_operand(const std::string& operand_str);
  bool get_CF() const;
  void set_CF(bool value);
  bool get_ZF() const;
  void set_ZF(bool value);
  bool get_SF() const;
  void set_SF(bool value);

private:
  RegisterMap regs32_;
  RegisterMap regs64_;
  Memory memory_;
  address_t instructionPointer_ = 0;
  std::vector<std::vector<std::string>> programInstructions_;  // parsed
  std::vector<std::string> programLines_; // raw
  std::map<std::string, address_t> symbolTable_;
  uint64_t rflags_;
  DatabaseManager& dbManager_;
  void handleMov(const std::string& dest, const std::string& src);
  void handleAdd(const std::string& dest, const std::string& src);
  void handleJmp(const std::string& targetLabel);
  void handleJne(const std::string& targetLabel);
  void handleInc(const std::string& targetLabel);
  void handleCmp(const std::string& targetLabel);

  
};

#endif // X86SIMULATOR_H
