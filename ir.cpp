#include "ir.h"

IRInstruction::IRInstruction()
    : opcode(IROpcode::Bubble),
      functional_unit_type(FunctionalUnitType::ALU) {}
