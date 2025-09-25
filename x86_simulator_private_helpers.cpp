#include "x86_simulator.h"
#include "decoder.h"
#include <immintrin.h>

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

    uint32_t destValue = 0;
    // Get the destination register value using its text representation from register_map_
    try {
        destValue = register_map_.get32(dest_operand.text);
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid destination operand in SUB: " + dest_operand.text;
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    uint32_t sourceValue = 0;
    if (src_operand.type == OperandType::REGISTER) {
        sourceValue = register_map_.get32(src_operand.text);
    } else {
        sourceValue = src_operand.value;
    }

    uint32_t result = destValue - sourceValue;

    set_ZF(result == 0);
    set_SF((result & 0x80000000) != 0);
    set_CF(destValue < sourceValue); // Set if borrow was needed
    bool dest_sign = (destValue & 0x80000000);
    bool src_sign = (sourceValue & 0x80000000);
    bool result_sign = (result & 0x80000000);
    set_OF((dest_sign != src_sign) && (dest_sign != result_sign));

    // Perform subtraction and update destination using register_map_
    try {
        register_map_.set32(dest_operand.text, result);
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid destination operand in SUB: " + dest_operand.text;
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handlePush(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 1) {
        log(session_id_, "PUSH instruction requires one operand.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& src_operand = decoded_instr.operands[0];
    uint64_t src_value = 0;
    size_t operand_size = 0;

    if (src_operand.type == OperandType::REGISTER) {
        const auto& reg_name = src_operand.text;
        if (register_map_.getRegisterNameMap64().count(reg_name)) {
            operand_size = 8;
            src_value = register_map_.get64(reg_name);
        } else if (register_map_.getRegisterNameMap32().count(reg_name)) {
            operand_size = 4;
            src_value = register_map_.get32(reg_name);
        } else {
            log(session_id_, "Unsupported register size for PUSH: " + reg_name, "ERROR", instructionPointer_, __FILE__, __LINE__);
            return;
        }
    } else if (src_operand.type == OperandType::IMMEDIATE) {
        operand_size = 8; // Assume 64-bit push for immediates
        src_value = src_operand.value;
    } else {
        log(session_id_, "PUSH only supports register or immediate operands currently.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    uint64_t rsp = register_map_.get64("rsp");
    rsp -= operand_size;
    register_map_.set64("rsp", rsp);

    if (rsp < memory_.stack_segment_start) {
        log(session_id_, "Stack overflow!", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    // Write value to the stack (little-endian)
    for (size_t i = 0; i < operand_size; ++i) {
        memory_.main_memory->at(rsp + i) = (src_value >> (i * 8)) & 0xFF;
    }
}

void X86Simulator::handlePop(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 1) {
        log(session_id_, "POP instruction requires one operand.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    if (dest_operand.type != OperandType::REGISTER) {
        log(session_id_, "POP only supports register operands currently.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const auto& reg_name = dest_operand.text;
    size_t operand_size = 0;

    if (register_map_.getRegisterNameMap64().count(reg_name)) {
        operand_size = 8;
    } else if (register_map_.getRegisterNameMap32().count(reg_name)) {
        operand_size = 4;
    } else {
        log(session_id_, "Unsupported register size for POP: " + reg_name, "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    uint64_t rsp = register_map_.get64("rsp");

    if (rsp + operand_size > memory_.stack_bottom) {
        log(session_id_, "Stack underflow!", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    uint64_t value = 0;
    for (size_t i = 0; i < operand_size; ++i) {
        value |= static_cast<uint64_t>(memory_.main_memory->at(rsp + i)) << (i * 8);
    }

    if (operand_size == 8) {
        register_map_.set64(reg_name, value);
    } else { // operand_size is 4
        register_map_.set32(reg_name, static_cast<uint32_t>(value));
    }

    register_map_.set64("rsp", rsp + operand_size);
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

void X86Simulator::handleVaddps(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 3) {
        log(session_id_, "Invalid number of operands for VADDPS", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& src1_operand = decoded_instr.operands[1];
    const DecodedOperand& src2_operand = decoded_instr.operands[2];

    if (dest_operand.type != OperandType::YMM_REGISTER || 
        src1_operand.type != OperandType::YMM_REGISTER || 
        src2_operand.type != OperandType::YMM_REGISTER) {
        log(session_id_, "VADDPS instruction requires YMM register operands.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    try {
        __m256i src1_val_i = register_map_.getYmm(src1_operand.text);
        __m256i src2_val_i = register_map_.getYmm(src2_operand.text);

        __m256 src1_val_ps = _mm256_castsi256_ps(src1_val_i);
        __m256 src2_val_ps = _mm256_castsi256_ps(src2_val_i);

        __m256 result_ps = _mm256_add_ps(src1_val_ps, src2_val_ps);

        register_map_.setYmm(dest_operand.text, _mm256_castps_si256(result_ps));
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid register in VADDPS: " + std::string(e.what());
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleVdivps(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 3) {
        log(session_id_, "Invalid number of operands for VDIVPS", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& src1_operand = decoded_instr.operands[1];
    const DecodedOperand& src2_operand = decoded_instr.operands[2];

    if (dest_operand.type != OperandType::YMM_REGISTER ||
        src1_operand.type != OperandType::YMM_REGISTER ||
        src2_operand.type != OperandType::YMM_REGISTER) {
        log(session_id_, "VDIVPS instruction requires YMM register operands.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    try {
        __m256i src1_val_i = register_map_.getYmm(src1_operand.text);
        __m256i src2_val_i = register_map_.getYmm(src2_operand.text);

        __m256 src1_val_ps = _mm256_castsi256_ps(src1_val_i);
        __m256 src2_val_ps = _mm256_castsi256_ps(src2_val_i);

        __m256 result_ps = _mm256_div_ps(src1_val_ps, src2_val_ps);

        register_map_.setYmm(dest_operand.text, _mm256_castps_si256(result_ps));
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid register in VDIVPS: " + std::string(e.what());
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleVmaxps(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 3) {
        log(session_id_, "Invalid number of operands for VMAXPS", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& src1_operand = decoded_instr.operands[1];
    const DecodedOperand& src2_operand = decoded_instr.operands[2];

    if (dest_operand.type != OperandType::YMM_REGISTER ||
        src1_operand.type != OperandType::YMM_REGISTER ||
        src2_operand.type != OperandType::YMM_REGISTER) {
        log(session_id_, "VMAXPS instruction requires YMM register operands.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    try {
        __m256i src1_val_i = register_map_.getYmm(src1_operand.text);
        __m256i src2_val_i = register_map_.getYmm(src2_operand.text);

        __m256 src1_val_ps = _mm256_castsi256_ps(src1_val_i);
        __m256 src2_val_ps = _mm256_castsi256_ps(src2_val_i);

        __m256 result_ps = _mm256_max_ps(src1_val_ps, src2_val_ps);

        register_map_.setYmm(dest_operand.text, _mm256_castps_si256(result_ps));
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid register in VMAXPS: " + std::string(e.what());
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleVpandn(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 3) {
        log(session_id_, "Invalid number of operands for VPANDN", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& src1_operand = decoded_instr.operands[1];
    const DecodedOperand& src2_operand = decoded_instr.operands[2];

    if (dest_operand.type != OperandType::YMM_REGISTER ||
        src1_operand.type != OperandType::YMM_REGISTER ||
        src2_operand.type != OperandType::YMM_REGISTER) {
        log(session_id_, "VPANDN instruction requires YMM register operands.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    try {
        __m256i src1_val = register_map_.getYmm(src1_operand.text);
        __m256i src2_val = register_map_.getYmm(src2_operand.text);
        
        // Emulate VPANDN using AVX1/SSE instructions as the target CPU does not support AVX2.
        // The operation is (NOT src1) AND src2.
        // We do this in two 128-bit chunks.

        // 1. Extract the lower and upper 128-bit lanes from the 256-bit registers.
        __m128i src1_low  = _mm256_extractf128_si256(src1_val, 0);
        __m128i src1_high = _mm256_extractf128_si256(src1_val, 1);
        __m128i src2_low  = _mm256_extractf128_si256(src2_val, 0);
        __m128i src2_high = _mm256_extractf128_si256(src2_val, 1);

        // 2. Perform the bitwise AND-NOT operation on each 128-bit lane.
        __m128i result_low  = _mm_andnot_si128(src1_low, src2_low);
        __m128i result_high = _mm_andnot_si128(src1_high, src2_high);

        // 3. Combine the 128-bit results back into a 256-bit register.
        __m256i result = _mm256_set_m128i(result_high, result_low);
        register_map_.setYmm(dest_operand.text, result);
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid register in VPANDN: " + std::string(e.what());
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleVpand(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 3) {
        log(session_id_, "Invalid number of operands for VPAND", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& src1_operand = decoded_instr.operands[1];
    const DecodedOperand& src2_operand = decoded_instr.operands[2];

    if (dest_operand.type != OperandType::YMM_REGISTER ||
        src1_operand.type != OperandType::YMM_REGISTER ||
        src2_operand.type != OperandType::YMM_REGISTER) {
        log(session_id_, "VPAND instruction requires YMM register operands.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    try {
        __m256i src1_val = register_map_.getYmm(src1_operand.text);
        __m256i src2_val = register_map_.getYmm(src2_operand.text);

        // Emulate VPAND using AVX1/SSE instructions as the target CPU does not support AVX2.
        // The operation is src1 AND src2.
        __m128i src1_low  = _mm256_extractf128_si256(src1_val, 0);
        __m128i src1_high = _mm256_extractf128_si256(src1_val, 1);
        __m128i src2_low  = _mm256_extractf128_si256(src2_val, 0);
        __m128i src2_high = _mm256_extractf128_si256(src2_val, 1);

        __m128i result_low  = _mm_and_si128(src1_low, src2_low);
        __m128i result_high = _mm_and_si128(src1_high, src2_high);

        __m256i result = _mm256_set_m128i(result_high, result_low);
        register_map_.setYmm(dest_operand.text, result);
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid register in VPAND: " + std::string(e.what());
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleVpmullw(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 3) {
        log(session_id_, "Invalid number of operands for VPMULLW", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& src1_operand = decoded_instr.operands[1];
    const DecodedOperand& src2_operand = decoded_instr.operands[2];

    if (dest_operand.type != OperandType::YMM_REGISTER ||
        src1_operand.type != OperandType::YMM_REGISTER ||
        src2_operand.type != OperandType::YMM_REGISTER) {
        log(session_id_, "VPMULLW instruction requires YMM register operands.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    try {
        __m256i src1_val = register_map_.getYmm(src1_operand.text);
        __m256i src2_val = register_map_.getYmm(src2_operand.text);

        // Emulate VPMULLW using AVX1/SSE instructions as the target CPU does not support AVX2.
        // The operation is a packed 16-bit multiply, keeping the low 16 bits of the result.
        __m128i src1_low  = _mm256_extractf128_si256(src1_val, 0);
        __m128i src1_high = _mm256_extractf128_si256(src1_val, 1);
        __m128i src2_low  = _mm256_extractf128_si256(src2_val, 0);
        __m128i src2_high = _mm256_extractf128_si256(src2_val, 1);

        __m128i result_low  = _mm_mullo_epi16(src1_low, src2_low);
        __m128i result_high = _mm_mullo_epi16(src1_high, src2_high);

        __m256i result = _mm256_set_m128i(result_high, result_low);
        register_map_.setYmm(dest_operand.text, result);
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid register in VPMULLW: " + std::string(e.what());
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleVminps(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 3) {
        log(session_id_, "Invalid number of operands for VMINPS", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& src1_operand = decoded_instr.operands[1];
    const DecodedOperand& src2_operand = decoded_instr.operands[2];

    if (dest_operand.type != OperandType::YMM_REGISTER ||
        src1_operand.type != OperandType::YMM_REGISTER ||
        src2_operand.type != OperandType::YMM_REGISTER) {
        log(session_id_, "VMINPS instruction requires YMM register operands.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    try {
        __m256i src1_val_i = register_map_.getYmm(src1_operand.text);
        __m256i src2_val_i = register_map_.getYmm(src2_operand.text);

        __m256 src1_val_ps = _mm256_castsi256_ps(src1_val_i);
        __m256 src2_val_ps = _mm256_castsi256_ps(src2_val_i);

        __m256 result_ps = _mm256_min_ps(src1_val_ps, src2_val_ps);

        register_map_.setYmm(dest_operand.text, _mm256_castps_si256(result_ps));
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid register in VMINPS: " + std::string(e.what());
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleVmovups(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "Invalid number of operands for VMOVUPS", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& src_operand = decoded_instr.operands[1];
    address_t data_segment_start = memory_.get_data_segment_start();

    if (dest_operand.type == OperandType::YMM_REGISTER && src_operand.type == OperandType::MEMORY) {
        // Load from memory
        __m256i value = memory_.read_ymm(src_operand.value);
        register_map_.setYmm(dest_operand.text, value);
    } else if (dest_operand.type == OperandType::MEMORY && src_operand.type == OperandType::YMM_REGISTER) {
        // Store to memory
        __m256i value = register_map_.getYmm(src_operand.text);
        memory_.write_ymm(dest_operand.value, value);
    } else if (dest_operand.type == OperandType::YMM_REGISTER && src_operand.type == OperandType::YMM_REGISTER) {
        __m256i value = register_map_.getYmm(src_operand.text);
        register_map_.setYmm(dest_operand.text, value);
    } else {
        log(session_id_, "Unsupported operand combination for VMOVUPS", "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleVpxor(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 3) {
        log(session_id_, "Invalid number of operands for VPXOR", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& src1_operand = decoded_instr.operands[1];
    const DecodedOperand& src2_operand = decoded_instr.operands[2];

    if (dest_operand.type != OperandType::YMM_REGISTER ||
        src1_operand.type != OperandType::YMM_REGISTER ||
        src2_operand.type != OperandType::YMM_REGISTER) {
        log(session_id_, "VPXOR instruction requires YMM register operands.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    try {
        __m256i src1_val = register_map_.getYmm(src1_operand.text);
        __m256i src2_val = register_map_.getYmm(src2_operand.text);

        // Emulate VPXOR using AVX1/SSE instructions as the target CPU does not support AVX2.
        // The operation is src1 XOR src2.
        __m128i src1_low  = _mm256_extractf128_si256(src1_val, 0);
        __m128i src1_high = _mm256_extractf128_si256(src1_val, 1);
        __m128i src2_low  = _mm256_extractf128_si256(src2_val, 0);
        __m128i src2_high = _mm256_extractf128_si256(src2_val, 1);

        __m128i result_low  = _mm_xor_si128(src1_low, src2_low);
        __m128i result_high = _mm_xor_si128(src1_high, src2_high);

        __m256i result = _mm256_set_m128i(result_high, result_low);
        register_map_.setYmm(dest_operand.text, result);
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid register in VPXOR: " + std::string(e.what());
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleVrcpps(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "Invalid number of operands for VRCPPS", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& src_operand = decoded_instr.operands[1];

    if (dest_operand.type != OperandType::YMM_REGISTER ||
        src_operand.type != OperandType::YMM_REGISTER) {
        log(session_id_, "VRCPPS instruction requires YMM register operands.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    try {
        __m256i src_val_i = register_map_.getYmm(src_operand.text);

        __m256 src_val_ps = _mm256_castsi256_ps(src_val_i);

        // Computes approximate reciprocals
        __m256 result_ps = _mm256_rcp_ps(src_val_ps);

        register_map_.setYmm(dest_operand.text, _mm256_castps_si256(result_ps));
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid register in VRCPPS: " + std::string(e.what());
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleVsqrtps(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "Invalid number of operands for VSQRTPS", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const auto& dest_op = decoded_instr.operands[0];
    const auto& src_op = decoded_instr.operands[1];

    if (dest_op.type != OperandType::YMM_REGISTER) {
        log(session_id_, "Destination operand for VSQRTPS must be a YMM register", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    __m256 src_ps;

    if (src_op.type == OperandType::YMM_REGISTER) {
        __m256i src_val_i = register_map_.getYmm(src_op.text);
        src_ps = _mm256_castsi256_ps(src_val_i);
    } else if (src_op.type == OperandType::MEMORY) {
        __m256i src_val_i = memory_.read_ymm(src_op.value);
        src_ps = _mm256_castsi256_ps(src_val_i);
    } else {
        log(session_id_, "Invalid source operand for VSQRTPS", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    // Perform element-wise square root on the packed single-precision floats.
    // This intrinsic handles special cases like sqrt(-ve) -> NaN, sqrt(-0) -> -0, etc.
    // It uses the rounding mode specified in the MXCSR register.
    __m256 result_ps = _mm256_sqrt_ps(src_ps);

    // TODO: The simulator needs to manage the MXCSR register for rounding modes
    // and handle floating-point exceptions (invalid operation, denormal, etc.).

    register_map_.setYmm(dest_op.text, _mm256_castps_si256(result_ps));
}

void X86Simulator::handleVsubps(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 3) {
        log(session_id_, "Invalid number of operands for VSUBPS", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& src1_operand = decoded_instr.operands[1];
    const DecodedOperand& src2_operand = decoded_instr.operands[2];

    if (dest_operand.type != OperandType::YMM_REGISTER ||
        src1_operand.type != OperandType::YMM_REGISTER ||
        src2_operand.type != OperandType::YMM_REGISTER) {
        log(session_id_, "VSUBPS instruction requires YMM register operands.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    try {
        __m256i src1_val_i = register_map_.getYmm(src1_operand.text);
        __m256i src2_val_i = register_map_.getYmm(src2_operand.text);

        __m256 src1_val_ps = _mm256_castsi256_ps(src1_val_i);
        __m256 src2_val_ps = _mm256_castsi256_ps(src2_val_i);

        __m256 result_ps = _mm256_sub_ps(src1_val_ps, src2_val_ps);

        register_map_.setYmm(dest_operand.text, _mm256_castps_si256(result_ps));
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid register in VSUBPS: " + std::string(e.what());
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleVpor(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 3) {
        log(session_id_, "Invalid number of operands for VPOR", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& src1_operand = decoded_instr.operands[1];
    const DecodedOperand& src2_operand = decoded_instr.operands[2];

    if (dest_operand.type != OperandType::YMM_REGISTER ||
        src1_operand.type != OperandType::YMM_REGISTER ||
        src2_operand.type != OperandType::YMM_REGISTER) {
        log(session_id_, "VPOR instruction requires YMM register operands.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    try {
        __m256i src1_val = register_map_.getYmm(src1_operand.text);
        __m256i src2_val = register_map_.getYmm(src2_operand.text);

        // Emulate VPOR using AVX1/SSE instructions as the target CPU does not support AVX2.
        // The operation is src1 OR src2.
        __m128i src1_low  = _mm256_extractf128_si256(src1_val, 0);
        __m128i src1_high = _mm256_extractf128_si256(src1_val, 1);
        __m128i src2_low  = _mm256_extractf128_si256(src2_val, 0);
        __m128i src2_high = _mm256_extractf128_si256(src2_val, 1);

        __m128i result_low  = _mm_or_si128(src1_low, src2_low);
        __m128i result_high = _mm_or_si128(src1_high, src2_high);

        __m256i result = _mm256_set_m128i(result_high, result_low);
        register_map_.setYmm(dest_operand.text, result);
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid register in VPOR: " + std::string(e.what());
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleIn(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "IN instruction requires two operands.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& src_operand = decoded_instr.operands[1];

    if (dest_operand.type != OperandType::REGISTER || dest_operand.text != "al") {
        log(session_id_, "IN instruction currently only supports AL as destination.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    if (src_operand.type != OperandType::IMMEDIATE) {
        log(session_id_, "IN instruction currently only supports immediate for port.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    // In a real scenario, this would read from an I/O port.
    // Here, we'll simulate reading a character from standard input.
    char input_char;
    std::cin >> input_char;
    register_map_.set8("al", static_cast<uint8_t>(input_char));
}

void X86Simulator::handleOut(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "OUT instruction requires two operands.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& src_operand = decoded_instr.operands[1];

    if (dest_operand.type != OperandType::IMMEDIATE) {
        log(session_id_, "OUT instruction currently only supports immediate for port.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    if (src_operand.type != OperandType::REGISTER || src_operand.text != "al") {
        log(session_id_, "OUT instruction currently only supports AL as source.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    // In a real scenario, this would write to an I/O port.
    // Here, we'll simulate writing a character to standard output.
    uint8_t output_char = register_map_.get8("al");
    std::cout << static_cast<char>(output_char);
}
