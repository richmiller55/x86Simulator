#include "ir_executor_helpers.h"
#include "i_simulator.h"
#include "register_map.h"
#include "memory.h"
#include "i_database_manager.h"
#include "INTEL_helpers.h"
#include "x86_simulator.h"
#include <variant>



/**
 * @brief Gets the value of an IR operand by using the architecture map.
 */
uint64_t getOperandValue(const IROperand& op, ISimulator& simulator) {
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
void setRegisterValue(const IRRegister& reg, uint64_t value, ISimulator& simulator) {
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

void setMemoryValue(const IRMemoryOperand& mem_op, uint64_t value, ISimulator& simulator) {
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
void handle_ir_add(const IRInstruction& ir_instr, ISimulator& simulator) {
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

void handle_ir_sub(const IRInstruction& ir_instr, ISimulator& simulator) {
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
void handle_ir_move(const IRInstruction& ir_instr, ISimulator& simulator) {
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
void handle_ir_load(const IRInstruction& ir_instr, ISimulator& simulator) {
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
void handle_ir_store(const IRInstruction& ir_instr, ISimulator& simulator) {
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
void handle_ir_jump(const IRInstruction& ir_instr, ISimulator& simulator) {
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
void handle_ir_branch(const IRInstruction& ir_instr, ISimulator& simulator) {
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
        case IRConditionCode::Equal: // JE, JZ
            should_jump = simulator.get_ZF();
            break;
        case IRConditionCode::NotEqual: // JNE, JNZ
            should_jump = !simulator.get_ZF();
            break;
        case IRConditionCode::Greater: // JG, JNLE
            should_jump = !simulator.get_ZF() && (simulator.get_SF() == simulator.get_OF());
            break;
        case IRConditionCode::GreaterOrEqual: // JGE, JNL
            should_jump = (simulator.get_SF() == simulator.get_OF());
            break;
        case IRConditionCode::Less: // JL, JNGE
            should_jump = (simulator.get_SF() != simulator.get_OF());
            break;
        case IRConditionCode::LessOrEqual: // JLE, JNG
            should_jump = simulator.get_ZF() || (simulator.get_SF() != simulator.get_OF());
            break;
        case IRConditionCode::Above: // JA, JNBE
            should_jump = !simulator.get_CF() && !simulator.get_ZF();
            break;
        case IRConditionCode::AboveOrEqual: // JAE, JNB, JNC
            should_jump = !simulator.get_CF();
            break;
        case IRConditionCode::Below: // JB, JNAE, JC
            should_jump = simulator.get_CF();
            break;
        case IRConditionCode::BelowOrEqual: // JBE, JNA
            should_jump = simulator.get_CF() || simulator.get_ZF();
            break;
        case IRConditionCode::Overflow: // JO
            should_jump = simulator.get_OF();
            break;
        case IRConditionCode::NotOverflow: // JNO
            should_jump = !simulator.get_OF();
            break;
        case IRConditionCode::Sign: // JS
            should_jump = simulator.get_SF();
            break;
        case IRConditionCode::NotSign: // JNS
            should_jump = !simulator.get_SF();
            break;
        case IRConditionCode::ParityEven: // JP, JPE
            should_jump = simulator.get_PF();
            break;
        case IRConditionCode::ParityOdd: // JNP, JPO
            should_jump = !simulator.get_PF();
            break;
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
void handle_ir_cmp(const IRInstruction& ir_instr, ISimulator& simulator) {
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
void handle_ir_inc(const IRInstruction& ir_instr, ISimulator& simulator) {
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

    switch (dest_reg.size) {
        case 8: {
            uint8_t value = getOperandValue(op, simulator);
            uint8_t result = value + 1;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80) != 0);
            simulator.set_OF(value == 0x7F);
            break;
        }
        case 16: {
            uint16_t value = getOperandValue(op, simulator);
            uint16_t result = value + 1;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000) != 0);
            simulator.set_OF(value == 0x7FFF);
            break;
        }
        case 32: {
            uint32_t value = getOperandValue(op, simulator);
            uint32_t result = value + 1;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80000000) != 0);
            simulator.set_OF(value == 0x7FFFFFFF);
            break;
        }
        case 64: {
            uint64_t value = getOperandValue(op, simulator);
            uint64_t result = value + 1;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000000000000000ULL) != 0);
            simulator.set_OF(value == 0x7FFFFFFFFFFFFFFFULL);
            break;
        }
        default:
            simulator.getDatabaseManager().log(simulator.get_session_id(), "Unsupported operand size for IR Inc", "ERROR", 0, __FILE__, __LINE__);
            break;
    }
    // Note: INC does not affect the Carry Flag (CF).
}

/**
 * @brief Executes an IR 'Syscall' instruction, which maps to 'INT' on x86.
 */
void handle_ir_syscall(const IRInstruction& ir_instr, ISimulator& simulator) {
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
void handle_ir_mul(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 1) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Mul (one-operand) requires one operand.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    auto& regs = simulator.getRegisterMap();

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
void handle_ir_imul(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 1) {
        simulator.getDatabaseManager().log(simulator.get_session_id(),
					   "IR IMul (one-operand) requires one operand.",
					   "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    auto& regs = simulator.getRegisterMap();

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
void handle_ir_dec(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 1) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Invalid number of operands for IR Dec", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& op = ir_instr.operands[0];

    if (!std::holds_alternative<IRRegister>(op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(),
					   "IR Dec requires a register destination.", "ERROR",
					   0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_reg = std::get<IRRegister>(op);

    switch (dest_reg.size) {
        case 8: {
            uint8_t value = getOperandValue(op, simulator);
            uint8_t result = value - 1;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80) != 0);
            simulator.set_OF(value == 0x80);
            break;
        }
        case 16: {
            uint16_t value = getOperandValue(op, simulator);
            uint16_t result = value - 1;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000) != 0);
            simulator.set_OF(value == 0x8000);
            break;
        }
        case 32: {
            uint32_t value = getOperandValue(op, simulator);
            uint32_t result = value - 1;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80000000) != 0);
            simulator.set_OF(value == 0x80000000);
            break;
        }
        case 64: {
            uint64_t value = getOperandValue(op, simulator);
            uint64_t result = value - 1;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000000000000000ULL) != 0);
            simulator.set_OF(value == 0x8000000000000000ULL);
            break;
        }
        default:
            simulator.getDatabaseManager().log(simulator.get_session_id(), "Unsupported operand size for IR Dec", "ERROR", 0, __FILE__, __LINE__);
            break;
    }
    // Note: DEC does not affect the Carry Flag (CF).
}

void handle_ir_call(const IRInstruction& ir_instr, ISimulator& simulator) {
    // 1. Get the target address from the operand
    const auto& target_op = ir_instr.operands[0];
    address_t target_address = std::get<uint64_t>(target_op);

    // 2. Calculate the return address
    address_t return_address = ir_instr.original_address + ir_instr.original_size;

    // 3. Push the return address onto the stack
    auto& regs = simulator.getRegisterMap();
    auto& mem = simulator.getMemory();
    
    // For now, we'll assume a 32-bit stack for the call return address to match the assembly code.
    address_t rsp = regs.get64("rsp");
    rsp -= 4; 
    regs.set64("rsp", rsp);

    // Write return address to the stack
    mem.write_stack_dword(rsp, static_cast<uint32_t>(return_address));

    // 4. Set RIP to the target address
    regs.set64("rip", target_address);
}

void handle_ir_push(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 1) {
        return; // Or log error
    }

    const auto& src_op = ir_instr.operands[0];
    uint64_t value = getOperandValue(src_op, simulator);

    auto& regs = simulator.getRegisterMap();
    auto& mem = simulator.getMemory();
    address_t rsp = regs.get64("rsp");

    uint32_t push_size = 0;
    if (const IRRegister* reg = std::get_if<IRRegister>(&src_op)) {
        push_size = reg->size / 8; // size is in bits
    } else if (std::holds_alternative<uint64_t>(src_op)) {
        // Ambiguous, could be 32 or 64. Let's assume 32 for now as it's more common in the provided code.
        // A better solution would involve hints from the decoder.
        push_size = 4;
    }

    if (push_size == 4) {
        rsp -= 4;
        regs.set64("rsp", rsp);
        mem.write_stack_dword(rsp, static_cast<uint32_t>(value));
    } else if (push_size == 8) {
        rsp -= 8;
        regs.set64("rsp", rsp);
        mem.write_stack(rsp, value);
    } else {
        // Unsupported push size, log error
    }
}

void handle_ir_pop(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 1) {
        return; // Or log error
    }

    const auto& dest_op = ir_instr.operands[0];
    if (!std::holds_alternative<IRRegister>(dest_op)) {
        return; // Or log error, pop destination must be a register
    }

    const auto& dest_reg = std::get<IRRegister>(dest_op);

    auto& regs = simulator.getRegisterMap();
    auto& mem = simulator.getMemory();
    address_t rsp = regs.get64("rsp");

    if (dest_reg.size == 32) {
        uint32_t value = mem.read_stack_dword(rsp);
        setRegisterValue(dest_reg, value, simulator);
        regs.set64("rsp", rsp + 4);
    } else if (dest_reg.size == 64) {
        uint64_t value = mem.read_stack(rsp);
        setRegisterValue(dest_reg, value, simulator);
        regs.set64("rsp", rsp + 8);
    } else {
        // Unsupported pop size, log error
    }
}

void handle_ir_xor(const IRInstruction& ir_instr, ISimulator& simulator) {
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

void handle_ir_and(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 2) { return; }
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    if (!std::holds_alternative<IRRegister>(dest_op)) { return; }
    const auto& dest_reg = std::get<IRRegister>(dest_op);

    // AND instruction clears CF and OF.
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
            // TODO: Set Parity Flag
            break;
        }
        case 16: {
            uint16_t val1 = getOperandValue(dest_op, simulator);
            uint16_t val2 = getOperandValue(src_op, simulator);
            uint16_t result = val1 & val2;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000) != 0);
            // TODO: Set Parity Flag
            break;
        }
        case 32: {
            uint32_t val1 = getOperandValue(dest_op, simulator);
            uint32_t val2 = getOperandValue(src_op, simulator);
            uint32_t result = val1 & val2;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80000000) != 0);
            // TODO: Set Parity Flag
            break;
        }
        case 64: {
            uint64_t val1 = getOperandValue(dest_op, simulator);
            uint64_t val2 = getOperandValue(src_op, simulator);
            uint64_t result = val1 & val2;
            setRegisterValue(dest_reg, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000000000000000ULL) != 0);
            // TODO: Set Parity Flag
            break;
        }
        default: return;
    }
}

void handle_ir_or(const IRInstruction& ir_instr, ISimulator& simulator) {
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

void handle_ir_not(const IRInstruction& ir_instr, ISimulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& dest_reg = std::get<IRRegister>(dest_op);

    uint32_t destValue = getOperandValue(dest_op, simulator);
    uint32_t result = ~destValue;
    setRegisterValue(dest_reg, result, simulator);
}

void handle_ir_shl(const IRInstruction& ir_instr, ISimulator& simulator) {
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
    // simulator.set_PF((set_bits % 2) == 0);
}

void handle_ir_shr(const IRInstruction& ir_instr, ISimulator& simulator) {
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
    // simulator.set_PF((set_bits % 2) == 0);
}

void handle_ir_sar(const IRInstruction& ir_instr, ISimulator& simulator) {
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
    // simulator.set_PF((set_bits % 2) == 0);
}
