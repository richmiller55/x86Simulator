#include "ir_executor_helpers.h"
#include "x86_simulator.h"
#include <variant>

/**
 * @brief Gets the value of an IR operand.
 * @note This is a placeholder implementation. A full implementation requires a robust
 *       mapping from abstract IRRegisters to concrete registers (e.g., eax, rbx) and
 *       full address calculation for IRMemoryOperands.
 */
uint64_t getOperandValue(const IROperand& op, RegisterMap& regs, Memory& mem) {
    if (std::holds_alternative<IRRegister>(op)) {
        const auto& ir_reg = std::get<IRRegister>(op);
        // Placeholder: a real implementation needs a mapping from IRRegister to a specific register name.
        if (ir_reg.type == IRRegisterType::GPR && ir_reg.index == 0) { // e.g., GPR[0] -> eax
            return regs.get32("eax");
        }
        if (ir_reg.type == IRRegisterType::GPR && ir_reg.index == 1) { // e.g., GPR[1] -> ecx
            return regs.get32("ecx");
        }
        if (ir_reg.type == IRRegisterType::GPR && ir_reg.index == 3) { // e.g., GPR[3] -> ebx
            return regs.get32("ebx");
        }
    } else if (std::holds_alternative<uint64_t>(op)) {
        return std::get<uint64_t>(op);
    } else if (std::holds_alternative<IRMemoryOperand>(op)) {
        const auto& mem_op = std::get<IRMemoryOperand>(op);
        // Simplified address calculation. A full implementation would handle base, index, scale.
        address_t addr = mem_op.displacement;
        return mem.read_dword(addr); // Assuming 32-bit read
    }
    // Other types (label, etc.) would be handled here.
    return 0;
}

/**
 * @brief Sets the value of an abstract IR register.
 * @note This is a placeholder implementation, similar to getOperandValue.
 */
void setRegisterValue(const IRRegister& reg, uint64_t value, RegisterMap& regs) {
    // Placeholder: a real implementation needs a mapping from IRRegister to a specific register name.
    if (reg.type == IRRegisterType::GPR && reg.index == 0) { // e.g., GPR[0] -> eax
        regs.set32("eax", value);
    }
    // Add other registers here...
}

/**
 * @brief Sets the value of a memory location based on an IRMemoryOperand.
 * @note This is a placeholder implementation.
 */
void setMemoryValue(const IRMemoryOperand& mem_op, uint64_t value, Memory& mem) {
    // Simplified address calculation. A full implementation would handle base, index, scale.
    address_t addr = mem_op.displacement;
    mem.write_qword(addr, value); // Assuming 64-bit write for now
}

/**
 * @brief Executes an IR 'Add' instruction.
 */
void handle_ir_add(const IRInstruction& ir_instr, X86Simulator& simulator) {
    if (ir_instr.operands.size() != 2) {
        simulator.log(simulator.get_session_id(), "Invalid number of operands for IR Add", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];

    if (!std::holds_alternative<IRRegister>(dest_op)) {
        simulator.log(simulator.get_session_id(), "IR Add requires a register destination.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_reg = std::get<IRRegister>(dest_op);

    uint32_t destValue = getOperandValue(dest_op, simulator.getRegisterMapForTesting(), simulator.getMemoryForTesting());
    uint32_t sourceValue = getOperandValue(src_op, simulator.getRegisterMapForTesting(), simulator.getMemoryForTesting());

    uint32_t result = destValue + sourceValue;
    setRegisterValue(dest_reg, result, simulator.getRegisterMapForTesting());

    // --- Flag Updates (using public flag setters on the simulator instance) ---
    simulator.set_ZF(result == 0);
    simulator.set_SF((result & 0x80000000) != 0);
    simulator.set_CF(result < destValue);
    // ... other flags (OF, AF, PF) would be set here ...
}

/**
 * @brief Executes an IR 'Move' instruction (register to register).
 */
void handle_ir_move(const IRInstruction& ir_instr, X86Simulator& simulator) {
    if (ir_instr.operands.size() != 2) {
        simulator.log(simulator.get_session_id(), "Invalid number of operands for IR Move", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];

    if (!std::holds_alternative<IRRegister>(dest_op)) {
        simulator.log(simulator.get_session_id(), "IR Move requires a register destination.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_reg = std::get<IRRegister>(dest_op);
    uint64_t sourceValue = getOperandValue(src_op, simulator.getRegisterMapForTesting(), simulator.getMemoryForTesting());

    setRegisterValue(dest_reg, sourceValue, simulator.getRegisterMapForTesting());
}

/**
 * @brief Executes an IR 'Load' instruction (memory to register).
 */
void handle_ir_load(const IRInstruction& ir_instr, X86Simulator& simulator) {
    if (ir_instr.operands.size() != 2) {
        simulator.log(simulator.get_session_id(), "Invalid number of operands for IR Load", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];

    if (!std::holds_alternative<IRRegister>(dest_op)) {
        simulator.log(simulator.get_session_id(), "IR Load requires a register destination.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }
    if (!std::holds_alternative<IRMemoryOperand>(src_op)) {
        simulator.log(simulator.get_session_id(), "IR Load requires a memory source.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_reg = std::get<IRRegister>(dest_op);
    uint64_t sourceValue = getOperandValue(src_op, simulator.getRegisterMapForTesting(), simulator.getMemoryForTesting());

    setRegisterValue(dest_reg, sourceValue, simulator.getRegisterMapForTesting());
}

/**
 * @brief Executes an IR 'Store' instruction (register to memory).
 */
void handle_ir_store(const IRInstruction& ir_instr, X86Simulator& simulator) {
    if (ir_instr.operands.size() != 2) {
        simulator.log(simulator.get_session_id(), "Invalid number of operands for IR Store", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];

    if (!std::holds_alternative<IRMemoryOperand>(dest_op)) {
        simulator.log(simulator.get_session_id(), "IR Store requires a memory destination.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_mem = std::get<IRMemoryOperand>(dest_op);
    uint64_t sourceValue = getOperandValue(src_op, simulator.getRegisterMapForTesting(), simulator.getMemoryForTesting());

    setMemoryValue(dest_mem, sourceValue, simulator.getMemoryForTesting());
}

/**
 * @brief Executes an IR 'Jump' instruction.
 */
void handle_ir_jump(const IRInstruction& ir_instr, X86Simulator& simulator) {
    if (ir_instr.operands.size() != 1) {
        simulator.log(simulator.get_session_id(), "Invalid number of operands for IR Jump", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& target_op = ir_instr.operands[0];
    address_t target_address = 0;

    // The target of a jump should be an immediate value (the address).
    if (std::holds_alternative<uint64_t>(target_op)) {
        target_address = std::get<uint64_t>(target_op);
    } else {
        // A label should have been resolved to an immediate address by the frontend.
        // If we get here, it's likely a logic error in the translation step.
        simulator.log(simulator.get_session_id(), "IR Jump target is not a valid address.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    // Directly set the instruction pointer.
    simulator.getRegisterMapForTesting().set64("rip", target_address);
}

/**
 * @brief Executes an IR 'Branch' instruction based on a condition.
 */
void handle_ir_branch(const IRInstruction& ir_instr, X86Simulator& simulator) {
    if (ir_instr.operands.size() != 2) {
        simulator.log(simulator.get_session_id(), "Invalid number of operands for IR Branch", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& target_op = ir_instr.operands[0];
    const auto& cond_op = ir_instr.operands[1];

    // --- 1. Get Target Address ---
    address_t target_address = 0;
    if (std::holds_alternative<uint64_t>(target_op)) {
        target_address = std::get<uint64_t>(target_op);
    } else {
        simulator.log(simulator.get_session_id(), "IR Branch target is not a valid address.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    // --- 2. Evaluate Condition ---
    if (!std::holds_alternative<IRConditionCode>(cond_op)) {
        simulator.log(simulator.get_session_id(), "IR Branch condition is not a valid IRConditionCode.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto condition = std::get<IRConditionCode>(cond_op);
    bool should_jump = false;

    switch (condition) {
        case IRConditionCode::NotEqual: // JNE
            should_jump = !simulator.get_ZF();
            break;
        case IRConditionCode::Equal: // JE
            should_jump = simulator.get_ZF();
            break;
        // Other conditions (Less, Greater, etc.) would be handled here.
        // case IRConditionCode::Less:
        //     should_jump = (simulator.get_SF() != simulator.get_OF());
        //     break;
        default:
            simulator.log(simulator.get_session_id(), "Unsupported IR branch condition.", "WARNING", 0, __FILE__, __LINE__);
            return;
    }

    // --- 3. Perform Jump if Condition is Met ---
    if (should_jump) {
        simulator.getRegisterMapForTesting().set64("rip", target_address);
    }
    // If the condition is not met, do nothing and let the IP advance normally.
}

/**
 * @brief Executes an IR 'Cmp' instruction, which performs a subtraction
 *        and updates flags without storing the result.
 */
void handle_ir_cmp(const IRInstruction& ir_instr, X86Simulator& simulator) {
    if (ir_instr.operands.size() != 2) {
        simulator.log(simulator.get_session_id(), "Invalid number of operands for IR Cmp", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& op1 = ir_instr.operands[0];
    const auto& op2 = ir_instr.operands[1];

    // Get the values of the operands. Assuming 32-bit for this example.
    uint32_t val1 = getOperandValue(op1, simulator.getRegisterMapForTesting(), simulator.getMemoryForTesting());
    uint32_t val2 = getOperandValue(op2, simulator.getRegisterMapForTesting(), simulator.getMemoryForTesting());

    // Perform the subtraction to determine the flags.
    uint32_t result = val1 - val2;

    // --- Update Flags ---
    simulator.set_ZF(result == 0);
    simulator.set_SF((result & 0x80000000) != 0);

    // Carry Flag (CF) is set if there was a borrow (unsigned subtraction).
    simulator.set_CF(val1 < val2);

    // Overflow Flag (OF) for signed subtraction.
    bool val1_sign = (val1 & 0x80000000);
    bool val2_sign = (val2 & 0x80000000);
    bool result_sign = (result & 0x80000000);
    simulator.set_OF((val1_sign != val2_sign) && (val1_sign != result_sign));

    // Adjust Flag (AF) for BCD arithmetic.
    simulator.set_AF((val1 & 0xF) < (val2 & 0xF));

    // Parity Flag (PF) for the number of set bits in the LSB.
    uint8_t lsb = result & 0xFF;
    int set_bits = 0;
    for (int i = 0; i < 8; ++i) { if ((lsb >> i) & 1) { set_bits++; } }
    simulator.set_PF((set_bits % 2) == 0);
}

/**
 * @brief Executes an IR 'Inc' operation, which is a special case of 'Add'.
 *        It increments an operand by 1 and updates flags, but does NOT affect the Carry Flag (CF).
 */
void handle_ir_inc(const IRInstruction& ir_instr, X86Simulator& simulator) {
    if (ir_instr.operands.size() != 1) {
        simulator.log(simulator.get_session_id(), "Invalid number of operands for IR Inc", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& op = ir_instr.operands[0];

    if (!std::holds_alternative<IRRegister>(op)) {
        simulator.log(simulator.get_session_id(), "IR Inc requires a register destination.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_reg = std::get<IRRegister>(op);

    // Assuming 32-bit for this example.
    uint32_t value = getOperandValue(op, simulator.getRegisterMapForTesting(), simulator.getMemoryForTesting());
    uint32_t result = value + 1;

    setRegisterValue(dest_reg, result, simulator.getRegisterMapForTesting());

    // --- Update Flags (INC does not affect CF) ---
    simulator.set_ZF(result == 0);
    simulator.set_SF((result & 0x80000000) != 0);

    // Overflow for inc occurs when incrementing the max positive signed value (e.g., 0x7FFFFFFF for 32-bit).
    simulator.set_OF(value == 0x7FFFFFFF);

    simulator.set_AF(((value & 0xF) + 1) > 0xF);

    uint8_t lsb = result & 0xFF;
    int set_bits = 0;
    for (int i = 0; i < 8; ++i) { if ((lsb >> i) & 1) { set_bits++; } }
    simulator.set_PF((set_bits % 2) == 0);
}

/**
 * @brief Executes an IR 'Syscall' instruction, which maps to 'INT' on x86.
 */
void handle_ir_syscall(const IRInstruction& ir_instr, X86Simulator& simulator) {
    if (ir_instr.operands.size() != 1) {
        simulator.log(simulator.get_session_id(), "IR Syscall requires one operand (the interrupt vector).", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& vector_op = ir_instr.operands[0];
    if (!std::holds_alternative<uint64_t>(vector_op)) {
        simulator.log(simulator.get_session_id(), "IR Syscall operand must be an immediate value.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    uint8_t interrupt_vector = std::get<uint64_t>(vector_op);

    if (interrupt_vector == 0x80) { // Linux syscall convention
        auto& regs = simulator.getRegisterMapForTesting();
        uint32_t syscall_num = regs.get32("eax");

        switch (syscall_num) {
            case 1: { // sys_exit
                uint32_t exit_code = regs.get32("ebx");
                std::string logMessage = "Program exited via sys_exit with code: " + std::to_string(exit_code);
                simulator.log(simulator.get_session_id(), logMessage, "INFO", 0, __FILE__, __LINE__);
                
                // In a real implementation, you would set a flag to halt the simulator.
                // For now, we can simulate this by setting RIP to a high value to stop the loop.
                regs.set64("rip", simulator.getMemoryForTesting().get_total_memory_size());
                break;
            }
            default: {
                simulator.log(simulator.get_session_id(), "Unsupported syscall: " + std::to_string(syscall_num), "WARNING", 0, __FILE__, __LINE__);
                break;
            }
        }
    }
}

/**
 * @brief Executes an IR 'Mul' instruction (unsigned, one-operand form).
 *        Multiplies EAX by the source operand. Stores result in EDX:EAX.
 */
void handle_ir_mul(const IRInstruction& ir_instr, X86Simulator& simulator) {
    if (ir_instr.operands.size() != 1) {
        simulator.log(simulator.get_session_id(), "IR Mul (one-operand) requires one operand.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    auto& regs = simulator.getRegisterMapForTesting();
    auto& mem = simulator.getMemoryForTesting();

    const auto& src_op = ir_instr.operands[0];

    // Get the source value (assuming 32-bit for this operation)
    uint32_t src_val = getOperandValue(src_op, regs, mem);

    // Get the value from the implicit EAX register
    uint64_t val_eax = regs.get32("eax");

    // Perform the 64-bit multiplication
    uint64_t result = val_eax * static_cast<uint64_t>(src_val);

    // Store the low 32 bits in EAX and the high 32 bits in EDX
    regs.set32("eax", static_cast<uint32_t>(result & 0xFFFFFFFF));
    regs.set32("edx", static_cast<uint32_t>(result >> 32));

    // Update Carry and Overflow flags. For unsigned MUL, they are set if the
    // upper half of the result (EDX) is non-zero.
    bool overflow = (regs.get32("edx") != 0);
    simulator.set_CF(overflow);
    simulator.set_OF(overflow);
}

/**
 * @brief Executes an IR 'IMul' instruction (signed, one-operand form).
 *        Multiplies EAX by the source operand. Stores result in EDX:EAX.
 */
void handle_ir_imul(const IRInstruction& ir_instr, X86Simulator& simulator) {
    if (ir_instr.operands.size() != 1) {
        simulator.log(simulator.get_session_id(), "IR IMul (one-operand) requires one operand.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    auto& regs = simulator.getRegisterMapForTesting();
    auto& mem = simulator.getMemoryForTesting();

    const auto& src_op = ir_instr.operands[0];

    // Get the source value as a signed 32-bit integer
    int32_t src_val = getOperandValue(src_op, regs, mem);

    // Get the value from the implicit EAX register as a signed 32-bit integer
    int64_t val_eax = static_cast<int32_t>(regs.get32("eax"));

    // Perform the 64-bit signed multiplication
    int64_t result = val_eax * static_cast<int64_t>(src_val);

    uint32_t result_low = static_cast<uint32_t>(result & 0xFFFFFFFF);
    uint32_t result_high = static_cast<uint32_t>(result >> 32);

    regs.set32("eax", result_low);
    regs.set32("edx", result_high);

    // Set CF and OF if the high part of the result (EDX) is not a sign-extension
    // of the low part (EAX). This means the result did not fit into 32 bits.
    bool fits;
    if ((result_low & 0x80000000) == 0) { // Positive result in EAX
        fits = (result_high == 0);
    } else { // Negative result in EAX
        fits = (result_high == 0xFFFFFFFF);
    }

    simulator.set_CF(!fits);
    simulator.set_OF(!fits);
}

/**
 * @brief Executes an IR 'Dec' operation, which is a special case of 'Sub'.
 *        It decrements an operand by 1 and updates flags, but does NOT affect the Carry Flag (CF).
 */
void handle_ir_dec(const IRInstruction& ir_instr, X86Simulator& simulator) {
    if (ir_instr.operands.size() != 1) {
        simulator.log(simulator.get_session_id(), "Invalid number of operands for IR Dec", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& op = ir_instr.operands[0];

    if (!std::holds_alternative<IRRegister>(op)) {
        simulator.log(simulator.get_session_id(), "IR Dec requires a register destination.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_reg = std::get<IRRegister>(op);

    // Assuming 32-bit for this example.
    uint32_t value = getOperandValue(op, simulator.getRegisterMapForTesting(), simulator.getMemoryForTesting());
    uint32_t result = value - 1;

    setRegisterValue(dest_reg, result, simulator.getRegisterMapForTesting());

    // --- Update Flags (DEC does not affect CF) ---
    simulator.set_ZF(result == 0);
    simulator.set_SF((result & 0x80000000) != 0);

    // Overflow for dec occurs when decrementing the minimum signed value (e.g., 0x80000000 for 32-bit).
    simulator.set_OF(value == 0x80000000);

    simulator.set_AF((value & 0xF) < 1); // Set if borrow from bit 4

    uint8_t lsb = result & 0xFF;
    int set_bits = 0;
    for (int i = 0; i < 8; ++i) { if ((lsb >> i) & 1) { set_bits++; } }
    simulator.set_PF((set_bits % 2) == 0);
}
