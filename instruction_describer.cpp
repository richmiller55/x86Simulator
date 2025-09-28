#include "instruction_describer.h"
#include <sstream>
#include <algorithm>

std::string InstructionDescriber::describe(const DecodedInstruction& instr, const RegisterMap& regs,
                                           const std::map<std::string, address_t>* symbol_table) {
    std::stringstream ss;
    std::string mnemonic = instr.mnemonic;
    std::transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(), ::tolower);

    // Create a reverse map for address-to-label lookup for convenience
    std::map<address_t, std::string> address_to_label;
    if (symbol_table) {
        for (const auto& pair : *symbol_table) {
            address_to_label[pair.second] = pair.first;
        }
    }

    auto get_target_text = [&](const DecodedOperand& operand) -> std::string {
        if (operand.type == OperandType::IMMEDIATE || operand.type == OperandType::LABEL) {
            auto it = address_to_label.find(operand.value);
            if (it != address_to_label.end()) {
                return it->second; // Return label
            }
        }
        return operand.text; // Return original text (address)
    };

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
    } else if (mnemonic == "cmp") {
        if (instr.operands.size() == 2) {
            const auto& op1 = instr.operands[0];
            const auto& op2 = instr.operands[1];
            ss << "Compares " << op1.text << " and " << op2.text << " and sets the status flags (ZF, SF, OF, CF) accordingly. Does not modify the operands.";
        }
    } else if (mnemonic == "jmp") {
        if (instr.operands.size() == 1) {
            const auto& target = instr.operands[0];
            ss << "Unconditionally jumps to " << get_target_text(target) << ".";
        }
    } else if (mnemonic == "call") {
        if (instr.operands.size() == 1) {
            const auto& target = instr.operands[0];
            ss << "Pushes the return address onto the stack and jumps to " << get_target_text(target) << ".";
        }

    } else if (mnemonic == "je") {
        if (instr.operands.size() == 1) {
            const auto& target = instr.operands[0];
            ss << "Jumps to " << get_target_text(target) << " if the Zero Flag (ZF) is set.";
        }
    } else if (mnemonic == "jl") {
        if (instr.operands.size() == 1) {
            const auto& target = instr.operands[0];
            ss << "Jumps to " << get_target_text(target) << " if SF != OF.";
        }
    } else if (mnemonic == "jae") {
        if (instr.operands.size() == 1) {
            const auto& target = instr.operands[0];
            ss << "Jumps to " << get_target_text(target) << " if the Carry Flag (CF) is not set.";
        }
    } else if (mnemonic == "jb") {
        if (instr.operands.size() == 1) {
            const auto& target = instr.operands[0];
            ss << "Jumps to " << get_target_text(target) << " if the Carry Flag (CF) is set.";
        }
    } else if (mnemonic == "jbe") {
        if (instr.operands.size() == 1) {
            const auto& target = instr.operands[0];
            ss << "Jumps to " << get_target_text(target) << " if CF is set or ZF is set.";
        }
    } else if (mnemonic == "js") {
        if (instr.operands.size() == 1) {
            const auto& target = instr.operands[0];
            ss << "Jumps to " << get_target_text(target) << " if the Sign Flag (SF) is set.";
        }
    } else if (mnemonic == "jns") {
        if (instr.operands.size() == 1) {
            const auto& target = instr.operands[0];
            ss << "Jumps to " << get_target_text(target) << " if the Sign Flag (SF) is not set.";
        }
    } else if (mnemonic == "jo") {
        if (instr.operands.size() == 1) {
            const auto& target = instr.operands[0];
            ss << "Jumps to " << get_target_text(target) << " if the Overflow Flag (OF) is set.";
        }
    } else if (mnemonic == "jno") {
        if (instr.operands.size() == 1) {
            const auto& target = instr.operands[0];
            ss << "Jumps to " << get_target_text(target) << " if the Overflow Flag (OF) is not set.";
        }
    } else if (mnemonic == "jge") {
        if (instr.operands.size() == 1) {
            const auto& target = instr.operands[0];
            ss << "Jumps to " << get_target_text(target) << " if SF == OF.";
        }
    } else if (mnemonic == "jle") {
        if (instr.operands.size() == 1) {
            const auto& target = instr.operands[0];
            ss << "Jumps to " << get_target_text(target) << " if ZF is set, or if SF != OF.";
        }
    } else if (mnemonic == "jg") {
        if (instr.operands.size() == 1) {
            const auto& target = instr.operands[0];
            ss << "Jumps to " << get_target_text(target) << " if ZF is 0 and SF == OF.";
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
    } else if (mnemonic == "shl") {
        if (instr.operands.size() == 2) {
            const auto& dest = instr.operands[0];
            const auto& count = instr.operands[1];
            ss << "Shifts the bits in " << dest.text << " to the left by " << count.text << " positions. "
               << "The last bit shifted out is placed in the Carry Flag (CF).";
        }
    } else if (mnemonic == "shr") {
        if (instr.operands.size() == 2) {
            const auto& dest = instr.operands[0];
            const auto& count = instr.operands[1];
            ss << "Shifts the bits in " << dest.text << " to the right by " << count.text << " positions. "
               << "The last bit shifted out is placed in the Carry Flag (CF).";
        }
    } else if (mnemonic == "sar") {
        if (instr.operands.size() == 2) {
            const auto& dest = instr.operands[0];
            const auto& count = instr.operands[1];
            ss << "Performs a signed right shift on " << dest.text << " by " << count.text << " positions, preserving the sign bit. "
               << "The last bit shifted out is placed in the Carry Flag (CF).";
        }
    } else if (mnemonic == "rol") {
        if (instr.operands.size() == 2) {
            const auto& dest = instr.operands[0];
            const auto& count = instr.operands[1];
            ss << "Rotates the bits in " << dest.text << " to the left by " << count.text << " positions. "
               << "The bit rotated out of the MSB is moved to the LSB and also copied to the Carry Flag (CF).";
        }
    } else if (mnemonic == "ror") {
        if (instr.operands.size() == 2) {
            const auto& dest = instr.operands[0];
            const auto& count = instr.operands[1];
            ss << "Rotates the bits in " << dest.text << " to the right by " << count.text << " positions. "
               << "The bit rotated out of the LSB is moved to the MSB and also copied to the Carry Flag (CF).";
        }
    } else if (mnemonic == "lea") {
        if (instr.operands.size() == 2) {
            const auto& dest = instr.operands[0];
            const auto& src = instr.operands[1];
            ss << "Computes the effective address of the source operand " << src.text 
               << " and stores it in the destination register " << dest.text << ".";
        }
    } else if (mnemonic == "xchg") {
        if (instr.operands.size() == 2) {
            const auto& op1 = instr.operands[0];
            const auto& op2 = instr.operands[1];
            ss << "Exchanges the contents of " << op1.text << " and " << op2.text << ".";
        }
    } else if (mnemonic == "movsx") {
        if (instr.operands.size() == 2) {
            const auto& dest = instr.operands[0];
            const auto& src = instr.operands[1];
            ss << "Moves the value from " << src.text << " to " << dest.text 
               << " with sign-extension.";
        }
    } else if (mnemonic == "movzx") {
        if (instr.operands.size() == 2) {
            const auto& dest = instr.operands[0];
            const auto& src = instr.operands[1];
            ss << "Moves the value from " << src.text << " to " << dest.text 
               << " with zero-extension.";
        }
    } else if (mnemonic == "movsb") {
        ss << "Moves a byte from the location specified by RSI to the location specified by RDI. "
           << "RSI and RDI are then incremented or decremented based on the Direction Flag (DF).";
    } else if (mnemonic == "movsw") {
        ss << "Moves a word (2 bytes) from the location specified by RSI to the location specified by RDI. "
           << "RSI and RDI are then incremented or decremented by 2 based on the Direction Flag (DF).";
    } else if (mnemonic == "movsd") {
        ss << "Moves a doubleword (4 bytes) from the location specified by RSI to the location specified by RDI. "
           << "RSI and RDI are then incremented or decremented by 4 based on the Direction Flag (DF).";
    } else if (mnemonic == "imul") {
        if (instr.operands.size() == 1) {
            const auto& src = instr.operands[0];
            ss << "Performs a signed multiplication of EAX by " << src.text 
               << ". The 64-bit result is stored in EDX:EAX.";
        }
        // Other forms of IMUL can be described here
     
    } else if (mnemonic == "idiv") {
        if (instr.operands.size() == 1) {
            const auto& src = instr.operands[0];
            ss << "Performs a signed division of the 64-bit value in EDX:EAX by " << src.text
               << ". The quotient is stored in EAX and the remainder in EDX.";
        }
    
    } else if (mnemonic == "div") {
        if (instr.operands.size() == 1) {
            const auto& src = instr.operands[0];
            ss << "Performs an unsigned division of the 64-bit value in EDX:EAX by " << src.text
               << ". The quotient is stored in EAX and the remainder in EDX.";
        }
    }
    // Default case for other instructions
    else {
        ss << "Mnemonic: " << mnemonic << ". No detailed description available yet.";
    }

    return ss.str();
}
