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
#include "i_database_manager.h"
#include "decoder.h" // Include for DecodedInstruction and DecodedOperand

class UIManager;


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
  X86Simulator(IDatabaseManager& dbManager, int session_id, bool headless = false);
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
  bool get_CF() const;
  void set_CF(bool value);
  bool get_ZF() const;
  void set_ZF(bool value);
  bool get_SF() const;
  void set_SF(bool value);
  bool get_OF() const;
  void set_OF(bool value);
  int get_session_id() const;
  uint64_t getRegister(const std::string& register_name);
  void log(int session_id, const std::string& message,
	   const std::string& level,
	   uint64_t instruction_pointer,
	   const std::string& source_file,
	   int source_line);
  void updateDisplay(); // This function now implicitly uses the current RIP
  void waitForInput();
  uint64_t pop();
  void push(uint64_t value); 
  void update_rflags_in_register_map();
  RegisterMap& getRegisterMapForTesting() { return register_map_; }
  bool is_headless() const;
private:
  IDatabaseManager& dbManager_;
  RegisterMap register_map_;
  Memory memory_;
  address_t instructionPointer_ = 0;
  std::unique_ptr<UIManager> ui_;
  uint64_t rflags_;
  int session_id_;
  bool headless_;
  address_t program_size_in_bytes_; 

  std::vector<std::string> programLines_; // raw
  std::map<std::string, address_t> symbolTable_;
  std::string entryPointLabel_ = "_start"; // Default entry point
  void handleMov(const DecodedInstruction& decoded_instr);
  void handleAdd(const DecodedInstruction& decoded_instr);
  void handleJmp(const DecodedInstruction& decoded_instr);
  void handleJne(const DecodedInstruction& decoded_instr);
  void handleInc(const DecodedInstruction& decoded_instr);
  void handleCmp(const DecodedInstruction& decoded_instr);
  void handleInt(const DecodedInstruction& decoded_instr);
  void handleMul(const DecodedInstruction& decoded_instr);
  void handleDec(const DecodedInstruction& decoded_instr);
  void handleDiv(const DecodedInstruction& decoded_instr);
  void handleAnd(const DecodedInstruction& decoded_instr);
  void handleOr(const DecodedInstruction& decoded_instr);
  void handleXor(const DecodedInstruction& decoded_instr);
  void handleNot(const DecodedInstruction& decoded_instr);
  void handleSub(const DecodedInstruction& decoded_instr);
  void handleVaddps(const DecodedInstruction& decoded_instr);
  void handleVdivps(const DecodedInstruction& decoded_instr);
  void handleVmaxps(const DecodedInstruction& decoded_instr);
  void handleVpandn(const DecodedInstruction& decoded_instr);
  void handleVpand(const DecodedInstruction& decoded_instr);
  void handleVpmullw(const DecodedInstruction& decoded_instr);
  void handleVminps(const DecodedInstruction& decoded_instr);
  void handleVpxor(const DecodedInstruction& decoded_instr);
  void handleVrcpps(const DecodedInstruction& decoded_instr);
  void handleVsqrtps(const DecodedInstruction& decoded_instr);
  void handleVsubps(const DecodedInstruction& decoded_instr);
  void handleVpor(const DecodedInstruction& decoded_instr);
  std::vector<std::string> parseLine(const std::string& line);
  
};

#endif // X86SIMULATOR_H
