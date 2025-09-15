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

#include "register_map.h" 
#include "string_utils.h" 
#include "parser_utils.h" 
#include "memory.h"
#include "register_enums.h"
#include "register_map.h"
#include "operand_types.h"
#include "operand.h"
#include "DatabaseManager.h"
#include "ui_manager.h"


class DecodedInstruction;
class DecodedOperand;
// Forward declarations for helper functions if they are *not* member functions

std::vector<std::string> readLinesFromFile(const std::string& filename);
std::vector<std::string> parseArguments(const std::string& argument_str);

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
  ~X86Simulator();
  
  void init(const std::string& program_name);
  bool executeInstruction(const DecodedInstruction& decoded_instr);
  void runSingleInstruction();
  bool isRunning();
  bool loadProgram(const std::string& filename);
  bool firstPass();
  bool secondPass();
  void runProgram();
  void dumpTextSegment(const std::string& filename);

  void displayRegistersWithDiff();
  void displayRegistersControlled();
  std::string trim(const std::string& str) ;
  DecodedOperand parse_operand(const std::string& operand_str);
  bool get_CF() const;
  void set_CF(bool value);
  bool get_ZF() const;
  void set_ZF(bool value);
  bool get_SF() const;
  void set_SF(bool value);
  int get_session_id() const;
  void set_session_id(int session_id);
  uint64_t getRegister(const std::string& register_name);
  void log(int session_id, const std::string& message,
	   const std::string& level,
	   uint64_t instruction_pointer,
	   const std::string& source_file,
	   int source_line);
  void updateDisplay();
  void waitForInput();
  uint64_t pop();
  void push(uint64_t value); 
private:
  DatabaseManager& dbManager_;
  RegisterMap register_map_;
  Memory memory_;
  address_t instructionPointer_ = 0;
  UIManager ui_;
  uint64_t rflags_;
  int session_id_;
  address_t program_size_in_bytes_; 

  std::vector<std::string> programLines_; // raw
  std::map<std::string, address_t> symbolTable_;
  void handleMov(const DecodedInstruction& decoded_instr);
  void handleAdd(const DecodedInstruction& decoded_instr);
  void handleJmp(const DecodedInstruction& decoded_instr);
  void handleJne(const DecodedInstruction& decoded_instr);
  void handleInc(const DecodedInstruction& decoded_instr);
  void handleCmp(const DecodedInstruction& decoded_instr);
  void handleSub(const DecodedInstruction& decoded_instr);
  std::vector<std::string> parseLine(const std::string& line);
  
};

#endif // X86SIMULATOR_H
