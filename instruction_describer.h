#ifndef INSTRUCTION_DESCRIBER_H
#define INSTRUCTION_DESCRIBER_H

#include <string>
#include "decoder.h"
#include "register_map.h"

class InstructionDescriber {
public:
    static std::string describe(const DecodedInstruction& instr, const RegisterMap& regs);
};

#endif // INSTRUCTION_DESCRIBER_H
