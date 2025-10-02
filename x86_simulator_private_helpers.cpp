#include "x86_simulator.h"
#include "decoder.h"
#include "avx_core.h"

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
    } else if (src_operand.type == OperandType::MEMORY) {
        // If the source is a memory location, read the value from memory.
        // Assuming a 32-bit read for now.
        address_t address = src_operand.value;
        uint32_t value_from_mem = memory_.read_dword(address);
        sourceValue = value_from_mem;
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

        // Set Adjust Flag (AF): Set if there is a carry from bit 3
        set_AF(((destValue & 0xF) + (sourceValue & 0xF)) > 0xF);

        // Set Parity Flag (PF): Set if the number of set bits in the least significant byte is even
        uint8_t lsb = result & 0xFF;
        int set_bits = 0;
        for (int i = 0; i < 8; ++i) {
            if ((lsb >> i) & 1) {
                set_bits++;
            }
        }
        set_PF((set_bits % 2) == 0);

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

    // Set Adjust Flag (AF): Set if there is a borrow from bit 4
    set_AF((destValue & 0xF) < (sourceValue & 0xF));

    // Set Parity Flag (PF): Set if the number of set bits in the least significant byte is even
    uint8_t lsb = result & 0xFF;
    int set_bits = 0;
    for (int i = 0; i < 8; ++i) {
        if ((lsb >> i) & 1) {
            set_bits++;
        }
    }
    set_PF((set_bits % 2) == 0);

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

    if (rsp < memory_.get_stack_segment_start()) {
        log(session_id_, "Stack overflow!", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    // Write value to the stack
    if (operand_size == 8) {
        memory_.write_stack(rsp, src_value);
    } else { // operand_size is 4
        memory_.write_stack_dword(rsp, static_cast<uint32_t>(src_value));
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

    if (rsp + operand_size > memory_.get_stack_bottom()) {
        log(session_id_, "Stack underflow!", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    if (operand_size == 8) {
        uint64_t value = memory_.read_stack(rsp);
        register_map_.set64(reg_name, value);
    } else { // operand_size is 4
        uint32_t value = memory_.read_stack_dword(rsp);
        register_map_.set32(reg_name, value);
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

void X86Simulator::handleCall(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.empty()) {
        log(session_id_, "CALL instruction requires a target.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& target_operand = decoded_instr.operands[0];
    if (target_operand.type != OperandType::IMMEDIATE) {
        log(session_id_, "CALL target must be a valid label or address.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    // The return address is the address of the instruction immediately following the CALL.
    uint64_t return_address = decoded_instr.address + decoded_instr.length_in_bytes;

    // Decrement stack pointer
    uint64_t rsp = register_map_.get64("rsp");
    rsp -= 8; // Pushing a 64-bit address
    register_map_.set64("rsp", rsp);

    if (rsp < memory_.get_stack_segment_start()) {
        log(session_id_, "Stack overflow!", "ERROR", instructionPointer_, __FILE__, __LINE__);
        register_map_.set64("rsp", rsp + 8); // Attempt to recover stack pointer
        return;
    }

    // Push the return address to the stack.
    memory_.write_qword(rsp, return_address);

    // Jump to target address
    address_t targetAddress = target_operand.value;
    register_map_.set64("rip", targetAddress);
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

void X86Simulator::handleJe(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.empty()) {
        log(session_id_, "JE instruction requires a target.", "ERROR",
	    instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& target_operand = decoded_instr.operands[0];
    if (get_ZF() == true) { // If Zero Flag is set, then jump
        // The target address is calculated during decoding and stored in the operand's value.
        address_t targetAddress = target_operand.value;
        register_map_.set64("rip", targetAddress);

    }
    // If ZF is not set, do nothing and let the instruction pointer advance normally.
}

void X86Simulator::handleJl(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.empty()) {
        log(session_id_, "JL instruction requires a target.", "ERROR",
	    instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& target_operand = decoded_instr.operands[0];
    if (get_SF() != get_OF()) { // If SF != OF, then jump
        // The target address is calculated during decoding and stored in the operand's value.
        address_t targetAddress = target_operand.value;
        register_map_.set64("rip", targetAddress);

    }
    // If SF == OF, do nothing and let the instruction pointer advance normally.
}

void X86Simulator::handleJb(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.empty()) {
        log(session_id_, "JB instruction requires a target.", "ERROR",
	    instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& target_operand = decoded_instr.operands[0];
    if (get_CF() == true) { // If Carry Flag is set, then jump
        // The target address is calculated during decoding and stored in the operand's value.
        address_t targetAddress = target_operand.value;
        register_map_.set64("rip", targetAddress);

    }
    // If CF is not set, do nothing and let the instruction pointer advance normally.
}

void X86Simulator::handleJae(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.empty()) {
        log(session_id_, "JAE instruction requires a target.", "ERROR",
	    instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& target_operand = decoded_instr.operands[0];
    if (get_CF() == false) { // If Carry Flag is not set, then jump
        // The target address is calculated during decoding and stored in the operand's value.
        address_t targetAddress = target_operand.value;
        register_map_.set64("rip", targetAddress);

    }
    // If CF is set, do nothing and let the instruction pointer advance normally.
}

void X86Simulator::handleJbe(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.empty()) {
        log(session_id_, "JBE instruction requires a target.", "ERROR",
	    instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& target_operand = decoded_instr.operands[0];
    if (get_CF() == true || get_ZF() == true) { // If Carry Flag is set OR Zero Flag is set, then jump
        // The target address is calculated during decoding and stored in the operand's value.
        address_t targetAddress = target_operand.value;
        register_map_.set64("rip", targetAddress);

    }
    // If CF is 0 and ZF is 0, do nothing and let the instruction pointer advance normally.
}

void X86Simulator::handleJs(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.empty()) {
        log(session_id_, "JS instruction requires a target.", "ERROR",
	    instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& target_operand = decoded_instr.operands[0];
    if (get_SF() == true) { // If Sign Flag is set, then jump
        // The target address is calculated during decoding and stored in the operand's value.
        address_t targetAddress = target_operand.value;
        register_map_.set64("rip", targetAddress);

    }
    // If SF is not set, do nothing and let the instruction pointer advance normally.
}

void X86Simulator::handleJns(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.empty()) {
        log(session_id_, "JNS instruction requires a target.", "ERROR",
	    instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& target_operand = decoded_instr.operands[0];
    if (get_SF() == false) { // If Sign Flag is not set, then jump
        // The target address is calculated during decoding and stored in the operand's value.
        address_t targetAddress = target_operand.value;
        register_map_.set64("rip", targetAddress);

    }
    // If SF is set, do nothing and let the instruction pointer advance normally.
}

void X86Simulator::handleJo(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.empty()) {
        log(session_id_, "JO instruction requires a target.", "ERROR",
	    instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& target_operand = decoded_instr.operands[0];
    if (get_OF() == true) { // If Overflow Flag is set, then jump
        // The target address is calculated during decoding and stored in the operand's value.
        address_t targetAddress = target_operand.value;
        register_map_.set64("rip", targetAddress);

    }
    // If OF is not set, do nothing and let the instruction pointer advance normally.
}

void X86Simulator::handleJno(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.empty()) {
        log(session_id_, "JNO instruction requires a target.", "ERROR",
	    instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& target_operand = decoded_instr.operands[0];
    if (get_OF() == false) { // If Overflow Flag is not set, then jump
        // The target address is calculated during decoding and stored in the operand's value.
        address_t targetAddress = target_operand.value;
        register_map_.set64("rip", targetAddress);

    }
    // If OF is set, do nothing and let the instruction pointer advance normally.
}

void X86Simulator::handleJge(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.empty()) {
        log(session_id_, "JGE instruction requires a target.", "ERROR",
	    instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& target_operand = decoded_instr.operands[0];
    if (get_SF() == get_OF()) { // If SF == OF, then jump
        // The target address is calculated during decoding and stored in the operand's value.
        address_t targetAddress = target_operand.value;
        register_map_.set64("rip", targetAddress);

    }
    // If SF != OF, do nothing and let the instruction pointer advance normally.
}

void X86Simulator::handleJle(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.empty()) {
        log(session_id_, "JLE instruction requires a target.", "ERROR",
	    instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& target_operand = decoded_instr.operands[0];
    if (get_ZF() || (get_SF() != get_OF())) { // If ZF is set, or if SF != OF, then jump
        // The target address is calculated during decoding and stored in the operand's value.
        address_t targetAddress = target_operand.value;
        register_map_.set64("rip", targetAddress);

    }
    // If ZF is not set and SF == OF, do nothing and let the instruction pointer advance normally.
}

void X86Simulator::handleJg(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.empty()) {
        log(session_id_, "JG instruction requires a target.", "ERROR",
	    instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& target_operand = decoded_instr.operands[0];
    if (!get_ZF() && (get_SF() == get_OF())) { // If ZF is 0 and SF == OF, then jump
        // The target address is calculated during decoding and stored in the operand's value.
        address_t targetAddress = target_operand.value;
        register_map_.set64("rip", targetAddress);

    }
    // If ZF is 1 or SF != OF, do nothing and let the instruction pointer advance normally.
}

void X86Simulator::handleJa(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.empty()) {
        log(session_id_, "JA instruction requires a target.", "ERROR",
	    instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& target_operand = decoded_instr.operands[0];
    if (!get_CF() && !get_ZF()) { // If CF is 0 and ZF is 0, then jump
        // The target address is calculated during decoding and stored in the operand's value.
        address_t targetAddress = target_operand.value;
        register_map_.set64("rip", targetAddress);

    }
    // If CF is 1 or ZF is 1, do nothing and let the instruction pointer advance normally.
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
    // For subtraction (a - b), overflow occurs if the operands have different signs
    // and the result's sign is different from the first operand's sign.
    bool val1_sign = (val1 & 0x80000000);
    bool val2_sign = (val2 & 0x80000000);
    bool result_sign = (result & 0x80000000);
    set_OF((val1_sign != val2_sign) && (result_sign != val1_sign));
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

void X86Simulator::handleImul(const DecodedInstruction& decoded_instr) {
    // This handles the one-operand form: IMUL r/m32
    if (decoded_instr.operands.size() != 1) {
        log(session_id_, "IMUL (one-operand) requires one operand.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& src_operand = decoded_instr.operands[0];
    int32_t src_val = 0;

    if (src_operand.type == OperandType::REGISTER) {
        try {
            src_val = register_map_.get32(src_operand.text);
        } catch (const std::out_of_range& e) {
            log(session_id_, "Invalid register in IMUL: " + src_operand.text, "ERROR", instructionPointer_, __FILE__, __LINE__);
            return;
        }
    } else {
        log(session_id_, "IMUL only supports register operands currently.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    int64_t val_eax = static_cast<int32_t>(register_map_.get32("eax"));
    int64_t result = val_eax * static_cast<int64_t>(src_val);

    uint32_t result_low = static_cast<uint32_t>(result & 0xFFFFFFFF);
    uint32_t result_high = static_cast<uint32_t>(result >> 32);

    register_map_.set32("eax", result_low);
    register_map_.set32("edx", result_high);

    // Set CF and OF if the high part of the result (EDX) is not a sign-extension of the low part (EAX).
    // This means the result did not fit into 32 bits.
    if (result_high == 0 && (result_low & 0x80000000) == 0) { // Positive result fits
        set_CF(false);
        set_OF(false);
    } else if (result_high == 0xFFFFFFFF && (result_low & 0x80000000) != 0) { // Negative result fits
        set_CF(false);
        set_OF(false);
    } else { // Result does not fit
        set_CF(true);
        set_OF(true);
    }
}

void X86Simulator::handleIdiv(const DecodedInstruction& decoded_instr) {
    // This handles the one-operand form: IDIV r/m32
    if (decoded_instr.operands.size() != 1) {
        log(session_id_, "IDIV (one-operand) requires one operand.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& divisor_operand = decoded_instr.operands[0];
    int32_t divisor = 0;

    if (divisor_operand.type == OperandType::REGISTER) {
        try {
            divisor = register_map_.get32(divisor_operand.text);
        } catch (const std::out_of_range& e) {
            log(session_id_, "Invalid register in IDIV: " + divisor_operand.text, "ERROR", instructionPointer_, __FILE__, __LINE__);
            return;
        }
    } else {
        log(session_id_, "IDIV only supports register operands currently.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    if (divisor == 0) {
        log(session_id_, "Divide error: Division by zero.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        // In a real CPU, this would trigger a #DE exception.
        return;
    }

    int64_t dividend = (static_cast<int64_t>(register_map_.get32("edx")) << 32) | register_map_.get32("eax");

    int64_t quotient_64 = dividend / divisor;
    int64_t remainder_64 = dividend % divisor;

    // Check for overflow. The quotient must fit within a 32-bit signed integer.
    if (quotient_64 > std::numeric_limits<int32_t>::max() || quotient_64 < std::numeric_limits<int32_t>::min()) {
        log(session_id_, "Divide error: Quotient overflows EAX.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        // This also triggers a #DE exception.
        return;
    }

    register_map_.set32("eax", static_cast<int32_t>(quotient_64));
    register_map_.set32("edx", static_cast<int32_t>(remainder_64));

    // The state of CF, OF, SF, ZF, AF, and PF is undefined after IDIV.
    // We can choose to leave them as they are or clear them. For now, we'll leave them.
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

void X86Simulator::handleShl(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "Invalid number of operands for SHL", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& count_operand = decoded_instr.operands[1];

    if (dest_operand.type != OperandType::REGISTER || count_operand.type != OperandType::IMMEDIATE) {
        log(session_id_, "SHL currently supports register, immediate operands.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    try {
        uint32_t dest_value = register_map_.get32(dest_operand.text);
        uint8_t count = count_operand.value & 0x1F; // Mask to 5 bits for 32-bit operands

        if (count == 0) {
            // No operation, flags are not affected
            return;
        }

        uint32_t result = dest_value << count;
        register_map_.set32(dest_operand.text, result);

        // Update RFLAGS
        // CF is the last bit shifted out
        set_CF((dest_value >> (32 - count)) & 1);

        // OF is defined only for 1-bit shifts.
        if (count == 1) {
            // OF is set if the top two bits of the original value were different.
            set_OF(((dest_value >> 31) & 1) != ((dest_value >> 30) & 1));
        }

        set_ZF(result == 0);
        set_SF((result & 0x80000000) != 0);
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid register in SHL: " + std::string(e.what());
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleShr(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "Invalid number of operands for SHR", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& count_operand = decoded_instr.operands[1];

    if (dest_operand.type != OperandType::REGISTER || count_operand.type != OperandType::IMMEDIATE) {
        log(session_id_, "SHR currently supports register, immediate operands.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    try {
        uint32_t dest_value = register_map_.get32(dest_operand.text);
        uint8_t count = count_operand.value & 0x1F; // Mask to 5 bits for 32-bit operands

        if (count == 0) {
            // No operation, flags are not affected
            return;
        }

        uint32_t result = dest_value >> count;
        register_map_.set32(dest_operand.text, result);

        // Update RFLAGS
        // CF is the last bit shifted out
        set_CF((dest_value >> (count - 1)) & 1);

        // OF is defined only for 1-bit shifts.
        if (count == 1) {
            // For SHR, OF is set to the most-significant bit of the original operand.
            set_OF((dest_value & 0x80000000) != 0);
        }

        set_ZF(result == 0);
        set_SF((result & 0x80000000) != 0); // SHR clears the MSB, so SF will be 0.
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid register in SHR: " + std::string(e.what());
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleSar(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "Invalid number of operands for SAR", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& count_operand = decoded_instr.operands[1];

    if (dest_operand.type != OperandType::REGISTER || count_operand.type != OperandType::IMMEDIATE) {
        log(session_id_, "SAR currently supports register, immediate operands.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    try {
        int32_t dest_value = register_map_.get32(dest_operand.text); // Treat as signed
        uint8_t count = count_operand.value & 0x1F; // Mask to 5 bits for 32-bit operands

        if (count == 0) {
            // No operation, flags are not affected
            return;
        }

        int32_t result = dest_value >> count; // C++ >> on signed types is an arithmetic shift
        register_map_.set32(dest_operand.text, result);

        // Update RFLAGS
        // CF is the last bit shifted out
        set_CF((dest_value >> (count - 1)) & 1);

        // OF is cleared for multi-bit shifts. For a 1-bit SAR, OF is always 0.
        if (count == 1) {
            set_OF(false);
        }

        set_ZF(result == 0);
        set_SF((result & 0x80000000) != 0);

    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid register in SAR: " + std::string(e.what());
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleRol(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "Invalid number of operands for ROL", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& count_operand = decoded_instr.operands[1];

    if (dest_operand.type != OperandType::REGISTER || count_operand.type != OperandType::IMMEDIATE) {
        log(session_id_, "ROL currently supports register, immediate operands.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    try {
        uint32_t dest_value = register_map_.get32(dest_operand.text);
        uint8_t count = count_operand.value & 0x1F; // Mask to 5 bits for 32-bit operands

        if (count == 0) {
            // No operation, flags are not affected
            return;
        }

        uint32_t result = (dest_value << count) | (dest_value >> (32 - count));
        register_map_.set32(dest_operand.text, result);

        // Update RFLAGS
        // CF is the last bit rotated out (which is the new LSB of the result)
        set_CF(result & 1);

        // OF is defined only for 1-bit rotates.
        if (count == 1) {
            // OF is set if the new sign bit is different from the new carry flag.
            bool new_sf = (result & 0x80000000) != 0;
            set_OF(new_sf != get_CF());
        }
        // For multi-bit rotates, OF is undefined. We'll leave it unmodified.

    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid register in ROL: " + std::string(e.what());
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleRor(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "Invalid number of operands for ROR", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& count_operand = decoded_instr.operands[1];

    if (dest_operand.type != OperandType::REGISTER || count_operand.type != OperandType::IMMEDIATE) {
        log(session_id_, "ROR currently supports register, immediate operands.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    try {
        uint32_t dest_value = register_map_.get32(dest_operand.text);
        uint8_t count = count_operand.value & 0x1F; // Mask to 5 bits for 32-bit operands

        if (count == 0) {
            // No operation, flags are not affected
            return;
        }

        uint32_t result = (dest_value >> count) | (dest_value << (32 - count));
        register_map_.set32(dest_operand.text, result);

        // Update RFLAGS
        // CF is the last bit rotated out (which is the new MSB of the result)
        set_CF((result & 0x80000000) != 0);

        // OF is defined only for 1-bit rotates.
        if (count == 1) {
            // OF is set if the two most-significant bits of the result are different.
            bool msb = (result & 0x80000000) != 0;
            bool msb_minus_1 = (result & 0x40000000) != 0;
            set_OF(msb != msb_minus_1);
        }
        // For multi-bit rotates, OF is undefined. We'll leave it unmodified.

    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid register in ROR: " + std::string(e.what());
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleLea(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "Invalid number of operands for LEA", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& src_operand = decoded_instr.operands[1];

    if (dest_operand.type != OperandType::REGISTER || src_operand.type != OperandType::MEMORY) {
        log(session_id_, "LEA requires a register destination and memory source.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    // For LEA, the "source" is an address calculation, not a value from memory.
    // We need to parse the memory operand text to figure out the address.
    // This is a simplified implementation for formats like `[reg]`
    const std::string& mem_text = src_operand.text;
    if (mem_text.length() > 2 && mem_text.front() == '[' && mem_text.back() == ']') {
        std::string reg_name = mem_text.substr(1, mem_text.length() - 2);
        try {
            // The "effective address" is simply the value in the base register.
            uint32_t effective_address = register_map_.get32(reg_name);

            // Store this calculated address into the destination register.
            register_map_.set32(dest_operand.text, effective_address);

        } catch (const std::out_of_range& e) {
            std::string logMessage = "Invalid register in LEA memory operand: " + reg_name;
            log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
        }
    } else {
        log(session_id_, "Unsupported memory addressing mode in LEA: " + mem_text, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleXchg(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "Invalid number of operands for XCHG", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& op1 = decoded_instr.operands[0];
    const DecodedOperand& op2 = decoded_instr.operands[1];

    if (op1.type != OperandType::REGISTER || op2.type != OperandType::REGISTER) {
        log(session_id_, "XCHG currently supports register-to-register exchange.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    try {
        // Assuming 32-bit registers for now
        uint32_t val1 = register_map_.get32(op1.text);
        uint32_t val2 = register_map_.get32(op2.text);

        // Swap the values
        uint32_t temp = val1;
        val1 = val2;
        val2 = temp;

        // Write the swapped values back to the registers
        register_map_.set32(op1.text, val1);
        register_map_.set32(op2.text, val2);

        // XCHG does not affect any flags.

    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid register in XCHG: " + std::string(e.what());
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleMovsx(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "Invalid number of operands for MOVSX", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_op = decoded_instr.operands[0];
    const DecodedOperand& src_op = decoded_instr.operands[1];

    // We are implementing MOVSX r32, r/m8
    if (dest_op.type != OperandType::REGISTER || src_op.type != OperandType::REGISTER) {
        log(session_id_, "MOVSX currently supports register-to-register.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    try {
        // Get the 8-bit source value
        uint8_t src_value = register_map_.get8(src_op.text);

        // Sign-extend the 8-bit value to 32 bits.
        // Cast the 8-bit value to a signed 8-bit integer (int8_t).
        // Then, assign it to a signed 32-bit integer (int32_t).
        // The compiler will handle the sign extension automatically.
        int32_t extended_value = static_cast<int8_t>(src_value);

        // Write the 32-bit result to the destination register.
        register_map_.set32(dest_op.text, extended_value);

        // MOVSX does not affect any flags.

    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid register in MOVSX: " + std::string(e.what());
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleMovzx(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "Invalid number of operands for MOVZX", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_op = decoded_instr.operands[0];
    const DecodedOperand& src_op = decoded_instr.operands[1];

    // We are implementing MOVZX r32, r/m8
    if (dest_op.type != OperandType::REGISTER || src_op.type != OperandType::REGISTER) {
        log(session_id_, "MOVZX currently supports register-to-register.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    try {
        // Get the 8-bit source value
        uint8_t src_value = register_map_.get8(src_op.text);

        // Zero-extend the 8-bit value to 32 bits.
        // A simple cast from uint8_t to uint32_t achieves this.
        uint32_t extended_value = static_cast<uint32_t>(src_value);

        // Write the 32-bit result to the destination register.
        register_map_.set32(dest_op.text, extended_value);

        // MOVZX does not affect any flags.

    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid register in MOVZX: " + std::string(e.what());
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleMovsb(const DecodedInstruction& decoded_instr) {
    // MOVSB: Move byte from [RSI] to [RDI]
    // RSI and RDI are then adjusted based on the Direction Flag (DF).

    try {
        address_t src_addr = register_map_.get64("rsi");
        address_t dest_addr = register_map_.get64("rdi");

        // Read the byte from the source address.
        uint8_t byte_to_move = memory_.read_byte(src_addr);

        // Write the byte to the destination address.
        memory_.write_byte(dest_addr, byte_to_move);

        // Adjust RSI and RDI based on the Direction Flag (DF).
        int8_t increment = get_DF() ? -1 : 1;
        register_map_.set64("rsi", src_addr + increment);
        register_map_.set64("rdi", dest_addr + increment);

        // MOVSB does not affect any flags other than the implicit update of RSI/RDI.

    } catch (const std::out_of_range& e) {
        log(session_id_, "Memory access out of bounds during MOVSB.", "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleMovsw(const DecodedInstruction& decoded_instr) {
    // MOVSW: Move word from [RSI] to [RDI]
    // RSI and RDI are then adjusted by 2 based on the Direction Flag (DF).

    try {
        address_t src_addr = register_map_.get64("rsi");
        address_t dest_addr = register_map_.get64("rdi");

        // Check for memory bounds for a 2-byte read/write
        if (src_addr + 1 >= memory_.get_total_memory_size() || dest_addr + 1 >= memory_.get_total_memory_size()) {
            throw std::out_of_range("Memory access out of bounds during MOVSW.");
        }

        // Read the word (2 bytes) from the source address.
        uint8_t byte1 = memory_.read_byte(src_addr);
        uint8_t byte2 = memory_.read_byte(src_addr + 1);

        // Write the word to the destination address.
        memory_.write_byte(dest_addr, byte1);
        memory_.write_byte(dest_addr + 1, byte2);

        // Adjust RSI and RDI by 2 based on the Direction Flag (DF).
        int8_t increment = get_DF() ? -2 : 2;
        register_map_.set64("rsi", src_addr + increment);
        register_map_.set64("rdi", dest_addr + increment);

        // MOVSW does not affect any flags other than the implicit update of RSI/RDI.

    } catch (const std::out_of_range& e) {
        log(session_id_, "Memory access out of bounds during MOVSW.", "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleMovsd(const DecodedInstruction& decoded_instr) {
    // MOVSD: Move doubleword from [RSI] to [RDI]
    // RSI and RDI are then adjusted by 4 based on the Direction Flag (DF).

    try {
        address_t src_addr = register_map_.get64("rsi");
        address_t dest_addr = register_map_.get64("rdi");

        // Check for memory bounds for a 4-byte read/write
        if (src_addr + 3 >= memory_.get_total_memory_size() || dest_addr + 3 >= memory_.get_total_memory_size()) {
            throw std::out_of_range("Memory access out of bounds during MOVSD.");
        }

        // Read the doubleword (4 bytes) from the source address.
        uint32_t dword_to_move = memory_.read_dword(src_addr);

        // Write the doubleword to the destination address.
        memory_.write_byte(dest_addr, dword_to_move & 0xFF);
        memory_.write_byte(dest_addr + 1, (dword_to_move >> 8) & 0xFF);
        memory_.write_byte(dest_addr + 2, (dword_to_move >> 16) & 0xFF);
        memory_.write_byte(dest_addr + 3, (dword_to_move >> 24) & 0xFF);

        // Adjust RSI and RDI by 4 based on the Direction Flag (DF).
        int8_t increment = get_DF() ? -4 : 4;
        register_map_.set64("rsi", src_addr + increment);
        register_map_.set64("rdi", dest_addr + increment);

        // MOVSD does not affect any flags other than the implicit update of RSI/RDI.

    } catch (const std::out_of_range& e) {
        log(session_id_, "Memory access out of bounds during MOVSD.", "ERROR", instructionPointer_, __FILE__, __LINE__);
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
        m256i_t src1_val = register_map_.getYmm(src1_operand.text);
        m256i_t src2_val = register_map_.getYmm(src2_operand.text);

        m256i_t result = _mm256_add_ps_sim(src1_val, src2_val);

        register_map_.setYmm(dest_operand.text, result);
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
        m256i_t src1_val = register_map_.getYmm(src1_operand.text);
        m256i_t src2_val = register_map_.getYmm(src2_operand.text);

        m256i_t result = _mm256_div_ps_sim(src1_val, src2_val);

        register_map_.setYmm(dest_operand.text, result);
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
        m256i_t src1_val = register_map_.getYmm(src1_operand.text);
        m256i_t src2_val = register_map_.getYmm(src2_operand.text);

        m256i_t result = _mm256_max_ps_sim(src1_val, src2_val);

        register_map_.setYmm(dest_operand.text, result);
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
        m256i_t src1_val = register_map_.getYmm(src1_operand.text);
        m256i_t src2_val = register_map_.getYmm(src2_operand.text);
        
        m128i_t src1_low  = _mm256_extractf128_si256_sim(src1_val, 0);
        m128i_t src1_high = _mm256_extractf128_si256_sim(src1_val, 1);
        m128i_t src2_low  = _mm256_extractf128_si256_sim(src2_val, 0);
        m128i_t src2_high = _mm256_extractf128_si256_sim(src2_val, 1);

        m128i_t result_low  = _mm_andnot_si128_sim(src1_low, src2_low);
        m128i_t result_high = _mm_andnot_si128_sim(src1_high, src2_high);

        m256i_t result = _mm256_set_m128i_sim(result_high, result_low);
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
        m256i_t src1_val = register_map_.getYmm(src1_operand.text);
        m256i_t src2_val = register_map_.getYmm(src2_operand.text);

        m128i_t src1_low  = _mm256_extractf128_si256_sim(src1_val, 0);
        m128i_t src1_high = _mm256_extractf128_si256_sim(src1_val, 1);
        m128i_t src2_low  = _mm256_extractf128_si256_sim(src2_val, 0);
        m128i_t src2_high = _mm256_extractf128_si256_sim(src2_val, 1);

        m128i_t result_low  = _mm_and_si128_sim(src1_low, src2_low);
        m128i_t result_high = _mm_and_si128_sim(src1_high, src2_high);

        m256i_t result = _mm256_set_m128i_sim(result_high, result_low);
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
        m256i_t src1_val = register_map_.getYmm(src1_operand.text);
        m256i_t src2_val = register_map_.getYmm(src2_operand.text);

        m128i_t src1_low  = _mm256_extractf128_si256_sim(src1_val, 0);
        m128i_t src1_high = _mm256_extractf128_si256_sim(src1_val, 1);
        m128i_t src2_low  = _mm256_extractf128_si256_sim(src2_val, 0);
        m128i_t src2_high = _mm256_extractf128_si256_sim(src2_val, 1);

        m128i_t result_low  = _mm_mullo_epi16_sim(src1_low, src2_low);
        m128i_t result_high = _mm_mullo_epi16_sim(src1_high, src2_high);

        m256i_t result = _mm256_set_m128i_sim(result_high, result_low);
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
        m256i_t src1_val = register_map_.getYmm(src1_operand.text);
        m256i_t src2_val = register_map_.getYmm(src2_operand.text);

        m256i_t result = _mm256_min_ps_sim(src1_val, src2_val);

        register_map_.setYmm(dest_operand.text, result);
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

    if (dest_operand.type == OperandType::YMM_REGISTER && src_operand.type == OperandType::MEMORY) {
        // Load from memory
        m256i_t value = memory_.read_ymm(src_operand.value);
        register_map_.setYmm(dest_operand.text, value);
    } else if (dest_operand.type == OperandType::MEMORY && src_operand.type == OperandType::YMM_REGISTER) {
        // Store to memory
        m256i_t value = register_map_.getYmm(src_operand.text);
        memory_.write_ymm(dest_operand.value, value);
    } else if (dest_operand.type == OperandType::YMM_REGISTER && src_operand.type == OperandType::YMM_REGISTER) {
        m256i_t value = register_map_.getYmm(src_operand.text);
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
        m256i_t src1_val = register_map_.getYmm(src1_operand.text);
        m256i_t src2_val = register_map_.getYmm(src2_operand.text);

        m128i_t src1_low  = _mm256_extractf128_si256_sim(src1_val, 0);
        m128i_t src1_high = _mm256_extractf128_si256_sim(src1_val, 1);
        m128i_t src2_low  = _mm256_extractf128_si256_sim(src2_val, 0);
        m128i_t src2_high = _mm256_extractf128_si256_sim(src2_val, 1);

        m128i_t result_low  = _mm_xor_si128_sim(src1_low, src2_low);
        m128i_t result_high = _mm_xor_si128_sim(src1_high, src2_high);

        m256i_t result = _mm256_set_m128i_sim(result_high, result_low);
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

    const auto& dest_op = decoded_instr.operands[0];
    const auto& src_op = decoded_instr.operands[1];

    if (dest_op.type != OperandType::YMM_REGISTER) {
        log(session_id_, "Destination operand for VRCPPS must be a YMM register", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    m256i_t src_val;

    if (src_op.type == OperandType::YMM_REGISTER) {
        src_val = register_map_.getYmm(src_op.text);
    } else if (src_op.type == OperandType::MEMORY) {
        src_val = memory_.read_ymm(src_op.value);
    } else {
        log(session_id_, "Invalid source operand for VRCPPS", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    m256i_t result = _mm256_rcp_ps_sim(src_val);

    register_map_.setYmm(dest_op.text, result);
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

    m256i_t src_val;

    if (src_op.type == OperandType::YMM_REGISTER) {
        src_val = register_map_.getYmm(src_op.text);
    } else if (src_op.type == OperandType::MEMORY) {
        src_val = memory_.read_ymm(src_op.value);
    } else {
        log(session_id_, "Invalid source operand for VSQRTPS", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    m256i_t result = _mm256_sqrt_ps_sim(src_val);

    register_map_.setYmm(dest_op.text, result);
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
        m256i_t src1_val = register_map_.getYmm(src1_operand.text);
        m256i_t src2_val = register_map_.getYmm(src2_operand.text);

        m256i_t result = _mm256_sub_ps_sim(src1_val, src2_val);

        register_map_.setYmm(dest_operand.text, result);
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
        m256i_t src1_val = register_map_.getYmm(src1_operand.text);
        m256i_t src2_val = register_map_.getYmm(src2_operand.text);

        m128i_t src1_low  = _mm256_extractf128_si256_sim(src1_val, 0);
        m128i_t src1_high = _mm256_extractf128_si256_sim(src1_val, 1);
        m128i_t src2_low  = _mm256_extractf128_si256_sim(src2_val, 0);
        m128i_t src2_high = _mm256_extractf128_si256_sim(src2_val, 1);

        m128i_t result_low  = _mm_or_si128_sim(src1_low, src2_low);
        m128i_t result_high = _mm_or_si128_sim(src1_high, src2_high);

        m256i_t result = _mm256_set_m128i_sim(result_high, result_low);
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
