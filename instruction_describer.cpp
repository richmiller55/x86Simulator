#include "instruction_describer.h"
#include <sstream>

std::string InstructionDescriber::describe(const DecodedInstruction& instr, const RegisterMap& regs) {
    std::stringstream ss;
    std::string mnemonic = instr.mnemonic;
    std::transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(), ::tolower);
    // Add more detailed descriptions based on the mnemonic
    if (mnemonic == "mov") {
        if (instr.operands.size() == 2) {
            const auto& dest = instr.operands[0];
            const auto& src = instr.operands[1];
            ss << "Moves the value from " << src.text << " to " << dest.text << ".";
        }
    } else if (mnemonic == "add") {
        if (instr.operands.size() == 2) {
            const auto& dest = instr.operands[0];
            const auto& src = instr.operands[1];
            ss << "Adds the value of " << src.text << " to " << dest.text << " and stores the result in " << dest.text << ".";
        }
    } else if (mnemonic == "jmp") {
        if (instr.operands.size() == 1) {
            const auto& target = instr.operands[0];
            ss << "Unconditionally jumps to the address " << target.text << ".";
        }
    } else if (mnemonic == "nop") {
        ss << "No operation. This instruction does nothing.";
    } else if (mnemonic == "ret") {
        ss << "Returns from the current procedure.";
    } else if (mnemonic == "push") {
        if (instr.operands.size() == 1) {
            ss << "Pushes the value of " << instr.operands[0].text << " onto the stack.";
        }
    } else if (mnemonic == "pop") {
        if (instr.operands.size() == 1) {
            ss << "Pops a value from the stack into " << instr.operands[0].text << ".";
        }
    }
    // Default case for other instructions
    else {
        ss << "Mnemonic: " << mnemonic << ". No detailed description available yet.";
    }

    return ss.str();
}
