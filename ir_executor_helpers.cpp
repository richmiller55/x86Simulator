#include "ir_executor_helpers.h"
#include "x86_simulator.h"
#include <variant>



/**
 * @brief Gets the value of an IR operand by using the architecture map.
 */
uint64_t getOperandValue(const IROperand& op, X86Simulator& simulator) {
    if (std::holds_alternative<IRRegister>(op)) {
        const auto& ir_reg = std::get<IRRegister>(op);
        const auto& arch = simulator.get_architecture();
        const std::string& reg_name = arch.get_register_name(ir_reg);
        auto& regs = simulator.getRegisterMap();

        switch (ir_reg.size) {
            case 8:   return regs.get8(reg_name);
            case 16:  return regs.get16(reg_name);
            case 32:  return regs.get32(reg_name);
            case 64:  return regs.get64(reg_name);
            default:
                throw std::runtime_error("Unsupported register size in getOperandValue: " + std::to_string(ir_reg.size));
        }
    } else if (std::holds_alternative<uint64_t>(op)) {
        return std::get<uint64_t>(op);
    } else if (std::holds_alternative<IRMemoryOperand>(op)) {
        const auto& mem_op = std::get<IRMemoryOperand>(op);
        auto& regs = simulator.getRegisterMap();
        auto& mem = simulator.getMemory();
        const auto& arch = simulator.get_architecture();

        address_t addr = mem_op.displacement;
        if (mem_op.base_reg) {
            const std::string& reg_name = arch.get_register_name(*mem_op.base_reg);
            addr += regs.get64(reg_name);
        }
        if (mem_op.index_reg) {
            const std::string& reg_name = arch.get_register_name(*mem_op.index_reg);
            uint64_t index_val = regs.get64(reg_name);
            addr += index_val * mem_op.scale;
        }

        switch (mem_op.size) {
            case 8:   return mem.read_byte(addr);
            case 16:  return mem.read_word(addr);
            case 32:  return mem.read_dword(addr);
            case 64:  return mem.read_qword(addr);
            default:
                throw std::runtime_error("Unsupported memory access size in getOperandValue: " + std::to_string(mem_op.size));
        }
    }
    return 0;
}

/**
 * @brief Sets the value of an abstract IR register using the architecture map.
 */
void setRegisterValue(const IRRegister& reg, uint64_t value, X86Simulator& simulator) {
    const auto& arch = simulator.get_architecture();
    const std::string& reg_name = arch.get_register_name(reg);
    auto& regs = simulator.getRegisterMap();

    switch (reg.size) {
        case 8:   regs.set8(reg_name, value); break;
        case 16:  regs.set16(reg_name, value); break;
        case 32:  regs.set32(reg_name, value); break;
        case 64:  regs.set64(reg_name, value); break;
        default:
            throw std::runtime_error("Unsupported register size in setRegisterValue: " + std::to_string(reg.size));
    }
}

void setMemoryValue(const IRMemoryOperand& mem_op, uint64_t value, X86Simulator& simulator) {
    auto& regs = simulator.getRegisterMap();
    auto& mem = simulator.getMemory();
    const auto& arch = simulator.get_architecture();

    address_t addr = mem_op.displacement;
    if (mem_op.base_reg) {
        const std::string& reg_name = arch.get_register_name(*mem_op.base_reg);
        addr += regs.get64(reg_name);
    }
    if (mem_op.index_reg) {
        const std::string& reg_name = arch.get_register_name(*mem_op.index_reg);
        uint64_t index_val = regs.get64(reg_name);
        addr += index_val * mem_op.scale;
    }

    switch (mem_op.size) {
        case 8:   mem.write_byte(addr, value); break;
        case 16:  mem.write_word(addr, value); break;
        case 32:  mem.write_dword(addr, value); break;
        case 64:  mem.write_qword(addr, value); break;
        default:
            throw std::runtime_error("Unsupported memory access size in setMemoryValue: " + std::to_string(mem_op.size));
    }
}

/**
 * @brief Executes an IR 'Add' instruction.
 */
void handle_ir_add(const IRInstruction& ir_instr, X86Simulator& simulator) {
    if (ir_instr.operands.size() != 2) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Invalid number of operands for IR Add", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];

    if (!std::holds_alternative<IRRegister>(dest_op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Add requires a register destination.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_reg = std::get<IRRegister>(dest_op);

    switch (dest_reg.size) {
        case 8: {
            uint8_t destValue = getOperandValue(dest_op, simulator);
            uint8_t sourceValue = getOperandValue(src_op, simulator);
            uint8_t result = destValue + sourceValue;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80) != 0);
            simulator.set_CF(static_cast<uint16_t>(destValue) + static_cast<uint16_t>(sourceValue) > 0xFF);
            // TODO: Set OF, AF, PF for 8-bit
            break;
        }
        case 16: {
            uint16_t destValue = getOperandValue(dest_op, simulator);
            uint16_t sourceValue = getOperandValue(src_op, simulator);
            uint16_t result = destValue + sourceValue;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000) != 0);
            simulator.set_CF(static_cast<uint32_t>(destValue) + static_cast<uint32_t>(sourceValue) > 0xFFFF);
            // TODO: Set OF, AF, PF for 16-bit
            break;
        }
        case 32: {
            uint32_t destValue = getOperandValue(dest_op, simulator);
            uint32_t sourceValue = getOperandValue(src_op, simulator);
            uint32_t result = destValue + sourceValue;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80000000) != 0);
            simulator.set_CF(static_cast<uint64_t>(destValue) + static_cast<uint64_t>(sourceValue) > 0xFFFFFFFF);
            // TODO: Set OF, AF, PF for 32-bit
            break;
        }
        case 64: {
            uint64_t destValue = getOperandValue(dest_op, simulator);
            uint64_t sourceValue = getOperandValue(src_op, simulator);
            uint64_t result = destValue + sourceValue;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000000000000000ULL) != 0);
            simulator.set_CF(result < destValue); // If result wrapped around, it will be less than the original.
            // TODO: Set OF, AF, PF for 64-bit
            break;
        }
        default:
            simulator.getDatabaseManager().log(simulator.get_session_id(), "Unsupported register size for IR Add", "ERROR", 0, __FILE__, __LINE__);
            break;
    }
}

void handle_ir_sub(const IRInstruction& ir_instr, X86Simulator& simulator) {
    if (ir_instr.operands.size() != 2) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Invalid number of operands for IR Sub", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];

    if (!std::holds_alternative<IRRegister>(dest_op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Sub requires a register destination.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_reg = std::get<IRRegister>(dest_op);

    switch (dest_reg.size) {
        case 8: {
            uint8_t val1 = getOperandValue(dest_op, simulator);
            uint8_t val2 = getOperandValue(src_op, simulator);
            uint8_t result = val1 - val2;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80) != 0);
            simulator.set_CF(val1 < val2);
            // TODO: Set OF, AF, PF for 8-bit
            break;
        }
        case 16: {
            uint16_t val1 = getOperandValue(dest_op, simulator);
            uint16_t val2 = getOperandValue(src_op, simulator);
            uint16_t result = val1 - val2;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000) != 0);
            simulator.set_CF(val1 < val2);
            // TODO: Set OF, AF, PF for 16-bit
            break;
        }
        case 32: {
            uint32_t val1 = getOperandValue(dest_op, simulator);
            uint32_t val2 = getOperandValue(src_op, simulator);
            uint32_t result = val1 - val2;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80000000) != 0);
            simulator.set_CF(val1 < val2);
            // TODO: Set OF, AF, PF for 32-bit
            break;
        }
        case 64: {
            uint64_t val1 = getOperandValue(dest_op, simulator);
            uint64_t val2 = getOperandValue(src_op, simulator);
            uint64_t result = val1 - val2;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000000000000000ULL) != 0);
            simulator.set_CF(val1 < val2);
            // TODO: Set OF, AF, PF for 64-bit
            break;
        }
        default:
            simulator.getDatabaseManager().log(simulator.get_session_id(), "Unsupported register size for IR Sub", "ERROR", 0, __FILE__, __LINE__);
            break;
    }
}

/**
 * @brief Executes an IR 'Move' instruction (register to register).
 */
void handle_ir_move(const IRInstruction& ir_instr, X86Simulator& simulator) {
    if (ir_instr.operands.size() != 2) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Invalid number of operands for IR Move", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];

    if (!std::holds_alternative<IRRegister>(dest_op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Move requires a register destination.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_reg = std::get<IRRegister>(dest_op);
    uint64_t sourceValue = getOperandValue(src_op, simulator);

    setRegisterValue(dest_reg, sourceValue, simulator);
}

/**
 * @brief Executes an IR 'Load' instruction (memory to register).
 */
void handle_ir_load(const IRInstruction& ir_instr, X86Simulator& simulator) {
    if (ir_instr.operands.size() != 2) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Invalid number of operands for IR Load", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];

    if (!std::holds_alternative<IRRegister>(dest_op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Load requires a register destination.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }
    if (!std::holds_alternative<IRMemoryOperand>(src_op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Load requires a memory source.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_reg = std::get<IRRegister>(dest_op);
    uint64_t sourceValue = getOperandValue(src_op, simulator);

    setRegisterValue(dest_reg, sourceValue, simulator);
}

/**
 * @brief Executes an IR 'Store' instruction (register to memory).
 */
void handle_ir_store(const IRInstruction& ir_instr, X86Simulator& simulator) {
    if (ir_instr.operands.size() != 2) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Invalid number of operands for IR Store", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];

    if (!std::holds_alternative<IRMemoryOperand>(dest_op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Store requires a memory destination.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_mem = std::get<IRMemoryOperand>(dest_op);
    uint64_t sourceValue = getOperandValue(src_op, simulator);

    setMemoryValue(dest_mem, sourceValue, simulator);
}

/**
 * @brief Executes an IR 'Jump' instruction.
 */
void handle_ir_jump(const IRInstruction& ir_instr, X86Simulator& simulator) {
    if (ir_instr.operands.size() != 1) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Invalid number of operands for IR Jump", "ERROR", 0, __FILE__, __LINE__);
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
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Jump target is not a valid address.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    // Directly set the instruction pointer.
    simulator.getRegisterMap().set64("rip", target_address);
}

/**
 * @brief Executes an IR 'Branch' instruction based on a condition.
 */
void handle_ir_branch(const IRInstruction& ir_instr, X86Simulator& simulator) {
    if (ir_instr.operands.size() != 2) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Invalid number of operands for IR Branch", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& target_op = ir_instr.operands[0];
    const auto& cond_op = ir_instr.operands[1];

    // --- 1. Get Target Address ---
    address_t target_address = 0;
    if (std::holds_alternative<uint64_t>(target_op)) {
        target_address = std::get<uint64_t>(target_op);
    } else {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Branch target is not a valid address.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    // --- 2. Evaluate Condition ---
    if (!std::holds_alternative<IRConditionCode>(cond_op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Branch condition is not a valid IRConditionCode.", "ERROR", 0, __FILE__, __LINE__);
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
            simulator.getDatabaseManager().log(simulator.get_session_id(), "Unsupported IR branch condition.", "WARNING", 0, __FILE__, __LINE__);
            return;
    }

    // --- 3. Perform Jump if Condition is Met ---
    if (should_jump) {
        simulator.getRegisterMap().set64("rip", target_address);
    }
    // If the condition is not met, do nothing and let the IP advance normally.
}

/**
 * @brief Executes an IR 'Cmp' instruction, which performs a subtraction
 *        and updates flags without storing the result.
 */
void handle_ir_cmp(const IRInstruction& ir_instr, X86Simulator& simulator) {
    if (ir_instr.operands.size() != 2) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Invalid number of operands for IR Cmp", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& op1 = ir_instr.operands[0];
    const auto& op2 = ir_instr.operands[1];

    // Determine size from the first operand, assuming they match.
    uint32_t size = 0;
    if (const IRRegister* reg = std::get_if<IRRegister>(&op1)) {
        size = reg->size;
    } else if (const IRMemoryOperand* mem = std::get_if<IRMemoryOperand>(&op1)) {
        size = mem->size;
    } else { // Should not happen if IR is well-formed
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Invalid first operand for IR Cmp", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    switch (size) {
        case 8: {
            uint8_t val1 = getOperandValue(op1, simulator);
            uint8_t val2 = getOperandValue(op2, simulator);
            uint8_t result = val1 - val2;
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80) != 0);
            simulator.set_CF(val1 < val2);
            break;
        }
        case 16: {
            uint16_t val1 = getOperandValue(op1, simulator);
            uint16_t val2 = getOperandValue(op2, simulator);
            uint16_t result = val1 - val2;
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000) != 0);
            simulator.set_CF(val1 < val2);
            break;
        }
        case 32: {
            uint32_t val1 = getOperandValue(op1, simulator);
            uint32_t val2 = getOperandValue(op2, simulator);
            uint32_t result = val1 - val2;
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80000000) != 0);
            simulator.set_CF(val1 < val2);
            break;
        }
        case 64: {
            uint64_t val1 = getOperandValue(op1, simulator);
            uint64_t val2 = getOperandValue(op2, simulator);
            uint64_t result = val1 - val2;
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000000000000000ULL) != 0);
            simulator.set_CF(val1 < val2);
            break;
        }
        default:
            simulator.getDatabaseManager().log(simulator.get_session_id(), "Unsupported operand size for IR Cmp", "ERROR", 0, __FILE__, __LINE__);
            break;
    }
}

/**
 * @brief Executes an IR 'Inc' operation, which is a special case of 'Add'.
 *        It increments an operand by 1 and updates flags, but does NOT affect the Carry Flag (CF).
 */
void handle_ir_inc(const IRInstruction& ir_instr, X86Simulator& simulator) {
    if (ir_instr.operands.size() != 1) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Invalid number of operands for IR Inc", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& op = ir_instr.operands[0];

    if (!std::holds_alternative<IRRegister>(op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Inc requires a register destination.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_reg = std::get<IRRegister>(op);

    // Assuming 32-bit for this example.
    uint32_t value = getOperandValue(op, simulator);
    uint32_t result = value + 1;

    setRegisterValue(dest_reg, result, simulator);

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
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Syscall requires one operand (the interrupt vector).", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& vector_op = ir_instr.operands[0];
    if (!std::holds_alternative<uint64_t>(vector_op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Syscall operand must be an immediate value.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    uint8_t interrupt_vector = std::get<uint64_t>(vector_op);

    if (interrupt_vector == 0x80) { // Linux syscall convention
        auto& regs = simulator.getRegisterMap();
        uint32_t syscall_num = regs.get32("eax");

        switch (syscall_num) {
            case 1: { // sys_exit
                uint32_t exit_code = regs.get32("ebx");
                std::string logMessage = "Program exited via sys_exit with code: " + std::to_string(exit_code);
                simulator.getDatabaseManager().log(simulator.get_session_id(), logMessage, "INFO", 0, __FILE__, __LINE__);
                
                // In a real implementation, you would set a flag to halt the simulator.
                // For now, we can simulate this by setting RIP to a high value to stop the loop.
                regs.set64("rip", simulator.getMemory().get_total_memory_size());
                break;
            }
            default: {
                simulator.getDatabaseManager().log(simulator.get_session_id(), "Unsupported syscall: " + std::to_string(syscall_num), "WARNING", 0, __FILE__, __LINE__);
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
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Mul (one-operand) requires one operand.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    auto& regs = simulator.getRegisterMap();
    auto& mem = simulator.getMemory();

    const auto& src_op = ir_instr.operands[0];

    // Get the source value (assuming 32-bit for this operation)
    uint32_t src_val = getOperandValue(src_op, simulator);

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
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR IMul (one-operand) requires one operand.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    auto& regs = simulator.getRegisterMap();
    auto& mem = simulator.getMemory();

    const auto& src_op = ir_instr.operands[0];

    // Get the source value as a signed 32-bit integer
    int32_t src_val = getOperandValue(src_op, simulator);

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
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Invalid number of operands for IR Dec", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& op = ir_instr.operands[0];

    if (!std::holds_alternative<IRRegister>(op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Dec requires a register destination.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_reg = std::get<IRRegister>(op);

    // Assuming 32-bit for this example.
    uint32_t value = getOperandValue(op, simulator);
    uint32_t result = value - 1;

    setRegisterValue(dest_reg, result, simulator);

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

void handle_ir_call(const IRInstruction& ir_instr, X86Simulator& simulator) {
    // 1. Get the target address from the operand
    const auto& target_op = ir_instr.operands[0];
    address_t target_address = std::get<uint64_t>(target_op);

    // 2. Calculate the return address
    address_t return_address = ir_instr.original_address + ir_instr.original_size;

    // 3. Push the return address onto the stack
    auto& regs = simulator.getRegisterMap();
    auto& mem = simulator.getMemory();
    
    // Decrement stack pointer
    address_t rsp = regs.get64("rsp");
    rsp -= 8; // Assuming a 64-bit stack
    regs.set64("rsp", rsp);

    // Write return address to the stack
    mem.write_qword(rsp, return_address);

    // 4. Set RIP to the target address
    regs.set64("rip", target_address);
}

void handle_ir_xor(const IRInstruction& ir_instr, X86Simulator& simulator) {
    if (ir_instr.operands.size() != 2) { return; }
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    if (!std::holds_alternative<IRRegister>(dest_op)) { return; }
    const auto& dest_reg = std::get<IRRegister>(dest_op);

    // Clear flags
    simulator.set_CF(false);
    simulator.set_OF(false);

    switch (dest_reg.size) {
        case 8: {
            uint8_t val1 = getOperandValue(dest_op, simulator);
            uint8_t val2 = getOperandValue(src_op, simulator);
            uint8_t result = val1 ^ val2;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80) != 0);
            break;
        }
        case 16: {
            uint16_t val1 = getOperandValue(dest_op, simulator);
            uint16_t val2 = getOperandValue(src_op, simulator);
            uint16_t result = val1 ^ val2;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000) != 0);
            break;
        }
        case 32: {
            uint32_t val1 = getOperandValue(dest_op, simulator);
            uint32_t val2 = getOperandValue(src_op, simulator);
            uint32_t result = val1 ^ val2;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80000000) != 0);
            break;
        }
        case 64: {
            uint64_t val1 = getOperandValue(dest_op, simulator);
            uint64_t val2 = getOperandValue(src_op, simulator);
            uint64_t result = val1 ^ val2;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000000000000000ULL) != 0);
            break;
        }
        default: return;
    }
    // TODO: Set Parity Flag
}

void handle_ir_and(const IRInstruction& ir_instr, X86Simulator& simulator) {
    if (ir_instr.operands.size() != 2) { return; }
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    if (!std::holds_alternative<IRRegister>(dest_op)) { return; }
    const auto& dest_reg = std::get<IRRegister>(dest_op);

    // Clear flags
    simulator.set_CF(false);
    simulator.set_OF(false);

    switch (dest_reg.size) {
        case 8: {
            uint8_t val1 = getOperandValue(dest_op, simulator);
            uint8_t val2 = getOperandValue(src_op, simulator);
            uint8_t result = val1 & val2;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80) != 0);
            break;
        }
        case 16: {
            uint16_t val1 = getOperandValue(dest_op, simulator);
            uint16_t val2 = getOperandValue(src_op, simulator);
            uint16_t result = val1 & val2;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000) != 0);
            break;
        }
        case 32: {
            uint32_t val1 = getOperandValue(dest_op, simulator);
            uint32_t val2 = getOperandValue(src_op, simulator);
            uint32_t result = val1 & val2;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80000000) != 0);
            break;
        }
        case 64: {
            uint64_t val1 = getOperandValue(dest_op, simulator);
            uint64_t val2 = getOperandValue(src_op, simulator);
            uint64_t result = val1 & val2;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000000000000000ULL) != 0);
            break;
        }
        default: return;
    }
    // TODO: Set Parity Flag
}

void handle_ir_or(const IRInstruction& ir_instr, X86Simulator& simulator) {
    if (ir_instr.operands.size() != 2) { return; }
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    if (!std::holds_alternative<IRRegister>(dest_op)) { return; }
    const auto& dest_reg = std::get<IRRegister>(dest_op);

    // Clear flags
    simulator.set_CF(false);
    simulator.set_OF(false);

    switch (dest_reg.size) {
        case 8: {
            uint8_t val1 = getOperandValue(dest_op, simulator);
            uint8_t val2 = getOperandValue(src_op, simulator);
            uint8_t result = val1 | val2;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80) != 0);
            break;
        }
        case 16: {
            uint16_t val1 = getOperandValue(dest_op, simulator);
            uint16_t val2 = getOperandValue(src_op, simulator);
            uint16_t result = val1 | val2;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000) != 0);
            break;
        }
        case 32: {
            uint32_t val1 = getOperandValue(dest_op, simulator);
            uint32_t val2 = getOperandValue(src_op, simulator);
            uint32_t result = val1 | val2;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80000000) != 0);
            break;
        }
        case 64: {
            uint64_t val1 = getOperandValue(dest_op, simulator);
            uint64_t val2 = getOperandValue(src_op, simulator);
            uint64_t result = val1 | val2;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000000000000000ULL) != 0);
            break;
        }
        default: return;
    }
    // TODO: Set Parity Flag
}

void handle_ir_not(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& dest_reg = std::get<IRRegister>(dest_op);

    uint32_t destValue = getOperandValue(dest_op, simulator);
    uint32_t result = ~destValue;
    setRegisterValue(dest_reg, result, simulator);
}

void handle_ir_shl(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& count_op = ir_instr.operands[1];
    const auto& dest_reg = std::get<IRRegister>(dest_op);

    uint32_t destValue = getOperandValue(dest_op, simulator);
    uint8_t count = getOperandValue(count_op, simulator);
    uint32_t result = destValue << count;
    setRegisterValue(dest_reg, result, simulator);

    simulator.set_ZF(result == 0);
    simulator.set_SF((result & 0x80000000) != 0);
    if (count > 0) {
        simulator.set_CF((destValue >> (32 - count)) & 1);
    }
    // OF is only affected on 1-bit shifts
    if (count == 1) {
        simulator.set_OF(((result >> 31) & 1) != simulator.get_CF());
    }

    // PF
    uint8_t lsb = result & 0xFF;
    int set_bits = 0;
    for (int i = 0; i < 8; ++i) { if ((lsb >> i) & 1) { set_bits++; } }
    simulator.set_PF((set_bits % 2) == 0);
}

void handle_ir_shr(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& count_op = ir_instr.operands[1];
    const auto& dest_reg = std::get<IRRegister>(dest_op);

    uint32_t destValue = getOperandValue(dest_op, simulator);
    uint8_t count = getOperandValue(count_op, simulator);
    uint32_t result = destValue >> count;
    setRegisterValue(dest_reg, result, simulator);

    simulator.set_ZF(result == 0);
    simulator.set_SF((result & 0x80000000) != 0);
    if (count > 0) {
        simulator.set_CF((destValue >> (count - 1)) & 1);
    }
    // OF is only affected on 1-bit shifts
    if (count == 1) {
        simulator.set_OF((destValue & 0x80000000) != 0);
    }

    // PF
    uint8_t lsb = result & 0xFF;
    int set_bits = 0;
    for (int i = 0; i < 8; ++i) { if ((lsb >> i) & 1) { set_bits++; } }
    simulator.set_PF((set_bits % 2) == 0);
}

void handle_ir_sar(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& count_op = ir_instr.operands[1];
    const auto& dest_reg = std::get<IRRegister>(dest_op);

    int32_t destValue = getOperandValue(dest_op, simulator);
    uint8_t count = getOperandValue(count_op, simulator);
    int32_t result = destValue >> count;
    setRegisterValue(dest_reg, result, simulator);

    simulator.set_ZF(result == 0);
    simulator.set_SF(result < 0);
    if (count > 0) {
        simulator.set_CF((destValue >> (count - 1)) & 1);
    }
    if (count == 1) {
        simulator.set_OF(false);
    }

    // PF
    uint8_t lsb = result & 0xFF;
    int set_bits = 0;
    for (int i = 0; i < 8; ++i) { if ((lsb >> i) & 1) { set_bits++; } }
    simulator.set_PF((set_bits % 2) == 0);
}

void handle_ir_packed_and(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    const auto& dest_reg = std::get<IRRegister>(dest_op);
    const auto& src_reg = std::get<IRRegister>(src_op);

    auto& regs = simulator.getRegisterMap();
    const auto& arch = simulator.get_architecture();

    const std::string& dest_reg_name = arch.get_register_name(dest_reg);
    const std::string& src_reg_name = arch.get_register_name(src_reg);

    m256i_t dest_val = regs.getYmm(dest_reg_name);
    m256i_t src_val = regs.getYmm(src_reg_name);

    m256i_t result;
    result.m128[0] = _mm_and_si128_sim(dest_val.m128[0], src_val.m128[0]);
    result.m128[1] = _mm_and_si128_sim(dest_val.m128[1], src_val.m128[1]);

    regs.setYmm(dest_reg_name, result);
}

void handle_ir_packed_and_not(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    const auto& dest_reg = std::get<IRRegister>(dest_op);
    const auto& src_reg = std::get<IRRegister>(src_op);

    auto& regs = simulator.getRegisterMap();
    const auto& arch = simulator.get_architecture();

    const std::string& dest_reg_name = arch.get_register_name(dest_reg);
    const std::string& src_reg_name = arch.get_register_name(src_reg);

    m256i_t dest_val = regs.getYmm(dest_reg_name);
    m256i_t src_val = regs.getYmm(src_reg_name);

    m256i_t result;
    result.m128[0] = _mm_andnot_si128_sim(dest_val.m128[0], src_val.m128[0]);
    result.m128[1] = _mm_andnot_si128_sim(dest_val.m128[1], src_val.m128[1]);

    regs.setYmm(dest_reg_name, result);
}

void handle_ir_packed_or(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    const auto& dest_reg = std::get<IRRegister>(dest_op);
    const auto& src_reg = std::get<IRRegister>(src_op);

    auto& regs = simulator.getRegisterMap();
    const auto& arch = simulator.get_architecture();

    const std::string& dest_reg_name = arch.get_register_name(dest_reg);
    const std::string& src_reg_name = arch.get_register_name(src_reg);

    m256i_t dest_val = regs.getYmm(dest_reg_name);
    m256i_t src_val = regs.getYmm(src_reg_name);

    m256i_t result;
    result.m128[0] = _mm_or_si128_sim(dest_val.m128[0], src_val.m128[0]);
    result.m128[1] = _mm_or_si128_sim(dest_val.m128[1], src_val.m128[1]);

    regs.setYmm(dest_reg_name, result);
}

void handle_ir_packed_xor(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    const auto& dest_reg = std::get<IRRegister>(dest_op);
    const auto& src_reg = std::get<IRRegister>(src_op);

    auto& regs = simulator.getRegisterMap();
    const auto& arch = simulator.get_architecture();

    const std::string& dest_reg_name = arch.get_register_name(dest_reg);
    const std::string& src_reg_name = arch.get_register_name(src_reg);

    m256i_t dest_val = regs.getYmm(dest_reg_name);
    m256i_t src_val = regs.getYmm(src_reg_name);

    m256i_t result;
    result.m128[0] = _mm_xor_si128_sim(dest_val.m128[0], src_val.m128[0]);
    result.m128[1] = _mm_xor_si128_sim(dest_val.m128[1], src_val.m128[1]);

    regs.setYmm(dest_reg_name, result);
}

void handle_ir_packed_add_ps(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    const auto& dest_reg = std::get<IRRegister>(dest_op);
    const auto& src_reg = std::get<IRRegister>(src_op);

    auto& regs = simulator.getRegisterMap();
    const auto& arch = simulator.get_architecture();

    const std::string& dest_reg_name = arch.get_register_name(dest_reg);
    const std::string& src_reg_name = arch.get_register_name(src_reg);

    m256i_t dest_val = regs.getYmm(dest_reg_name);
    m256i_t src_val = regs.getYmm(src_reg_name);

    m256i_t result = _mm256_add_ps_sim(dest_val, src_val);

    regs.setYmm(dest_reg_name, result);
}

void handle_ir_packed_sub_ps(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    const auto& dest_reg = std::get<IRRegister>(dest_op);
    const auto& src_reg = std::get<IRRegister>(src_op);

    auto& regs = simulator.getRegisterMap();
    const auto& arch = simulator.get_architecture();

    const std::string& dest_reg_name = arch.get_register_name(dest_reg);
    const std::string& src_reg_name = arch.get_register_name(src_reg);

    m256i_t dest_val = regs.getYmm(dest_reg_name);
    m256i_t src_val = regs.getYmm(src_reg_name);

    m256i_t result = _mm256_sub_ps_sim(dest_val, src_val);

    regs.setYmm(dest_reg_name, result);
}

void handle_ir_packed_mul_ps(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    const auto& dest_reg = std::get<IRRegister>(dest_op);
    const auto& src_reg = std::get<IRRegister>(src_op);

    auto& regs = simulator.getRegisterMap();
    const auto& arch = simulator.get_architecture();

    const std::string& dest_reg_name = arch.get_register_name(dest_reg);
    const std::string& src_reg_name = arch.get_register_name(src_reg);

    m256i_t dest_val = regs.getYmm(dest_reg_name);
    m256i_t src_val = regs.getYmm(src_reg_name);

    m256i_t result = _mm256_mul_ps_sim(dest_val, src_val);

    regs.setYmm(dest_reg_name, result);
}

void handle_ir_packed_div_ps(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    const auto& dest_reg = std::get<IRRegister>(dest_op);
    const auto& src_reg = std::get<IRRegister>(src_op);

    auto& regs = simulator.getRegisterMap();
    const auto& arch = simulator.get_architecture();

    const std::string& dest_reg_name = arch.get_register_name(dest_reg);
    const std::string& src_reg_name = arch.get_register_name(src_reg);

    m256i_t dest_val = regs.getYmm(dest_reg_name);
    m256i_t src_val = regs.getYmm(src_reg_name);

    m256i_t result = _mm256_div_ps_sim(dest_val, src_val);

    regs.setYmm(dest_reg_name, result);
}

void handle_ir_packed_max_ps(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    const auto& dest_reg = std::get<IRRegister>(dest_op);
    const auto& src_reg = std::get<IRRegister>(src_op);

    auto& regs = simulator.getRegisterMap();
    const auto& arch = simulator.get_architecture();

    const std::string& dest_reg_name = arch.get_register_name(dest_reg);
    const std::string& src_reg_name = arch.get_register_name(src_reg);

    m256i_t dest_val = regs.getYmm(dest_reg_name);
    m256i_t src_val = regs.getYmm(src_reg_name);

    m256i_t result = _mm256_max_ps_sim(dest_val, src_val);

    regs.setYmm(dest_reg_name, result);
}

void handle_ir_packed_min_ps(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    const auto& dest_reg = std::get<IRRegister>(dest_op);
    const auto& src_reg = std::get<IRRegister>(src_op);

    auto& regs = simulator.getRegisterMap();
    const auto& arch = simulator.get_architecture();

    const std::string& dest_reg_name = arch.get_register_name(dest_reg);
    const std::string& src_reg_name = arch.get_register_name(src_reg);

    m256i_t dest_val = regs.getYmm(dest_reg_name);
    m256i_t src_val = regs.getYmm(src_reg_name);

    m256i_t result = _mm256_min_ps_sim(dest_val, src_val);

    regs.setYmm(dest_reg_name, result);
}

void handle_ir_packed_sqrt_ps(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    const auto& dest_reg = std::get<IRRegister>(dest_op);
    const auto& src_reg = std::get<IRRegister>(src_op);

    auto& regs = simulator.getRegisterMap();
    const auto& arch = simulator.get_architecture();

    const std::string& dest_reg_name = arch.get_register_name(dest_reg);
    const std::string& src_reg_name = arch.get_register_name(src_reg);

    m256i_t src_val = regs.getYmm(src_reg_name);

    m256i_t result = _mm256_sqrt_ps_sim(src_val);

    regs.setYmm(dest_reg_name, result);
}

void handle_ir_packed_reciprocal_ps(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    const auto& dest_reg = std::get<IRRegister>(dest_op);
    const auto& src_reg = std::get<IRRegister>(src_op);

    auto& regs = simulator.getRegisterMap();
    const auto& arch = simulator.get_architecture();

    const std::string& dest_reg_name = arch.get_register_name(dest_reg);
    const std::string& src_reg_name = arch.get_register_name(src_reg);

    m256i_t src_val = regs.getYmm(src_reg_name);

    m256i_t result = _mm256_rcp_ps_sim(src_val);

    regs.setYmm(dest_reg_name, result);
}

void handle_ir_packed_mul_low_i16(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    const auto& dest_reg = std::get<IRRegister>(dest_op);
    const auto& src_reg = std::get<IRRegister>(src_op);

    auto& regs = simulator.getRegisterMap();
    const auto& arch = simulator.get_architecture();

    const std::string& dest_reg_name = arch.get_register_name(dest_reg);
    const std::string& src_reg_name = arch.get_register_name(src_reg);

    m256i_t dest_val = regs.getYmm(dest_reg_name);
    m256i_t src_val = regs.getYmm(src_reg_name);

    m256i_t result;
    result.m128[0] = _mm_mullo_epi16_sim(dest_val.m128[0], src_val.m128[0]);
    result.m128[1] = _mm_mullo_epi16_sim(dest_val.m128[1], src_val.m128[1]);

    regs.setYmm(dest_reg_name, result);
}

void handle_ir_vector_zero(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& dest_reg = std::get<IRRegister>(dest_op);

    auto& regs = simulator.getRegisterMap();
    const auto& arch = simulator.get_architecture();

    const std::string& dest_reg_name = arch.get_register_name(dest_reg);

    m256i_t result = _mm256_setzero_si256_sim();

    regs.setYmm(dest_reg_name, result);
}

void handle_ir_ret(const IRInstruction& ir_instr, X86Simulator& simulator) {
    auto& regs = simulator.getRegisterMap();
    auto& mem = simulator.getMemory();

    address_t rsp = regs.get64("rsp");
    address_t return_address = mem.read_qword(rsp);
    regs.set64("rsp", rsp + 8);
    regs.set64("rip", return_address);
}

void handle_ir_div(const IRInstruction& ir_instr, X86Simulator& simulator) {
    auto& regs = simulator.getRegisterMap();
    const auto& src_op = ir_instr.operands[0];

    uint32_t size = 0;
    if (const IRRegister* reg = std::get_if<IRRegister>(&src_op)) {
        size = reg->size;
    } else if (const IRMemoryOperand* mem = std::get_if<IRMemoryOperand>(&src_op)) {
        size = mem->size;
    } else {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Invalid source operand for IR Div", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    auto halt_for_exception = [&]() {
        simulator.getDatabaseManager().log(simulator.get_session_id(),
					   "Divide Error Exception (#DE)", "ERROR", regs.get64("rip"), __FILE__, __LINE__);
        regs.set64("rip", simulator.getMemory().get_total_memory_size());
    };

    switch (size) {
        case 8: {
            uint8_t divisor = getOperandValue(src_op, simulator);
            if (divisor == 0) { return halt_for_exception(); }
            uint16_t dividend = regs.get16("ax");
            uint16_t quotient = dividend / divisor;
            if (quotient > 0xFF) { return halt_for_exception(); } // Check for overflow
            uint8_t remainder = dividend % divisor;
            regs.set8("al", quotient);
            regs.set8("ah", remainder);
            break;
        }
        case 16: {
            uint16_t divisor = getOperandValue(src_op, simulator);
            if (divisor == 0) { return halt_for_exception(); }
            uint32_t dividend = (static_cast<uint32_t>(regs.get16("dx")) << 16) | regs.get16("ax");
            uint32_t quotient = dividend / divisor;
            if (quotient > 0xFFFF) { return halt_for_exception(); } // Check for overflow
            uint16_t remainder = dividend % divisor;
            regs.set16("ax", quotient);
            regs.set16("dx", remainder);
            break;
        }
        case 32: {
            uint32_t divisor = getOperandValue(src_op, simulator);
            if (divisor == 0) { return halt_for_exception(); }
            uint64_t dividend = (static_cast<uint64_t>(regs.get32("edx")) << 32) | regs.get32("eax");
            uint64_t quotient = dividend / divisor;
            if (quotient > 0xFFFFFFFF) { return halt_for_exception(); } // Check for overflow
            uint32_t remainder = dividend % divisor;
            regs.set32("eax", quotient);
            regs.set32("edx", remainder);
            break;
        }
        case 64: {
            uint64_t divisor = getOperandValue(src_op, simulator);
            if (divisor == 0) { return halt_for_exception(); }
            unsigned __int128 dividend = (static_cast<unsigned __int128>(regs.get64("rdx")) << 64) | regs.get64("rax");
            unsigned __int128 quotient = dividend / divisor;
            if (quotient > 0xFFFFFFFFFFFFFFFF) { return halt_for_exception(); } // Check for overflow
            uint64_t remainder = dividend % divisor;
            regs.set64("rax", quotient);
            regs.set64("rdx", remainder);
            break;
        }
        default:
            simulator.getDatabaseManager().log(simulator.get_session_id(), "Unsupported operand size for IR Div", "ERROR", 0, __FILE__, __LINE__);
            break;
    }
    // DIV instruction leaves flags undefined.
}
