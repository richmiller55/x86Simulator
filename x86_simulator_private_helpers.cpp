#include "x86_simulator.h"
#include "decoder.h"

// In x86_simulator_private_helpers.cpp or x86_simulator.cpp (depending on where they are defined)

void X86Simulator::handleMov(const DecodedInstruction& decoded_instr) {
    // We expect exactly two operands for a MOV instruction
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "Invalid number of operands for MOV", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& src_operand = decoded_instr.operands[1];

    uint64_t sourceValue = 0;
    if (src_operand.type == OperandType::REGISTER) {
        // If the source is a register, get its value
        sourceValue = getRegister(src_operand.text);
    } else {
        // Otherwise, it's an immediate value
        sourceValue = src_operand.value;
    }

    // Use the register_map_ to set the destination register's value
    try {
        // Assuming 32-bit move for now based on program1.asm
        register_map_.set32(dest_operand.text, sourceValue);
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid destination operand in MOV: " + dest_operand.text;
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

// In x86_simulator_private_helpers.cpp or x86_simulator.cpp

void X86Simulator::handleAdd(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "Invalid number of operands for ADD", "ERROR",
	    instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& src_operand = decoded_instr.operands[1];

    uint64_t destValue = 0;
    // Get the destination register value using its text representation from register_map_
    try {
        destValue = register_map_.get64(dest_operand.text);
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid destination operand in ADD: " + dest_operand.text;
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    // The source value is available directly from the DecodedOperand
    const uint64_t sourceValue = src_operand.value;

    // Perform addition and update destination using register_map_
    try {
        register_map_.set64(dest_operand.text, destValue + sourceValue);
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid destination operand in ADD: " + dest_operand.text;
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleSub(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "Invalid number of operands for SUB", "ERROR",
	    instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& src_operand = decoded_instr.operands[1];

    uint64_t destValue = 0;
    // Get the destination register value using its text representation from register_map_
    try {
        destValue = register_map_.get64(dest_operand.text);
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid destination operand in SUB: " + dest_operand.text;
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    // The source value is available directly from the DecodedOperand
    const uint64_t sourceValue = src_operand.value;

    // Perform subtraction and update destination using register_map_
    try {
        register_map_.set64(dest_operand.text, destValue - sourceValue);
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid destination operand in SUB: " + dest_operand.text;
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleJmp(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.empty()) {
        log(session_id_, "JMP instruction requires a target.", "ERROR",
	    instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& target_operand = decoded_instr.operands[0];
    const std::string& targetLabel = target_operand.text; // The label name is in the 'text' field

    // The target address is calculated during decoding and stored in the operand's value.
    if (target_operand.type == OperandType::IMMEDIATE) {
        // Jumps modify the instruction pointer directly
        address_t targetAddress = target_operand.value;
        register_map_.set64("rip", targetAddress);
    } else {
        std::string logMessage = "JMP target must be a valid label or address. Label '" + targetLabel + "' not found.";
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleJne(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.empty()) {
        log(session_id_, "JNE instruction requires a target.", "ERROR",
	    instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& target_operand = decoded_instr.operands[0];
    if (get_ZF() == false) { // If Zero Flag is not set, then jump
        // The target address is calculated during decoding and stored in the operand's value.
        address_t targetAddress = target_operand.value;
        register_map_.set64("rip", targetAddress);

    }
    // If ZF is set, do nothing and let the instruction pointer advance normally.
}

void X86Simulator::handleInc(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.empty()) {
        log(session_id_, "INC instruction requires an operand.", "ERROR",
            instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& operand = decoded_instr.operands[0];
    if (operand.type == OperandType::REGISTER) {
        try {
            uint32_t value = register_map_.get32(operand.text);
            uint32_t result = value + 1;
            register_map_.set32(operand.text, result);

            // Update RFLAGS. INC affects OF, SF, ZF, AF, PF, but not CF.
            set_ZF(result == 0);
            set_SF((result & 0x80000000) != 0);

            // Overflow for inc occurs when incrementing the max positive signed value (0x7FFFFFFF)
            set_OF(value == 0x7FFFFFFF);
            // AF and PF are not implemented yet.
        } catch (const std::out_of_range& e) {
            std::string logMessage = "Invalid register in INC: " + operand.text;
            log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
        }
    } else {
        log(session_id_, "INC only supports register operands currently.", "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleCmp(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "Invalid number of operands for CMP.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }
    
    const DecodedOperand& operand1 = decoded_instr.operands[0];
    const DecodedOperand& operand2 = decoded_instr.operands[1];

    uint32_t val1 = 0; // Assuming 32-bit comparison for now
    try {
        val1 = register_map_.get32(operand1.text);
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid destination operand in CMP: " + operand1.text;
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    uint32_t val2 = operand2.value; // Assuming immediate for now
    uint32_t result = val1 - val2;

    set_ZF(result == 0);
    set_SF((result & 0x80000000) != 0); // Check 32-bit sign

    // Set Carry Flag (CF): Set if there was a borrow (unsigned)
    set_CF(val1 < val2);

    // Set Overflow Flag (OF): Set on signed overflow.
    // Overflow occurs if signs of operands are different and sign of result is same as source.
    bool val1_sign = (val1 & 0x80000000);
    bool val2_sign = (val2 & 0x80000000);
    bool result_sign = (result & 0x80000000);
    set_OF(val1_sign != val2_sign && val2_sign == result_sign);
}