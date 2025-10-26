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
#include "decoder.h" 
#include "architecture.h"
#include "ir.h"
#include "pipeline.h"
#include "program_decoder.h"

#include "i_simulator.h"

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
bool is_number(const std::string& s);

class X86Simulator : public ISimulator {
#ifdef GOOGLE_TEST
friend class SimulatorCoreTest;
friend class IRExecutorTest;
#endif
public:
  // Constructor, other public methods
  X86Simulator(IDatabaseManager& db_manager, Memory& memory, int session_id, bool headless = false);
  ~X86Simulator();
  
  bool executeInstruction(const DecodedInstruction& decoded_instr);
  void runSingleInstruction();
  bool isRunning();
  void runProgram() override;
  bool loadProgram(const std::string& program_path) override;
  bool firstPass() override;
  bool secondPass() override;

  void accept(IRVisitor& visitor, const IRInstruction& instr) override;
  const char* get_stack_pointer_name() const override;
  const char* get_instruction_pointer_name() const override;

  void execute_ir_instruction(const IRInstruction& ir_instr) override;
  void dumpTextSegment(const std::string& filename);
  void dumpMemoryRange(const std::string& filename, address_t start_addr, size_t size);
  void dumpDataSegment(const std::string& filename);
  void dumpBssSegment(const std::string& filename);
  void dumpSymbolTable(const std::string& filename);
  // --- ISimulator Interface Implementation ---
  IRegisterMap& getRegisterMap() override { return *register_map_; }
  const IRegisterMap& getRegisterMap() const override { return *register_map_; }
  Memory& getMemory() override { return memory_; }
  const Memory& getMemory() const override { return memory_; }
  IDatabaseManager& getDatabaseManager() override { return db_manager_; }
  const Architecture& get_architecture() const override { return architecture_; }
  ProgramDecoder* getProgramDecoder() override { return program_decoder_.get(); }
  int get_session_id() const override { return session_id_; }

  bool get_CF() const override;
  void set_CF(bool value) override;
  bool get_ZF() const override;
  void set_ZF(bool value) override;
  bool get_SF() const override;
  void set_SF(bool value) override;
  bool get_OF() const override;
  void set_OF(bool value) override;
  bool get_DF() const;
  void set_DF(bool value);
  bool get_AF() const;
  void set_AF(bool value);
  bool get_PF() const override;
  void set_PF(bool val) override;
  
  uint64_t get_system_register(const std::string& name) override;
  void set_system_register(const std::string& name, uint64_t value) override;
  
  // --- I/O Handling ---
  void log_out(uint16_t port, uint64_t value);
  const std::vector<std::pair<uint16_t, uint64_t>>& get_out_log() const;
  bool is_headless() const;

#if defined(GOOGLE_TEST)
  RegisterMap& getRegisterMapForTesting() { return *register_map_; }
  Memory& getMemoryForTesting() { return memory_; }
#endif

private:
  // Private helper methods
  std::string trim(const std::string& str);

  // --- Member Variables ---
  IDatabaseManager& db_manager_;
  Memory& memory_;
  Architecture architecture_;
  std::unique_ptr<RegisterMap> register_map_;

  int session_id_;
  bool headless_;
    
  address_t instructionPointer_ = 0;
  address_t program_size_in_bytes_ = 0;

  std::unique_ptr<UIManager> ui_;
  std::unique_ptr<Pipeline> pipeline_;
  std::unique_ptr<ProgramDecoder> program_decoder_;
  std::map<std::string, address_t> symbolTable_;
  std::vector<std::pair<uint16_t, uint64_t>> out_log_;
  std::vector<std::string> programLines_; // raw
  std::string entryPointLabel_ = "_start"; // Default entry point

  
};

#endif // X86SIMULATOR_H
