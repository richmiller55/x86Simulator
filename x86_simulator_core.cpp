#include "x86_simulator.h"

X86Simulator::X86Simulator()
    : regs32_(Registers32), regs64_(Registers64),
      memory_(), rflags_(0) {
    rflags_ |= (1ULL << RFLAGS_ALWAYS_SET_BIT_1);
}

