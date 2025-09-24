#include "instruction_describer.h"
#include <sstream>
#include <algorithm>

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
    } else if (mnemonic == "in") {
        if (instr.operands.size() == 2) {
            const auto& dest = instr.operands[0];
            const auto& src = instr.operands[1];
            ss << "Transfers data from port " << src.text << " to " << dest.text << ".";
        }
    } else if (mnemonic == "out") {
        if (instr.operands.size() == 2) {
            const auto& dest = instr.operands[0];
            const auto& src = instr.operands[1];
            ss << "Transfers data from " << src.text << " to port " << dest.text << ".";
        }
    } else if (mnemonic == "vaddps") {
        if (instr.operands.size() == 3) {
            const auto& dest = instr.operands[0];
            const auto& src1 = instr.operands[1];
            const auto& src2 = instr.operands[2];
            ss << "Adds packed single-precision floating-point values from " << src2.text << " to " << src1.text << " and stores the result in " << dest.text << ".";
        }
    } else if (mnemonic == "vdivps") {
        if (instr.operands.size() == 3) {
            const auto& dest = instr.operands[0];
            const auto& src1 = instr.operands[1];
            const auto& src2 = instr.operands[2];
            ss << "Divides packed single-precision floating-point values in " << src1.text << " by values in " << src2.text << " and stores the result in " << dest.text << ".";
        }
    } else if (mnemonic == "vmaxps") {
        if (instr.operands.size() == 3) {
            const auto& dest = instr.operands[0];
            const auto& src1 = instr.operands[1];
            const auto& src2 = instr.operands[2];
            ss << "Compares packed single-precision floating-point values in " << src1.text << " and " << src2.text << " and stores the maximum values in " << dest.text << ".";
        }
    } else if (mnemonic == "vpandn") {
        if (instr.operands.size() == 3) {
            const auto& dest = instr.operands[0];
            const auto& src1 = instr.operands[1];
            const auto& src2 = instr.operands[2];
            ss << "Performs a bitwise AND of the inverted " << src1.text << " with " << src2.text << " and stores the result in " << dest.text << ".";
        }
    } else if (mnemonic == "vpand") {
        if (instr.operands.size() == 3) {
            const auto& dest = instr.operands[0];
            const auto& src1 = instr.operands[1];
            const auto& src2 = instr.operands[2];
            ss << "Performs a bitwise AND of " << src1.text << " and " << src2.text << " and stores the result in " << dest.text << ".";
        }
    } else if (mnemonic == "vpmullw") {
        if (instr.operands.size() == 3) {
            const auto& dest = instr.operands[0];
            const auto& src1 = instr.operands[1];
            const auto& src2 = instr.operands[2];
            ss << "Multiplies packed 16-bit integers in " << src1.text << " and " << src2.text << ", storing the low 16 bits of the results in " << dest.text << ".";
        }
    } else if (mnemonic == "vminps") {
        if (instr.operands.size() == 3) {
            const auto& dest = instr.operands[0];
            const auto& src1 = instr.operands[1];
            const auto& src2 = instr.operands[2];
            ss << "Compares packed single-precision floating-point values in " << src1.text << " and " << src2.text << " and stores the minimum values in " << dest.text << ".";
        }
    } else if (mnemonic == "vmovups") {
        if (instr.operands.size() == 2) {
            const auto& op1 = instr.operands[0];
            const auto& op2 = instr.operands[1];
            ss << "Moves unaligned packed single-precision floating-point values from " << op2.text << " to " << op1.text << ".";
        } else {
            ss << "Moves unaligned packed single-precision floating-point values.";
        }
    } else if (mnemonic == "vpxor") {
        if (instr.operands.size() == 3) {
            const auto& dest = instr.operands[0];
            const auto& src1 = instr.operands[1];
            const auto& src2 = instr.operands[2];
            ss << "Performs a bitwise XOR of " << src1.text << " and " << src2.text << " and stores the result in " << dest.text << ".";
        }
    } else if (mnemonic == "vrcpps") {
        if (instr.operands.size() == 2) {
            const auto& dest = instr.operands[0];
            const auto& src = instr.operands[1];
            ss << "Computes approximate reciprocals of packed single-precision floating-point values in " << src.text << " and stores the results in " << dest.text << ".";
        }
    } else if (mnemonic == "vsqrtps") {
        if (instr.operands.size() == 2) {
            const auto& dest = instr.operands[0];
            const auto& src = instr.operands[1];
            ss << "Computes the square roots of packed single-precision floating-point values in " << src.text << " and stores the results in " << dest.text << ".";
        }
    } else if (mnemonic == "vsubps") {
        if (instr.operands.size() == 3) {
            const auto& dest = instr.operands[0];
            const auto& src1 = instr.operands[1];
            const auto& src2 = instr.operands[2];
            ss << "Subtracts packed single-precision floating-point values in " << src2.text << " from " << src1.text << " and stores the result in " << dest.text << ".";
        }
    } else if (mnemonic == "vpor") {
        if (instr.operands.size() == 3) {
            const auto& dest = instr.operands[0];
            const auto& src1 = instr.operands[1];
            const auto& src2 = instr.operands[2];
            ss << "Performs a bitwise OR of " << src1.text << " and " << src2.text << " and stores the result in " << dest.text << ".";
        }
    }
    // Default case for other instructions
    else {
        ss << "Mnemonic: " << mnemonic << ". No detailed description available yet.";
    }

    return ss.str();
}
