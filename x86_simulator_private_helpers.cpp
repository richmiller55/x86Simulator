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

    if (dest_operand.type != OperandType::REGISTER || src_operand.type != OperandType::REGISTER) {
        log(session_id_, "ADD instruction requires register operands.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    try {
        uint32_t destValue = register_map_.get32(dest_operand.text);
        uint32_t sourceValue = register_map_.get32(src_operand.text);
        uint32_t result = destValue + sourceValue;
        register_map_.set32(dest_operand.text, result);

        set_ZF(result == 0);
        set_SF((result & 0x80000000) != 0);
        set_CF(result < destValue); 
        bool dest_sign = (destValue & 0x80000000);
        bool src_sign = (sourceValue & 0x80000000);
        bool result_sign = (result & 0x80000000);
        set_OF((dest_sign == src_sign) && (dest_sign != result_sign));

    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid register in ADD: " + std::string(e.what());
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

    uint32_t val2 = 0;
    if (operand2.type == OperandType::REGISTER) {
        try {
            val2 = register_map_.get32(operand2.text);
        } catch (const std::out_of_range& e) {
            std::string logMessage = "Invalid source operand in CMP: " + operand2.text;
            log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
            return;
        }
    } else { // Assuming immediate
        val2 = operand2.value;
    }

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

void X86Simulator::handleInt(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.empty() || decoded_instr.operands[0].type != OperandType::IMMEDIATE) {
        log(session_id_, "INT instruction requires an immediate operand.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    uint8_t interrupt_vector = decoded_instr.operands[0].value;

    if (interrupt_vector == 0x80) { // Linux syscall
        uint32_t syscall_num = register_map_.get32("eax");
        if (syscall_num == 1) { // sys_exit
            uint32_t exit_code = register_map_.get32("ebx");
            std::string logMessage = "Program exited with code: " + std::to_string(exit_code);
            log(session_id_, logMessage, "INFO", instructionPointer_, __FILE__, __LINE__);
            
            // Here you would set a flag to stop the main simulation loop.
            // For now, we'll just log it. The loop will stop on the next iteration
            // because we will advance RIP past the end of the program.
        } else {
            log(session_id_, "Unsupported syscall: " + std::to_string(syscall_num), "WARNING", instructionPointer_, __FILE__, __LINE__);
        }
    }
}

void X86Simulator::handleMul(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 1) {
        log(session_id_, "MUL instruction requires one operand.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& src_operand = decoded_instr.operands[0];
    uint32_t src_val = 0;

    if (src_operand.type == OperandType::REGISTER) {
        try {
            src_val = register_map_.get32(src_operand.text);
        } catch (const std::out_of_range& e) {
            log(session_id_, "Invalid register in MUL: " + src_operand.text, "ERROR", instructionPointer_, __FILE__, __LINE__);
            return;
        }
    } else {
        log(session_id_, "MUL only supports register operands currently.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    uint64_t val_eax = register_map_.get32("eax");
    uint64_t result = val_eax * static_cast<uint64_t>(src_val);

    register_map_.set32("eax", static_cast<uint32_t>(result & 0xFFFFFFFF));
    register_map_.set32("edx", static_cast<uint32_t>(result >> 32));

    set_CF(register_map_.get32("edx") != 0);
    set_OF(register_map_.get32("edx") != 0);
}

void X86Simulator::handleDec(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.empty()) {
        log(session_id_, "DEC instruction requires an operand.", "ERROR",
            instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& operand = decoded_instr.operands[0];
    if (operand.type == OperandType::REGISTER) {
        try {
            uint32_t value = register_map_.get32(operand.text);
            uint32_t result = value - 1;
            register_map_.set32(operand.text, result);

            // Update RFLAGS. DEC affects OF, SF, ZF, AF, PF, but not CF.
            set_ZF(result == 0);
            set_SF((result & 0x80000000) != 0);

            // Overflow for dec occurs when decrementing the min negative signed value (0x80000000)
            set_OF(value == 0x80000000);
        } catch (const std::out_of_range& e) {
            log(session_id_, "Invalid register in DEC: " + operand.text, "ERROR", instructionPointer_, __FILE__, __LINE__);
        }
    } else {
        log(session_id_, "DEC only supports register operands currently.", "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleDiv(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 1) {
        log(session_id_, "DIV instruction requires one operand.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& divisor_operand = decoded_instr.operands[0];
    uint32_t divisor = 0;

    if (divisor_operand.type == OperandType::REGISTER) {
        try {
            divisor = register_map_.get32(divisor_operand.text);
        } catch (const std::out_of_range& e) {
            log(session_id_, "Invalid register in DIV: " + divisor_operand.text, "ERROR", instructionPointer_, __FILE__, __LINE__);
            return;
        }
    } else {
        log(session_id_, "DIV only supports register operands currently.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    if (divisor == 0) {
        log(session_id_, "Divide error: Division by zero.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        // In a real CPU, this would trigger a #DE exception. We'll halt execution.
        // You could set a halt flag here, e.g., isRunning_ = false;
        return;
    }

    uint64_t dividend = (static_cast<uint64_t>(register_map_.get32("edx")) << 32) | register_map_.get32("eax");

    if (dividend / divisor > 0xFFFFFFFF) {
        log(session_id_, "Divide error: Quotient overflows EAX.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        // This also triggers a #DE exception.
        return;
    }

    register_map_.set32("eax", static_cast<uint32_t>(dividend / divisor)); // Quotient
    register_map_.set32("edx", static_cast<uint32_t>(dividend % divisor)); // Remainder
}

void X86Simulator::handleAnd(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "Invalid number of operands for AND", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& src_operand = decoded_instr.operands[1];

    if (dest_operand.type != OperandType::REGISTER) {
        log(session_id_, "AND destination must be a register.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    try {
        uint32_t destValue = register_map_.get32(dest_operand.text);
        uint32_t sourceValue = 0;

        if (src_operand.type == OperandType::REGISTER) {
            sourceValue = register_map_.get32(src_operand.text);
        } else { // Assuming immediate
            sourceValue = src_operand.value;
        }

        uint32_t result = destValue & sourceValue;
        register_map_.set32(dest_operand.text, result);

        // Update RFLAGS
        set_CF(false); // Carry Flag is cleared
        set_OF(false); // Overflow Flag is cleared
        set_ZF(result == 0);
        set_SF((result & 0x80000000) != 0);
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid register in AND: " + std::string(e.what());
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleOr(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "Invalid number of operands for OR", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& src_operand = decoded_instr.operands[1];

    if (dest_operand.type != OperandType::REGISTER) {
        log(session_id_, "OR destination must be a register.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    try {
        uint32_t destValue = register_map_.get32(dest_operand.text);
        uint32_t sourceValue = 0;

        if (src_operand.type == OperandType::REGISTER) {
            sourceValue = register_map_.get32(src_operand.text);
        } else { // Assuming immediate
            sourceValue = src_operand.value;
        }

        uint32_t result = destValue | sourceValue;
        register_map_.set32(dest_operand.text, result);

        // Update RFLAGS
        set_CF(false); // Carry Flag is cleared
        set_OF(false); // Overflow Flag is cleared
        set_ZF(result == 0);
        set_SF((result & 0x80000000) != 0);
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid register in OR: " + std::string(e.what());
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleXor(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "Invalid number of operands for XOR", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& src_operand = decoded_instr.operands[1];

    if (dest_operand.type != OperandType::REGISTER) {
        log(session_id_, "XOR destination must be a register.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    try {
        uint32_t destValue = register_map_.get32(dest_operand.text);
        uint32_t sourceValue = 0;

        if (src_operand.type == OperandType::REGISTER) {
            sourceValue = register_map_.get32(src_operand.text);
        } else { // Assuming immediate
            sourceValue = src_operand.value;
        }

        uint32_t result = destValue ^ sourceValue;
        register_map_.set32(dest_operand.text, result);

        // Update RFLAGS
        set_CF(false); // Carry Flag is cleared
        set_OF(false); // Overflow Flag is cleared
        set_ZF(result == 0);
        set_SF((result & 0x80000000) != 0);
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid register in XOR: " + std::string(e.what());
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleNot(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 1) {
        log(session_id_, "NOT instruction requires one operand.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& operand = decoded_instr.operands[0];

    if (operand.type != OperandType::REGISTER) {
        log(session_id_, "NOT only supports register operands currently.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    try {
        // Assuming 32-bit operation for now
        uint32_t value = register_map_.get32(operand.text);
        uint32_t result = ~value;
        register_map_.set32(operand.text, result);

        // The NOT instruction does not affect any flags.
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid register in NOT: " + operand.text;
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}
