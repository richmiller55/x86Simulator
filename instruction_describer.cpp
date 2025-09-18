#include "instruction_describer.h"
#include <sstream>

std::string InstructionDescriber::describe(const DecodedInstruction& instr, const RegisterMap& regs) {
    std::stringstream ss;

    // Add more detailed descriptions based on the mnemonic
    if (instr.mnemonic == "mov") {
        if (instr.operands.size() == 2) {
            const auto& dest = instr.operands[0];
            const auto& src = instr.operands[1];
            ss << "Moves the value from " << src.text << " to " << dest.text << ".";
        }
    } else if (instr.mnemonic == "add") {
        if (instr.operands.size() == 2) {
            const auto& dest = instr.operands[0];
            const auto& src = instr.operands[1];
            ss << "Adds the value of " << src.text << " to " << dest.text << " and stores the result in " << dest.text << ".";
        }
    } else if (instr.mnemonic == "jmp") {
        if (instr.operands.size() == 1) {
            const auto& target = instr.operands[0];
            ss << "Unconditionally jumps to the address " << target.text << ".";
        }
    } else if (instr.mnemonic == "nop") {
        ss << "No operation. This instruction does nothing.";
    } else if (instr.mnemonic == "ret") {
        ss << "Returns from the current procedure.";
    } else if (instr.mnemonic == "push") {
        if (instr.operands.size() == 1) {
            ss << "Pushes the value of " << instr.operands[0].text << " onto the stack.";
        }
    } else if (instr.mnemonic == "pop") {
        if (instr.operands.size() == 1) {
            ss << "Pops a value from the stack into " << instr.operands[0].text << ".";
        }
    }
    // Default case for other instructions
    else {
        ss << "Mnemonic: " << instr.mnemonic << ". No detailed description available yet.";
    }

    return ss.str();
}
