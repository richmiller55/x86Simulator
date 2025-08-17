#include "x86_simulator.h"

X86Simulator::X86Simulator()
    : regs32_(Registers32), regs64_(Registers64),
      memory_()  { }

