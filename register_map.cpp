#include "register_map.h"
#include <stdexcept> // For std::out_of_range

// Default constructor - Initializer list should use proper member names.
// Note: You can't call member functions here unless they are part of a helper function.
// This is because the object is still being constructed.
// Helper functions (like createRegisterNameMap64) are a good pattern for this.
RegisterMap::RegisterMap()
  : register_name_map_64_(),
    register_name_map_32_(),
    registers64_(),
    registers32_(),
    RegSeg_()
{
  // The maps are initialized as empty.
  // The helper functions below should be called in the init() function or from a factory function.
}

// Function to initialize the maps
bool RegisterMap::init() {
  static const std::map<std::string, Reg64> name_map_64 = {
    {"rax", RAX}, {"rbx", RBX}, {"rcx", RCX}, {"rdx", RDX},
    {"rsi", RSI}, {"rdi", RDI}, {"rbp", RBP}, {"rsp", RSP},
    {"r8", R8}, {"r9", R9}, {"r10", R10}, {"r11", R11},
    {"r12", R12}, {"r13", R13}, {"r14", R14}, {"r15", R15},
    {"rip", RIP}, {"rflags", RFLAGS}
  };
  register_name_map_64_ = name_map_64;

  static const std::map<std::string, Reg32> name_map_32 = {
    {"eax", EAX}, {"ebx", EBX}, {"ecx", ECX}, {"edx", EDX},
    {"esi", ESI}, {"edi", EDI}, {"ebp", EBP}, {"esp", ESP},
    {"eflags", EFLAGS}
  };
  register_name_map_32_ = name_map_32;

  // Resize the register vectors based on the enum sizes (if you have them defined)
  // For example: registers64_.resize(NUM_REGISTERS64, 0);
  registers64_.resize(NUM_REG64, 0);
  registers32_.resize(NUM_REG32, 0);

  return true;
}

uint64_t RegisterMap::get64(const std::string& reg_name) const {
  if (auto it = register_name_map_64_.find(reg_name); it != register_name_map_64_.end()) {
    return registers64_[static_cast<size_t>(it->second)];
  }
  throw std::out_of_range("Invalid 64-bit register name: " + reg_name);
}

void RegisterMap::set64(const std::string& reg_name, uint64_t value) {
  if (auto it = register_name_map_64_.find(reg_name); it != register_name_map_64_.end()) {
    registers64_[static_cast<size_t>(it->second)] = value;
    return;
  }
  throw std::out_of_range("Invalid 64-bit register name: " + reg_name);
}

uint64_t RegisterMap::get32(const std::string& reg_name) const {
  if (auto it = register_name_map_32_.find(reg_name); it != register_name_map_32_.end()) {
    return registers32_[static_cast<size_t>(it->second)];
  }
  throw std::out_of_range("Invalid 32-bit register name: " + reg_name);
}

void RegisterMap::set32(const std::string& reg_name, uint64_t value) {
  if (auto it = register_name_map_32_.find(reg_name); it != register_name_map_32_.end()) {
    registers32_[static_cast<size_t>(it->second)] = static_cast<uint32_t>(value);
    return;
  }
  throw std::out_of_range("Invalid 32-bit register name: " + reg_name);
}

const std::map<std::string, Reg64>& RegisterMap::getRegisterNameMap64() const {
  return register_name_map_64_;
}

const std::map<std::string, Reg32>& RegisterMap::getRegisterNameMap32() const {
  return register_name_map_32_;
}
