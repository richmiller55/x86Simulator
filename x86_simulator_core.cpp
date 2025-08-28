#include "x86_simulator.h"

// Constructor with DatabaseManager injection
X86Simulator::X86Simulator(DatabaseManager& dbManager)
    : dbManager_(dbManager), // Initialize the new member
      regs32_(Registers32),
      regs64_(Registers64),
      memory_(),
      rflags_(0) {
  rflags_ |= (1ULL << RFLAGS_ALWAYS_SET_BIT_1);
}
