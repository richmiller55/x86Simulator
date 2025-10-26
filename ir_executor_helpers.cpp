#include "ir_executor_helpers.h"
#include <iostream>
#include "i_simulator.h"
#include "register_map.h"
#include "memory.h"
#include "i_database_manager.h"
#include "INTEL_helpers.h"
#include "x86_simulator.h"
#include <variant>
#include <limits>
#include "avx_core.h"
#include <cmath>

float getFloatOperandValue(const IROperand& op, ISimulator& simulator) {
    if (const std::string* reg_name = std::get_if<std::string>(&op)) {
        return simulator.getRegisterMap().get_float(*reg_name);
    } else if (const uint64_t* imm = std::get_if<uint64_t>(&op)) {
        // Immediate is integer, needs conversion. Or it could be a label.
        // This path might need more thought depending on how float immediates are represented.
        return static_cast<float>(*imm);
    }
    // Handle memory operands if necessary
    throw std::runtime_error("Unsupported operand type for getFloatOperandValue");
}

double getDoubleOperandValue(const IROperand& op, ISimulator& simulator) {
    if (const std::string* reg_name = std::get_if<std::string>(&op)) {
        return simulator.getRegisterMap().get_double(*reg_name);
    } else if (const uint64_t* imm = std::get_if<uint64_t>(&op)) {
        return static_cast<double>(*imm);
    }
    // Handle memory operands if necessary
    throw std::runtime_error("Unsupported operand type for getDoubleOperandValue");
}

void setFloatRegisterValue(const std::string& reg_name, float value, ISimulator& simulator) {
    simulator.getRegisterMap().set_float(reg_name, value);
}

void setDoubleRegisterValue(const std::string& reg_name, double value, ISimulator& simulator) {
    simulator.getRegisterMap().set_double(reg_name, value);
}




/**
 * @brief Gets the value of an IR operand by using the architecture map.
 */
int64_t getOperandValue(const IROperand& op, ISimulator& simulator) {
    if (std::holds_alternative<std::string>(op)) {
        const auto& reg_name = std::get<std::string>(op);
        const auto& arch = simulator.get_architecture();
        if (!arch.is_register(reg_name)) {
            // This could be a label, but for getOperandValue it must be a register.
            // Label addresses should already be resolved into uint64_t.
            throw std::runtime_error("Invalid register name or unresolved label in getOperandValue: " + reg_name);
        }
        auto& regs = simulator.getRegisterMap();
        uint32_t size = arch.get_register_size_bits(reg_name);

        switch (size) {
            case 8:   return regs.get8(reg_name);
            case 16:  return regs.get16(reg_name);
            case 32:  return regs.get32(reg_name);
            case 64:  return regs.get64(reg_name);
            default:
                if (size > 64) { // Vector registers
                    throw std::runtime_error("Vector register " + reg_name + " cannot be used as a scalar operand in getOperandValue");
                }
                throw std::runtime_error("Unsupported register size in getOperandValue: " + std::to_string(size));
        }
    } else if (std::holds_alternative<uint64_t>(op)) {
        return static_cast<int64_t>(std::get<uint64_t>(op));
    } else if (std::holds_alternative<IRMemoryOperand>(op)) {
        const auto& mem_op = std::get<IRMemoryOperand>(op);
        auto& regs = simulator.getRegisterMap();
        auto& mem = simulator.getMemory();
        const auto& arch = simulator.get_architecture();

        address_t addr = mem_op.displacement;
        if (mem_op.base_reg.has_value()) {
            const std::string& reg_name = mem_op.base_reg.value();
            if (arch.pointer_size_bits == 32) {
                addr += regs.get32(reg_name);
            } else {
                addr += regs.get64(reg_name);
            }
        }
        if (mem_op.index_reg.has_value()) {
            const std::string& reg_name = mem_op.index_reg.value();
            if (arch.pointer_size_bits == 32) {
                uint32_t index_val = regs.get32(reg_name);
                addr += index_val * mem_op.scale;
            } else {
                uint64_t index_val = regs.get64(reg_name);
                addr += index_val * mem_op.scale;
            }
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
 * @brief Sets the value of a register by name.
 */
void setRegisterValue(const std::string& reg_name, int64_t value, ISimulator& simulator) {
    const auto& arch = simulator.get_architecture();
    auto& regs = simulator.getRegisterMap();
    uint32_t size = arch.get_register_size_bits(reg_name);

    switch (size) {
        case 8:   regs.set8(reg_name, static_cast<uint8_t>(value)); break;
        case 16:  regs.set16(reg_name, static_cast<uint16_t>(value)); break;
        case 32:  regs.set32(reg_name, static_cast<uint32_t>(value)); break;
        case 64:  regs.set64(reg_name, static_cast<uint64_t>(value)); break;
        default:
            throw std::runtime_error("Unsupported register size in setRegisterValue: " + std::to_string(size));
    }
}

void setMemoryValue(const IRMemoryOperand& mem_op, uint64_t value, ISimulator& simulator) {
    setMemoryValue(mem_op, static_cast<int64_t>(value), simulator);
}

void setMemoryValue(const IRMemoryOperand& mem_op, int64_t value, ISimulator& simulator) {
    auto& regs = simulator.getRegisterMap();
    auto& mem = simulator.getMemory();
    const auto& arch = simulator.get_architecture();

    address_t addr = mem_op.displacement;
    if (mem_op.base_reg.has_value()) {
        const std::string& reg_name = mem_op.base_reg.value();
        if (arch.pointer_size_bits == 32) {
            addr += regs.get32(reg_name);
        } else {
            addr += regs.get64(reg_name);
        }
    }
    if (mem_op.index_reg.has_value()) {
        const std::string& reg_name = mem_op.index_reg.value();
        if (arch.pointer_size_bits == 32) {
            uint32_t index_val = regs.get32(reg_name);
            addr += index_val * mem_op.scale;
        } else {
            uint64_t index_val = regs.get64(reg_name);
            addr += index_val * mem_op.scale;
        }
    }

    switch (mem_op.size) {
        case 8:   mem.write_byte(addr, static_cast<uint8_t>(value)); break;
        case 16:  mem.write_word(addr, static_cast<uint16_t>(value)); break;
        case 32:  mem.write_dword(addr, static_cast<uint32_t>(value)); break;
        case 64:  mem.write_qword(addr, static_cast<uint64_t>(value)); break;
        default:
            throw std::runtime_error("Unsupported memory access size in setMemoryValue: " + std::to_string(mem_op.size));
    }
}

/**
 * @brief Executes an IR 'Add' instruction.
 */
void handle_ir_add(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() < 2 || ir_instr.operands.size() > 3) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Invalid number of operands for IR Add", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_op = ir_instr.operands[0];
    if (!std::holds_alternative<std::string>(dest_op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Add requires a register destination.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }
    const auto& dest_name = std::get<std::string>(dest_op);
    const auto& arch = simulator.get_architecture();
    uint32_t size = arch.get_register_size_bits(dest_name);

    int64_t val1, val2;
    if (ir_instr.operands.size() == 2) { // 2-operand form: dest += src
        val1 = getOperandValue(dest_op, simulator);
        val2 = getOperandValue(ir_instr.operands[1], simulator);
    } else { // 3-operand form: dest = src1 + src2
        val1 = getOperandValue(ir_instr.operands[1], simulator);
        val2 = getOperandValue(ir_instr.operands[2], simulator);
    }

    switch (size) {
        case 8: {
            uint8_t result = static_cast<uint8_t>(val1) + static_cast<uint8_t>(val2);
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80) != 0);
            simulator.set_CF(static_cast<uint16_t>(val1) + static_cast<uint16_t>(val2) > 0xFF);
            // TODO: Set OF, AF, PF for 8-bit
            break;
        }
        case 16: {
            uint16_t result = static_cast<uint16_t>(val1) + static_cast<uint16_t>(val2);
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000) != 0);
            simulator.set_CF(static_cast<uint32_t>(val1) + static_cast<uint32_t>(val2) > 0xFFFF);
            // TODO: Set OF, AF, PF for 16-bit
            break;
        }
        case 32: {
            uint32_t u_val1 = static_cast<uint32_t>(val1);
            uint32_t u_val2 = static_cast<uint32_t>(val2);
            uint64_t result64 = static_cast<uint64_t>(u_val1) + static_cast<uint64_t>(u_val2);
            uint32_t result = static_cast<uint32_t>(result64);
            setRegisterValue(dest_name, result, simulator);

            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80000000) != 0);
            simulator.set_CF(result64 > 0xFFFFFFFF);

            // Set Overflow Flag (OF)
            bool val1_sign = (u_val1 & 0x80000000) != 0;
            bool val2_sign = (u_val2 & 0x80000000) != 0;
            bool result_sign = (result & 0x80000000) != 0;
            simulator.set_OF((val1_sign == val2_sign) && (val1_sign != result_sign));
            
            // TODO: Set AF, PF for 32-bit
            break;
        }
        case 64: {
            uint64_t result = static_cast<uint64_t>(val1) + static_cast<uint64_t>(val2);
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000000000000000ULL) != 0);
            simulator.set_CF(static_cast<uint64_t>(val1) > result); // If result wrapped around, it will be less than the original.
            // TODO: Set OF, AF, PF for 64-bit
            break;
        }
        default:
            simulator.getDatabaseManager().log(simulator.get_session_id(), "Unsupported register size for IR Add", "ERROR", 0, __FILE__, __LINE__);
            break;
    }
}

void handle_ir_sub(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() < 2 || ir_instr.operands.size() > 3) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Invalid number of operands for IR Sub", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_op = ir_instr.operands[0];
    if (!std::holds_alternative<std::string>(dest_op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Sub requires a register destination.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }
    const auto& dest_name = std::get<std::string>(dest_op);
    const auto& arch = simulator.get_architecture();
    uint32_t size = arch.get_register_size_bits(dest_name);

    int64_t val1, val2;
    if (ir_instr.operands.size() == 2) { // 2-operand form: dest -= src
        val1 = getOperandValue(dest_op, simulator);
        val2 = getOperandValue(ir_instr.operands[1], simulator);
    } else { // 3-operand form: dest = src1 - src2
        val1 = getOperandValue(ir_instr.operands[1], simulator);
        val2 = getOperandValue(ir_instr.operands[2], simulator);
    }

    switch (size) {
        case 8: {
            uint8_t result = static_cast<uint8_t>(val1) - static_cast<uint8_t>(val2);
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80) != 0);
            simulator.set_CF(static_cast<uint8_t>(val1) < static_cast<uint8_t>(val2));
            // TODO: Set OF, AF, PF for 8-bit
            break;
        }
        case 16: {
            uint16_t result = static_cast<uint16_t>(val1) - static_cast<uint16_t>(val2);
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000) != 0);
            simulator.set_CF(static_cast<uint16_t>(val1) < static_cast<uint16_t>(val2));
            // TODO: Set OF, AF, PF for 16-bit
            break;
        }
        case 32: {
            int64_t result64 = val1 - val2;
            uint32_t result = static_cast<uint32_t>(result64);
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80000000) != 0);
            simulator.set_CF(static_cast<uint64_t>(val1) < static_cast<uint64_t>(val2));
            // TODO: Set OF, AF, PF for 32-bit
            break;
        }
        case 64: {
            uint64_t result = static_cast<uint64_t>(val1) - static_cast<uint64_t>(val2);
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000000000000000ULL) != 0);
            simulator.set_CF(static_cast<uint64_t>(val1) < static_cast<uint64_t>(val2));
            // TODO: Set OF, AF, PF for 64-bit
            break;
        }
        default:
            simulator.getDatabaseManager().log(simulator.get_session_id(), "Unsupported register size for IR Sub", "ERROR", 0, __FILE__, __LINE__);
            break;
    }
}

void handle_ir_addc(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() < 2 || ir_instr.operands.size() > 3) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Invalid number of operands for IR AddC", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_op = ir_instr.operands[0];
    if (!std::holds_alternative<std::string>(dest_op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR AddC requires a register destination.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }
    const auto& dest_name = std::get<std::string>(dest_op);
    const auto& arch = simulator.get_architecture();
    uint32_t size = arch.get_register_size_bits(dest_name);

    int64_t val1, val2;
    uint8_t carry = simulator.get_CF();

    if (ir_instr.operands.size() == 2) { // 2-operand form: dest += src + CF
        val1 = getOperandValue(dest_op, simulator);
        val2 = getOperandValue(ir_instr.operands[1], simulator);
    } else { // 3-operand form: dest = src1 + src2 + CF
        val1 = getOperandValue(ir_instr.operands[1], simulator);
        val2 = getOperandValue(ir_instr.operands[2], simulator);
    }

    switch (size) {
        case 32: { // Assuming 32-bit for ARM
            uint64_t result64 = static_cast<uint64_t>(val1) + static_cast<uint64_t>(val2) + carry;
            uint32_t result32 = static_cast<uint32_t>(result64);
            setRegisterValue(dest_name, result32, simulator);
            simulator.set_ZF(result32 == 0);
            simulator.set_SF((result32 & 0x80000000) != 0);
            simulator.set_CF(result64 > 0xFFFFFFFF);
            // TODO: Set OF
            break;
        }
        default:
            // Simplified, only handling 32-bit for now
            break;
    }
}

void handle_ir_subc(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() < 2 || ir_instr.operands.size() > 3) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Invalid number of operands for IR SubC", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_op = ir_instr.operands[0];
    if (!std::holds_alternative<std::string>(dest_op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR SubC requires a register destination.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }
    const auto& dest_name = std::get<std::string>(dest_op);
    const auto& arch = simulator.get_architecture();
    uint32_t size = arch.get_register_size_bits(dest_name);

    int64_t val1, val2;
    uint8_t borrow = 1 - simulator.get_CF();

    if (ir_instr.operands.size() == 2) { // 2-operand form: dest -= src - !CF
        val1 = getOperandValue(dest_op, simulator);
        val2 = getOperandValue(ir_instr.operands[1], simulator);
    } else { // 3-operand form: dest = src1 - src2 - !CF
        val1 = getOperandValue(ir_instr.operands[1], simulator);
        val2 = getOperandValue(ir_instr.operands[2], simulator);
    }

    switch (size) {
        case 32: { // Assuming 32-bit for ARM
            uint64_t result64 = static_cast<uint64_t>(val1) - static_cast<uint64_t>(val2) - borrow;
            uint32_t result32 = static_cast<uint32_t>(result64);
            setRegisterValue(dest_name, result32, simulator);
            simulator.set_ZF(result32 == 0);
            simulator.set_SF((result32 & 0x80000000) != 0);
            simulator.set_CF(static_cast<uint64_t>(val1) < (static_cast<uint64_t>(val2) + borrow));
            // TODO: Set OF
            break;
        }
        default:
            // Simplified, only handling 32-bit for now
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

    if (!std::holds_alternative<std::string>(dest_op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Move requires a register destination.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_name = std::get<std::string>(dest_op);
    int64_t sourceValue = getOperandValue(src_op, simulator);

    setRegisterValue(dest_name, sourceValue, simulator);
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

    if (!std::holds_alternative<std::string>(dest_op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Load requires a register destination.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }
    if (!std::holds_alternative<IRMemoryOperand>(src_op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Load requires a memory source.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_name = std::get<std::string>(dest_op);
    int64_t sourceValue = getOperandValue(src_op, simulator);

    setRegisterValue(dest_name, sourceValue, simulator);
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
    int64_t sourceValue = getOperandValue(src_op, simulator);

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

    if (std::holds_alternative<uint64_t>(target_op)) {
        target_address = std::get<uint64_t>(target_op);
    } else if (std::holds_alternative<std::string>(target_op)) {
        target_address = getOperandValue(target_op, simulator);
    } else {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Jump target is not a valid address or register.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const char* ip_name = simulator.get_instruction_pointer_name();
    setRegisterValue(ip_name, target_address, simulator);
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
        const char* ip_name = simulator.get_instruction_pointer_name();
        setRegisterValue(ip_name, target_address, simulator);
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
    if (const std::string* reg_name = std::get_if<std::string>(&op1)) {
        size = simulator.get_architecture().get_register_size_bits(*reg_name);
    } else if (const IRMemoryOperand* mem = std::get_if<IRMemoryOperand>(&op1)) {
        size = mem->size;
    } else { // Should not happen if IR is well-formed
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Invalid first operand for IR Cmp", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    switch (size) {
        case 8: {
            int8_t val1 = getOperandValue(op1, simulator);
            int8_t val2 = getOperandValue(op2, simulator);
            int8_t result = val1 - val2;
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80) != 0);
            simulator.set_CF(static_cast<uint8_t>(val1) < static_cast<uint8_t>(val2));
            bool val1_sign = (val1 & 0x80) != 0;
            bool val2_sign = (val2 & 0x80) != 0;
            bool result_sign = (result & 0x80) != 0;
            simulator.set_OF((val1_sign != val2_sign) && (result_sign != val1_sign));
            break;
        }
        case 16: {
            int16_t val1 = getOperandValue(op1, simulator);
            int16_t val2 = getOperandValue(op2, simulator);
            int16_t result = val1 - val2;
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000) != 0);
            simulator.set_CF(static_cast<uint16_t>(val1) < static_cast<uint16_t>(val2));
            bool val1_sign = (val1 & 0x8000) != 0;
            bool val2_sign = (val2 & 0x8000) != 0;
            bool result_sign = (result & 0x8000) != 0;
            simulator.set_OF((val1_sign != val2_sign) && (result_sign != val1_sign));
            break;
        }
        case 32: {
            int32_t val1 = getOperandValue(op1, simulator);
            int32_t val2 = getOperandValue(op2, simulator);
            int32_t result = val1 - val2;
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80000000) != 0);
            simulator.set_CF(static_cast<uint32_t>(val1) < static_cast<uint32_t>(val2));
            bool val1_sign = (val1 & 0x80000000) != 0;
            bool val2_sign = (val2 & 0x80000000) != 0;
            bool result_sign = (result & 0x80000000) != 0;
            simulator.set_OF((val1_sign != val2_sign) && (result_sign != val1_sign));
            break;
        }
        case 64: {
            int64_t val1 = getOperandValue(op1, simulator);
            int64_t val2 = getOperandValue(op2, simulator);
            int64_t result = val1 - val2;
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000000000000000ULL) != 0);
            simulator.set_CF(static_cast<uint64_t>(val1) < static_cast<uint64_t>(val2));
            bool val1_sign = (val1 & 0x8000000000000000ULL) != 0;
            bool val2_sign = (val2 & 0x8000000000000000ULL) != 0;
            bool result_sign = (result & 0x8000000000000000ULL) != 0;
            simulator.set_OF((val1_sign != val2_sign) && (result_sign != val1_sign));
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

    if (!std::holds_alternative<std::string>(op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Inc requires a register destination.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_name = std::get<std::string>(op);
    const auto& arch = simulator.get_architecture();
    uint32_t size = arch.get_register_size_bits(dest_name);

    switch (size) {
        case 8: {
            int8_t value = getOperandValue(op, simulator);
            uint8_t result = value + 1;
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80) != 0);
            simulator.set_OF(value == 0x7F);
            break;
        }
        case 16: {
            int16_t value = getOperandValue(op, simulator);
            uint16_t result = value + 1;
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000) != 0);
            simulator.set_OF(value == 0x7FFF);
            break;
        }
        case 32: {
            int32_t value = getOperandValue(op, simulator);
            uint32_t result = value + 1;
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80000000) != 0);
            simulator.set_OF(value == 0x7FFFFFFF);
            break;
        }
        case 64: {
            int64_t value = getOperandValue(op, simulator);
            uint64_t result = value + 1;
            setRegisterValue(dest_name, result, simulator);
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
    if (ir_instr.operands.empty()) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Syscall requires at least one operand.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& vector_op = ir_instr.operands[0];
    if (!std::holds_alternative<uint64_t>(vector_op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Syscall operand must be an immediate value.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    uint8_t interrupt_vector = std::get<uint64_t>(vector_op);
    auto& regs = simulator.getRegisterMap();
    const auto& arch = simulator.get_architecture();

    if (arch.isa == ISA::X86 && interrupt_vector == 0x80) { // Linux x86 syscall convention
        uint32_t syscall_num = regs.get32("eax");

        switch (syscall_num) {
            case 1: { // sys_exit
                uint32_t exit_code = regs.get32("ebx");
                std::string logMessage = "Program exited via sys_exit with code: " + std::to_string(exit_code);
                simulator.getDatabaseManager().log(simulator.get_session_id(), logMessage, "INFO", 0, __FILE__, __LINE__);
                
                const char* ip_name = simulator.get_instruction_pointer_name();
                setRegisterValue(ip_name, simulator.getMemory().get_total_memory_size(), simulator);
                break;
            }
            default: {
                simulator.getDatabaseManager().log(simulator.get_session_id(), "Unsupported x86 syscall: " + std::to_string(syscall_num), "WARNING", 0, __FILE__, __LINE__);
                break;
            }
        }
    } else if (arch.isa == ISA::ARM) { // Linux ARM EABI syscall convention
        // The immediate value in SWI is ignored by the kernel.
        uint32_t syscall_num = regs.get32("r7");

        switch (syscall_num) {
            case 1: { // __NR_exit
                uint32_t exit_code = regs.get32("r0");
                std::string logMessage = "Program exited via __NR_exit with code: " + std::to_string(exit_code);
                simulator.getDatabaseManager().log(simulator.get_session_id(), logMessage, "INFO", 0, __FILE__, __LINE__);

                const char* ip_name = simulator.get_instruction_pointer_name();
                setRegisterValue(ip_name, simulator.getMemory().get_total_memory_size(), simulator);
                break;
            }
            default: {
                simulator.getDatabaseManager().log(simulator.get_session_id(), "Unsupported ARM syscall: " + std::to_string(syscall_num), "WARNING", 0, __FILE__, __LINE__);
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

    if (!std::holds_alternative<std::string>(op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(),
					   "IR Dec requires a register destination.", "ERROR",
					   0, __FILE__, __LINE__);
        return;
    }

    const auto& dest_name = std::get<std::string>(op);
    const auto& arch = simulator.get_architecture();
    uint32_t size = arch.get_register_size_bits(dest_name);

    switch (size) {
        case 8: {
            int8_t value = getOperandValue(op, simulator);
            uint8_t result = value - 1;
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80) != 0);
            simulator.set_OF(value == 0x80);
            break;
        }
        case 16: {
            int16_t value = getOperandValue(op, simulator);
            uint16_t result = value - 1;
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000) != 0);
            simulator.set_OF(value == 0x8000);
            break;
        }
        case 32: {
            int32_t value = getOperandValue(op, simulator);
            uint32_t result = value - 1;
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80000000) != 0);
            simulator.set_OF(static_cast<uint32_t>(value) == 0x80000000);
            break;
        }
        case 64: {
            int64_t value = getOperandValue(op, simulator);
            uint64_t result = value - 1;
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000000000000000ULL) != 0);
            simulator.set_OF(static_cast<uint64_t>(value) == 0x8000000000000000ULL);
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
    address_t target_address = 0;

    if (std::holds_alternative<uint64_t>(target_op)) {
        target_address = std::get<uint64_t>(target_op);
    } else if (std::holds_alternative<std::string>(target_op)) {
        target_address = getOperandValue(target_op, simulator);
    } else {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Call target is not a valid address or register.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    // 2. Calculate the return address
    address_t return_address = ir_instr.original_address + ir_instr.original_size;

    // 3. Push the return address onto the stack
    auto& regs = simulator.getRegisterMap();
    auto& mem = simulator.getMemory();
    const char* sp_name = simulator.get_stack_pointer_name();
    
    address_t sp = regs.get64(sp_name);
    // ARM 'BL' stores return address in LR, and stack is not implicitly touched.
    // x86 'CALL' pushes return address on the stack.
    // This generic handler will follow the x86 model for now.
    // A more robust solution would be to have architecture-specific handlers
    // or a more detailed IR.
    sp -= 8; 
    regs.set64(sp_name, sp);

    // Write return address to the stack
    mem.write_stack(sp, return_address);

    // 4. Set IP to the target address
    const char* ip_name = simulator.get_instruction_pointer_name();
    setRegisterValue(ip_name, target_address, simulator);
}

void handle_ir_push(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 1) {
        return; // Or log error
    }

    const auto& src_op = ir_instr.operands[0];
    int64_t value = getOperandValue(src_op, simulator);

    auto& regs = simulator.getRegisterMap();
    auto& mem = simulator.getMemory();
    const char* sp_name = simulator.get_stack_pointer_name();
    address_t sp = regs.get64(sp_name);

    uint32_t push_size = 0;
    if (const std::string* reg_name = std::get_if<std::string>(&src_op)) {
        push_size = simulator.get_architecture().get_register_size_bits(*reg_name) / 8;
    } else if (std::holds_alternative<uint64_t>(src_op)) {
        push_size = 4; // Assume 32-bit for immediates
    }

    if (push_size == 4) {
        sp -= 4;
        regs.set64(sp_name, sp);
        mem.write_stack_dword(sp, static_cast<uint32_t>(value));
    } else if (push_size == 8) {
        sp -= 8;
        regs.set64(sp_name, sp);
        mem.write_stack(sp, value);
    } else {
        // Unsupported push size, log error
    }
}

void handle_ir_pop(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 1) {
        return; // Or log error
    }

    const auto& dest_op = ir_instr.operands[0];
    if (!std::holds_alternative<std::string>(dest_op)) {
        return; // Or log error, pop destination must be a register
    }

    const auto& dest_name = std::get<std::string>(dest_op);
    const auto& arch = simulator.get_architecture();
    uint32_t size = arch.get_register_size_bits(dest_name);

    auto& regs = simulator.getRegisterMap();
    auto& mem = simulator.getMemory();
    const char* sp_name = simulator.get_stack_pointer_name();
    address_t sp = regs.get64(sp_name);

    if (size == 32) {
        uint32_t value = mem.read_stack_dword(sp);
        setRegisterValue(dest_name, static_cast<int64_t>(value), simulator);
        regs.set64(sp_name, sp + 4);
    } else if (size == 64) {
        uint64_t value = mem.read_stack(sp);
        setRegisterValue(dest_name, static_cast<int64_t>(value), simulator);
        regs.set64(sp_name, sp + 8);
    } else {
        // Unsupported pop size, log error
    }
}

void handle_ir_and(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() < 2 || ir_instr.operands.size() > 3) { return; }
    const auto& dest_op = ir_instr.operands[0];
    if (!std::holds_alternative<std::string>(dest_op)) { return; }
    const auto& dest_name = std::get<std::string>(dest_op);
    const auto& arch = simulator.get_architecture();
    uint32_t size = arch.get_register_size_bits(dest_name);

    int64_t val1, val2;
    if (ir_instr.operands.size() == 2) { // 2-operand form: dest &= src
        val1 = getOperandValue(dest_op, simulator);
        val2 = getOperandValue(ir_instr.operands[1], simulator);
    } else { // 3-operand form: dest = src1 & src2
        val1 = getOperandValue(ir_instr.operands[1], simulator);
        val2 = getOperandValue(ir_instr.operands[2], simulator);
    }

    // AND instruction clears CF and OF.
    simulator.set_CF(false);
    simulator.set_OF(false);

    switch (size) {
        case 8: {
            uint8_t result = static_cast<uint8_t>(val1) & static_cast<uint8_t>(val2);
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80) != 0);
            break;
        }
        case 16: {
            uint16_t result = static_cast<uint16_t>(val1) & static_cast<uint16_t>(val2);
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000) != 0);
            break;
        }
        case 32: {
            uint32_t result = static_cast<uint32_t>(val1) & static_cast<uint32_t>(val2);
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80000000) != 0);
            break;
        }
        case 64: {
            uint64_t result = val1 & val2;
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000000000000000ULL) != 0);
            break;
        }
        default: return;
    }
}

void handle_ir_or(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() < 2 || ir_instr.operands.size() > 3) { return; }
    const auto& dest_op = ir_instr.operands[0];
    if (!std::holds_alternative<std::string>(dest_op)) { return; }
    const auto& dest_name = std::get<std::string>(dest_op);
    const auto& arch = simulator.get_architecture();
    uint32_t size = arch.get_register_size_bits(dest_name);

    int64_t val1, val2;
    if (ir_instr.operands.size() == 2) { // 2-operand form: dest |= src
        val1 = getOperandValue(dest_op, simulator);
        val2 = getOperandValue(ir_instr.operands[1], simulator);
    } else { // 3-operand form: dest = src1 | src2
        val1 = getOperandValue(ir_instr.operands[1], simulator);
        val2 = getOperandValue(ir_instr.operands[2], simulator);
    }

    // OR instruction clears CF and OF.
    simulator.set_CF(false);
    simulator.set_OF(false);

    switch (size) {
        case 8: {
            uint8_t result = static_cast<uint8_t>(val1) | static_cast<uint8_t>(val2);
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80) != 0);
            break;
        }
        case 16: {
            uint16_t result = static_cast<uint16_t>(val1) | static_cast<uint16_t>(val2);
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000) != 0);
            break;
        }
        case 32: {
            uint32_t result = static_cast<uint32_t>(val1) | static_cast<uint32_t>(val2);
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80000000) != 0);
            break;
        }
        case 64: {
            uint64_t result = val1 | val2;
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000000000000000ULL) != 0);
            break;
        }
        default: return;
    }
}

void handle_ir_xor(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() < 2 || ir_instr.operands.size() > 3) { return; }
    const auto& dest_op = ir_instr.operands[0];
    if (!std::holds_alternative<std::string>(dest_op)) { return; }
    const auto& dest_name = std::get<std::string>(dest_op);
    const auto& arch = simulator.get_architecture();
    uint32_t size = arch.get_register_size_bits(dest_name);

    int64_t val1, val2;
    if (ir_instr.operands.size() == 2) { // 2-operand form: dest ^= src
        val1 = getOperandValue(dest_op, simulator);
        val2 = getOperandValue(ir_instr.operands[1], simulator);
    } else { // 3-operand form: dest = src1 ^ src2
        val1 = getOperandValue(ir_instr.operands[1], simulator);
        val2 = getOperandValue(ir_instr.operands[2], simulator);
    }

    // XOR instruction clears CF and OF.
    simulator.set_CF(false);
    simulator.set_OF(false);

    switch (size) {
        case 8: {
            uint8_t result = static_cast<uint8_t>(val1) ^ static_cast<uint8_t>(val2);
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80) != 0);
            break;
        }
        case 16: {
            uint16_t result = static_cast<uint16_t>(val1) ^ static_cast<uint16_t>(val2);
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000) != 0);
            break;
        }
        case 32: {
            uint32_t result = static_cast<uint32_t>(val1) ^ static_cast<uint32_t>(val2);
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x80000000) != 0);
            break;
        }
        case 64: {
            uint64_t result = val1 ^ val2;
            setRegisterValue(dest_name, result, simulator);
            simulator.set_ZF(result == 0);
            simulator.set_SF((result & 0x8000000000000000ULL) != 0);
            break;
        }
        default: return;
    }
}

void handle_ir_not(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 1) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Invalid number of operands for IR Not", "ERROR", 0, __FILE__, __LINE__);
        return;
    }
    const auto& dest_op = ir_instr.operands[0];
    if (!std::holds_alternative<std::string>(dest_op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Not requires a register destination.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }
    const auto& dest_name = std::get<std::string>(dest_op);

    int64_t destValue = getOperandValue(dest_op, simulator);
    uint32_t result = ~static_cast<uint32_t>(destValue);
    setRegisterValue(dest_name, result, simulator);
}

void handle_ir_shl(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 2) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Invalid number of operands for IR Shl", "ERROR", 0, __FILE__, __LINE__);
        return;
    }
    const auto& dest_op = ir_instr.operands[0];
    const auto& count_op = ir_instr.operands[1];
    if (!std::holds_alternative<std::string>(dest_op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Shl requires a register destination.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }
    const auto& dest_name = std::get<std::string>(dest_op);

    int64_t destValue = getOperandValue(dest_op, simulator);
    uint8_t count = getOperandValue(count_op, simulator);
    uint32_t result = static_cast<uint32_t>(destValue) << count;
    setRegisterValue(dest_name, result, simulator);

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
    if (ir_instr.operands.size() != 2) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Invalid number of operands for IR Shr", "ERROR", 0, __FILE__, __LINE__);
        return;
    }
    const auto& dest_op = ir_instr.operands[0];
    const auto& count_op = ir_instr.operands[1];
    if (!std::holds_alternative<std::string>(dest_op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Shr requires a register destination.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }
    const auto& dest_name = std::get<std::string>(dest_op);

    int64_t destValue = getOperandValue(dest_op, simulator);
    uint8_t count = getOperandValue(count_op, simulator);
    uint32_t result = static_cast<uint32_t>(destValue) >> count;
    setRegisterValue(dest_name, result, simulator);

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
    if (ir_instr.operands.size() != 2) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Invalid number of operands for IR Sar", "ERROR", 0, __FILE__, __LINE__);
        return;
    }
    const auto& dest_op = ir_instr.operands[0];
    const auto& count_op = ir_instr.operands[1];
    if (!std::holds_alternative<std::string>(dest_op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "IR Sar requires a register destination.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }
    const auto& dest_name = std::get<std::string>(dest_op);

    int64_t destValue = getOperandValue(dest_op, simulator);
    uint8_t count = getOperandValue(count_op, simulator);
    int32_t result = static_cast<int32_t>(destValue) >> count;
    setRegisterValue(dest_name, result, simulator);

    simulator.set_ZF(result == 0);
    simulator.set_SF(result < 0);
    if (count > 0) {
        simulator.set_CF((static_cast<int32_t>(destValue) >> (count - 1)) & 1);
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

void handle_ir_in(const IRInstruction& ir_instr, ISimulator& simulator) {
    // For now, we'll just read a character from stdin.
    // A more complete implementation would handle ports.
    char input_char;
    std::cin >> input_char;
    const auto& dest_op = ir_instr.operands[0];
    if (const std::string* dest_name = std::get_if<std::string>(&dest_op)) {
        setRegisterValue(*dest_name, input_char, simulator);
    }
}

void handle_ir_out(const IRInstruction& ir_instr, ISimulator& simulator) {
    // For now, we'll just write a character to stdout.
    // A more complete implementation would handle ports.
    const auto& src_op = ir_instr.operands[1];
    int64_t value = getOperandValue(src_op, simulator);
    std::cout << static_cast<char>(value);
}

void handle_ir_ret(const IRInstruction& ir_instr, ISimulator& simulator) {
    auto& regs = simulator.getRegisterMap();
    auto& mem = simulator.getMemory();
    const char* sp_name = simulator.get_stack_pointer_name();
    address_t sp = regs.get64(sp_name);
    const auto& arch = simulator.get_architecture();

    uint64_t return_address = 0;
    uint32_t pop_size = arch.pointer_size_bits / 8;

    if (pop_size == 4) {
        return_address = mem.read_stack_dword(sp);
    } else { // Assume 8 for 64-bit
        return_address = mem.read_stack(sp);
    }

    const char* ip_name = simulator.get_instruction_pointer_name();
    setRegisterValue(ip_name, return_address, simulator);
    regs.set64(sp_name, sp + pop_size);
}

void handle_ir_tst(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 2) { return; }
    const auto& op1 = ir_instr.operands[0];
    const auto& op2 = ir_instr.operands[1];
    int64_t val1 = getOperandValue(op1, simulator);
    int64_t val2 = getOperandValue(op2, simulator);
    uint32_t result = static_cast<uint32_t>(val1) & static_cast<uint32_t>(val2);
    simulator.set_ZF(result == 0);
    simulator.set_SF((result & 0x80000000) != 0);
}

void handle_ir_teq(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 2) { return; }
    const auto& op1 = ir_instr.operands[0];
    const auto& op2 = ir_instr.operands[1];
    int64_t val1 = getOperandValue(op1, simulator);
    int64_t val2 = getOperandValue(op2, simulator);
    uint32_t result = static_cast<uint32_t>(val1) ^ static_cast<uint32_t>(val2);
    simulator.set_ZF(result == 0);
    simulator.set_SF((result & 0x80000000) != 0);
}

void handle_ir_cmn(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 2) { return; }
    const auto& op1 = ir_instr.operands[0];
    const auto& op2 = ir_instr.operands[1];
    int64_t val1 = getOperandValue(op1, simulator);
    int64_t val2 = getOperandValue(op2, simulator);
    uint64_t result64 = static_cast<uint64_t>(val1) + static_cast<uint64_t>(val2);
    uint32_t result32 = static_cast<uint32_t>(result64);
    simulator.set_ZF(result32 == 0);
    simulator.set_SF((result32 & 0x80000000) != 0);
    simulator.set_CF(result64 > 0xFFFFFFFF);
    // TODO: Set OF
}

void handle_ir_movenot(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 2) { return; }
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    if (!std::holds_alternative<std::string>(dest_op)) { return; }
    const auto& dest_name = std::get<std::string>(dest_op);
    int64_t src_val = getOperandValue(src_op, simulator);
    uint64_t result = ~static_cast<uint64_t>(src_val);
    setRegisterValue(dest_name, result, simulator);
}

void handle_ir_andnot(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 3) { return; }
    const auto& dest_op = ir_instr.operands[0];
    const auto& src1_op = ir_instr.operands[1];
    const auto& src2_op = ir_instr.operands[2];
    if (!std::holds_alternative<std::string>(dest_op)) { return; }
    const auto& dest_name = std::get<std::string>(dest_op);
    int64_t src1_val = getOperandValue(src1_op, simulator);
    int64_t src2_val = getOperandValue(src2_op, simulator);
    uint64_t result = static_cast<uint64_t>(src1_val) & ~static_cast<uint64_t>(src2_val);
    setRegisterValue(dest_name, result, simulator);
}

void handle_ir_div(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 3) return;

    const auto& dest_op = ir_instr.operands[0];
    const auto& src1_op = ir_instr.operands[1];
    const auto& src2_op = ir_instr.operands[2];

    if (!std::holds_alternative<std::string>(dest_op)) return;

    int32_t src1_val = getOperandValue(src1_op, simulator);
    int32_t src2_val = getOperandValue(src2_op, simulator);

    if (src2_val == 0) {
        // Handle division by zero. For now, just log it.
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Division by zero.", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    int32_t result = src1_val / src2_val;

    const auto& dest_name = std::get<std::string>(dest_op);
    setRegisterValue(dest_name, result, simulator);
}

void handle_ir_nop(const IRInstruction& ir_instr, ISimulator& simulator) {
    // NOP does nothing.
}

void handle_ir_swap(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 3) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Invalid number of operands for IR Swap", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& rd_op = ir_instr.operands[0]; // Destination register
    const auto& rm_op = ir_instr.operands[1]; // Source register
    const auto& rn_mem_op = ir_instr.operands[2]; // Memory operand

    if (!std::holds_alternative<std::string>(rd_op) || !std::holds_alternative<std::string>(rm_op) || !std::holds_alternative<IRMemoryOperand>(rn_mem_op)) {
        simulator.getDatabaseManager().log(simulator.get_session_id(), "Invalid operand types for IR Swap", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    const auto& rd_name = std::get<std::string>(rd_op);
    const auto& rn_mem = std::get<IRMemoryOperand>(rn_mem_op);

    // Note: This implementation is not atomic.
    // In a multi-threaded environment, this would need a lock.
    int64_t value_from_mem = getOperandValue(rn_mem, simulator);
    int64_t value_from_rm = getOperandValue(rm_op, simulator);

    setMemoryValue(rn_mem, value_from_rm, simulator);
    setRegisterValue(rd_name, value_from_mem, simulator);
}

void handle_ir_move_to_system_register(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 2) { return; }

    const auto& dest_op = ir_instr.operands[0]; // System register name (string)
    const auto& src_op = ir_instr.operands[1];  // Source GPR

    if (!std::holds_alternative<std::string>(dest_op)) { return; }

    const std::string& sys_reg_name = std::get<std::string>(dest_op);
    uint64_t value = getOperandValue(src_op, simulator);

    simulator.set_system_register(sys_reg_name, value);
}

void handle_ir_move_from_system_register(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 2) { return; }

    const auto& dest_op = ir_instr.operands[0]; // Destination GPR
    const auto& src_op = ir_instr.operands[1];  // System register name (string)

    if (!std::holds_alternative<std::string>(dest_op) || !std::holds_alternative<std::string>(src_op)) { return; }

    const auto& dest_name = std::get<std::string>(dest_op);
    const std::string& sys_reg_name = std::get<std::string>(src_op);

    uint64_t value = simulator.get_system_register(sys_reg_name);
    setRegisterValue(dest_name, value, simulator);
}

void handle_ir_count_leading_zeros(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 2) { return; }

    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];

    if (!std::holds_alternative<std::string>(dest_op) || !std::holds_alternative<std::string>(src_op)) { return; }

    const auto& dest_name = std::get<std::string>(dest_op);
    uint32_t src_val = getOperandValue(src_op, simulator);

    if (src_val == 0) {
        setRegisterValue(dest_name, 32, simulator);
    } else {
        setRegisterValue(dest_name, __builtin_clz(src_val), simulator);
    }
}

void handle_ir_reverse_bits(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 2) { return; }
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    if (!std::holds_alternative<std::string>(dest_op) || !std::holds_alternative<std::string>(src_op)) { return; }

    const auto& dest_name = std::get<std::string>(dest_op);
    uint32_t src_val = getOperandValue(src_op, simulator);
    uint32_t result = 0;
    for (int i = 0; i < 32; ++i) {
        if ((src_val >> i) & 1) {
            result |= 1 << (31 - i);
        }
    }
    setRegisterValue(dest_name, result, simulator);
}

void handle_ir_reverse_bytes(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 2) { return; }
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    if (!std::holds_alternative<std::string>(dest_op) || !std::holds_alternative<std::string>(src_op)) { return; }

    const auto& dest_name = std::get<std::string>(dest_op);
    uint32_t src_val = getOperandValue(src_op, simulator);
    uint32_t result = __builtin_bswap32(src_val);
    setRegisterValue(dest_name, result, simulator);
}

void handle_ir_reverse_bytes16(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 2) { return; }
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    if (!std::holds_alternative<std::string>(dest_op) || !std::holds_alternative<std::string>(src_op)) { return; }

    const auto& dest_name = std::get<std::string>(dest_op);
    uint32_t src_val = getOperandValue(src_op, simulator);
    uint32_t result = ((src_val & 0xFF00FF00) >> 8) | ((src_val & 0x00FF00FF) << 8);
    setRegisterValue(dest_name, result, simulator);
}

void handle_ir_reverse_bytes_signed_halfword(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 2) { return; }
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    if (!std::holds_alternative<std::string>(dest_op) || !std::holds_alternative<std::string>(src_op)) { return; }

    const auto& dest_name = std::get<std::string>(dest_op);
    uint32_t src_val = getOperandValue(src_op, simulator);
    // Reverse bytes in lower 16 bits
    uint16_t bottom_half = src_val & 0xFFFF;
    uint16_t reversed_bottom = (bottom_half >> 8) | (bottom_half << 8);
    // Sign-extend from the new bit 7 of the reversed value
    int32_t result = static_cast<int16_t>(reversed_bottom);
    setRegisterValue(dest_name, result, simulator);
}

// Helper function for 32-bit signed saturating addition
int32_t saturate_add(int32_t a, int32_t b) {
    int64_t sum = static_cast<int64_t>(a) + static_cast<int64_t>(b);
    if (sum > std::numeric_limits<int32_t>::max()) {
        return std::numeric_limits<int32_t>::max();
    } else if (sum < std::numeric_limits<int32_t>::min()) {
        return std::numeric_limits<int32_t>::min();
    }
    return static_cast<int32_t>(sum);
}

// Helper function for 32-bit signed saturating subtraction
int32_t saturate_sub(int32_t a, int32_t b) {
    int64_t diff = static_cast<int64_t>(a) - static_cast<int64_t>(b);
    if (diff > std::numeric_limits<int32_t>::max()) {
        return std::numeric_limits<int32_t>::max();
    } else if (diff < std::numeric_limits<int32_t>::min()) {
        return std::numeric_limits<int32_t>::min();
    }
    return static_cast<int32_t>(diff);
}

void handle_ir_saturating_add(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 3) return;
    const auto& dest_op = ir_instr.operands[0];
    const auto& src1_op = ir_instr.operands[1];
    const auto& src2_op = ir_instr.operands[2];

    int32_t src1_val = static_cast<int32_t>(getOperandValue(src1_op, simulator));
    int32_t src2_val = static_cast<int32_t>(getOperandValue(src2_op, simulator));

    int32_t result = saturate_add(src1_val, src2_val);

    if (const std::string* dest_name = std::get_if<std::string>(&dest_op)) {
        setRegisterValue(*dest_name, result, simulator);
    }
    // Note: This simplified implementation does not set the Q flag.
}

void handle_ir_saturating_sub(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 3) return;
    const auto& dest_op = ir_instr.operands[0];
    const auto& src1_op = ir_instr.operands[1];
    const auto& src2_op = ir_instr.operands[2];

    int32_t src1_val = static_cast<int32_t>(getOperandValue(src1_op, simulator));
    int32_t src2_val = static_cast<int32_t>(getOperandValue(src2_op, simulator));

    int32_t result = saturate_sub(src1_val, src2_val);

    if (const std::string* dest_name = std::get_if<std::string>(&dest_op)) {
        setRegisterValue(*dest_name, result, simulator);
    }
    // Note: This simplified implementation does not set the Q flag.
}

void handle_ir_saturating_double_add(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 3) return;
    const auto& dest_op = ir_instr.operands[0];
    const auto& src1_op = ir_instr.operands[1];
    const auto& src2_op = ir_instr.operands[2];

    int32_t src1_val = static_cast<int32_t>(getOperandValue(src1_op, simulator));
    int32_t src2_val = static_cast<int32_t>(getOperandValue(src2_op, simulator));

    int32_t doubled_src2 = saturate_add(src2_val, src2_val);
    int32_t result = saturate_add(src1_val, doubled_src2);

    if (const std::string* dest_name = std::get_if<std::string>(&dest_op)) {
        setRegisterValue(*dest_name, result, simulator);
    }
    // Note: This simplified implementation does not set the Q flag.
}

void handle_ir_saturating_double_sub(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 3) return;
    const auto& dest_op = ir_instr.operands[0];
    const auto& src1_op = ir_instr.operands[1];
    const auto& src2_op = ir_instr.operands[2];

    int32_t src1_val = static_cast<int32_t>(getOperandValue(src1_op, simulator));
    int32_t src2_val = static_cast<int32_t>(getOperandValue(src2_op, simulator));

    int32_t doubled_src2 = saturate_add(src2_val, src2_val);
    int32_t result = saturate_sub(src1_val, doubled_src2);

    if (const std::string* dest_name = std::get_if<std::string>(&dest_op)) {
        setRegisterValue(*dest_name, result, simulator);
    }
    // Note: This simplified implementation does not set the Q flag.
}

void handle_ir_multiply_accumulate(const IRInstruction& ir_instr, ISimulator& simulator) {
    // MLA Rd, Rm, Rs, Rn  => Rd = (Rm * Rs) + Rn
    if (ir_instr.operands.size() != 4) return;
    const auto& rd_op = ir_instr.operands[0];
    const auto& rm_op = ir_instr.operands[1];
    const auto& rs_op = ir_instr.operands[2];
    const auto& rn_op = ir_instr.operands[3];

    int32_t rm_val = getOperandValue(rm_op, simulator);
    int32_t rs_val = getOperandValue(rs_op, simulator);
    int32_t rn_val = getOperandValue(rn_op, simulator);

    int32_t result = (rm_val * rs_val) + rn_val;

    if (const std::string* rd_name = std::get_if<std::string>(&rd_op)) {
        setRegisterValue(*rd_name, result, simulator);
    }
}

void handle_ir_multiply_subtract(const IRInstruction& ir_instr, ISimulator& simulator) {
    // MLS Rd, Rm, Rs, Rn  => Rd = Rn - (Rm * Rs)
    if (ir_instr.operands.size() != 4) return;
    const auto& rd_op = ir_instr.operands[0];
    const auto& rm_op = ir_instr.operands[1];
    const auto& rs_op = ir_instr.operands[2];
    const auto& rn_op = ir_instr.operands[3];

    int32_t rm_val = getOperandValue(rm_op, simulator);
    int32_t rs_val = getOperandValue(rs_op, simulator);
    int32_t rn_val = getOperandValue(rn_op, simulator);

    int32_t result = rn_val - (rm_val * rs_val);

    if (const std::string* rd_name = std::get_if<std::string>(&rd_op)) {
        setRegisterValue(*rd_name, result, simulator);
    }
}

void handle_ir_unsigned_multiply_long(const IRInstruction& ir_instr, ISimulator& simulator) {
    // UMULL RdLo, RdHi, Rm, Rs => {RdHi, RdLo} = Rm * Rs
    if (ir_instr.operands.size() != 4) return;
    const auto& rdlo_op = ir_instr.operands[0];
    const auto& rdhi_op = ir_instr.operands[1];
    const auto& rm_op = ir_instr.operands[2];
    const auto& rs_op = ir_instr.operands[3];

    uint32_t rm_val = getOperandValue(rm_op, simulator);
    uint32_t rs_val = getOperandValue(rs_op, simulator);

    uint64_t result = static_cast<uint64_t>(rm_val) * rs_val;

    if (const std::string* rdlo_name = std::get_if<std::string>(&rdlo_op)) {
        setRegisterValue(*rdlo_name, static_cast<uint32_t>(result), simulator);
    }
    if (const std::string* rdhi_name = std::get_if<std::string>(&rdhi_op)) {
        setRegisterValue(*rdhi_name, static_cast<uint32_t>(result >> 32), simulator);
    }
}

void handle_ir_signed_multiply_long(const IRInstruction& ir_instr, ISimulator& simulator) {
    // SMULL RdLo, RdHi, Rm, Rs => {RdHi, RdLo} = Rm * Rs
    if (ir_instr.operands.size() != 4) return;
    const auto& rdlo_op = ir_instr.operands[0];
    const auto& rdhi_op = ir_instr.operands[1];
    const auto& rm_op = ir_instr.operands[2];
    const auto& rs_op = ir_instr.operands[3];

    int32_t rm_val = getOperandValue(rm_op, simulator);
    int32_t rs_val = getOperandValue(rs_op, simulator);

    int64_t result = static_cast<int64_t>(rm_val) * rs_val;

    if (const std::string* rdlo_name = std::get_if<std::string>(&rdlo_op)) {
        setRegisterValue(*rdlo_name, static_cast<uint32_t>(result), simulator);
    }
    if (const std::string* rdhi_name = std::get_if<std::string>(&rdhi_op)) {
        setRegisterValue(*rdhi_name, static_cast<uint32_t>(result >> 32), simulator);
    }
}

void handle_ir_unsigned_multiply_accumulate_long(const IRInstruction& ir_instr, ISimulator& simulator) {
    // UMLAL RdLo, RdHi, Rm, Rs => {RdHi, RdLo} = (Rm * Rs) + {RdHi, RdLo}
    if (ir_instr.operands.size() != 4) return;
    const auto& rdlo_op = ir_instr.operands[0];
    const auto& rdhi_op = ir_instr.operands[1];
    const auto& rm_op = ir_instr.operands[2];
    const auto& rs_op = ir_instr.operands[3];

    uint32_t rm_val = getOperandValue(rm_op, simulator);
    uint32_t rs_val = getOperandValue(rs_op, simulator);
    uint32_t rdlo_val = getOperandValue(rdlo_op, simulator);
    uint32_t rdhi_val = getOperandValue(rdhi_op, simulator);

    uint64_t existing_val = (static_cast<uint64_t>(rdhi_val) << 32) | rdlo_val;
    uint64_t product = static_cast<uint64_t>(rm_val) * rs_val;
    uint64_t result = product + existing_val;

    if (const std::string* rdlo_name = std::get_if<std::string>(&rdlo_op)) {
        setRegisterValue(*rdlo_name, static_cast<uint32_t>(result), simulator);
    }
    if (const std::string* rdhi_name = std::get_if<std::string>(&rdhi_op)) {
        setRegisterValue(*rdhi_name, static_cast<uint32_t>(result >> 32), simulator);
    }
}

void handle_ir_signed_multiply_accumulate_long(const IRInstruction& ir_instr, ISimulator& simulator) {
    // SMLAL RdLo, RdHi, Rm, Rs => {RdHi, RdLo} = (Rm * Rs) + {RdHi, RdLo}
    if (ir_instr.operands.size() != 4) return;
    const auto& rdlo_op = ir_instr.operands[0];
    const auto& rdhi_op = ir_instr.operands[1];
    const auto& rm_op = ir_instr.operands[2];
    const auto& rs_op = ir_instr.operands[3];

    int32_t rm_val = getOperandValue(rm_op, simulator);
    int32_t rs_val = getOperandValue(rs_op, simulator);
    int32_t rdlo_val = getOperandValue(rdlo_op, simulator);
    int32_t rdhi_val = getOperandValue(rdhi_op, simulator);

    int64_t existing_val = (static_cast<int64_t>(rdhi_val) << 32) | static_cast<uint32_t>(rdlo_val);
    int64_t product = static_cast<int64_t>(rm_val) * rs_val;
    int64_t result = product + existing_val;

    if (const std::string* rdlo_name = std::get_if<std::string>(&rdlo_op)) {
        setRegisterValue(*rdlo_name, static_cast<uint32_t>(result), simulator);
    }
    if (const std::string* rdhi_name = std::get_if<std::string>(&rdhi_op)) {
        setRegisterValue(*rdhi_name, static_cast<uint32_t>(result >> 32), simulator);
    }
}

void handle_ir_breakpoint(const IRInstruction& ir_instr, ISimulator& simulator) {
    // TODO: Implement breakpoint logic, e.g., halt simulation, notify debugger.
    simulator.getDatabaseManager().log(simulator.get_session_id(), "Breakpoint instruction hit.", "INFO", 0, __FILE__, __LINE__);
}

void handle_ir_wait_for_interrupt(const IRInstruction& ir_instr, ISimulator& simulator) {
    // TODO: Implement WFI logic, e.g., pause core until an interrupt occurs.
    simulator.getDatabaseManager().log(simulator.get_session_id(), "WFI instruction encountered.", "INFO", 0, __FILE__, __LINE__);
}

void handle_ir_wait_for_event(const IRInstruction& ir_instr, ISimulator& simulator) {
    // TODO: Implement WFE logic.
    simulator.getDatabaseManager().log(simulator.get_session_id(), "WFE instruction encountered.", "INFO", 0, __FILE__, __LINE__);
}

void handle_ir_send_event(const IRInstruction& ir_instr, ISimulator& simulator) {
    // TODO: Implement SEV logic.
    simulator.getDatabaseManager().log(simulator.get_session_id(), "SEV instruction encountered.", "INFO", 0, __FILE__, __LINE__);
}

void handle_ir_compare_and_branch_if_not_zero(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 2) return;

    const auto& reg_op = ir_instr.operands[0];
    const auto& target_op = ir_instr.operands[1];

    int64_t reg_val = getOperandValue(reg_op, simulator);

    if (reg_val != 0) {
        address_t target_address = 0;
        if (std::holds_alternative<uint64_t>(target_op)) {
            target_address = std::get<uint64_t>(target_op);
        } else {
            return; // Invalid target
        }

        const char* ip_name = simulator.get_instruction_pointer_name();
        setRegisterValue(ip_name, target_address, simulator);
    }
}


void handle_ir_float_add_s(const IRInstruction& ir_instr, ISimulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src1_op = ir_instr.operands[1];
    const auto& src2_op = ir_instr.operands[2];
    float src1 = getFloatOperandValue(src1_op, simulator);
    float src2 = getFloatOperandValue(src2_op, simulator);
    float result = src1 + src2;
    setFloatRegisterValue(std::get<std::string>(dest_op), result, simulator);
}

void handle_ir_float_sub_s(const IRInstruction& ir_instr, ISimulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src1_op = ir_instr.operands[1];
    const auto& src2_op = ir_instr.operands[2];
    float src1 = getFloatOperandValue(src1_op, simulator);
    float src2 = getFloatOperandValue(src2_op, simulator);
    float result = src1 - src2;
    setFloatRegisterValue(std::get<std::string>(dest_op), result, simulator);
}

void handle_ir_float_mul_s(const IRInstruction& ir_instr, ISimulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src1_op = ir_instr.operands[1];
    const auto& src2_op = ir_instr.operands[2];
    float src1 = getFloatOperandValue(src1_op, simulator);
    float src2 = getFloatOperandValue(src2_op, simulator);
    float result = src1 * src2;
    setFloatRegisterValue(std::get<std::string>(dest_op), result, simulator);
}

void handle_ir_float_div_s(const IRInstruction& ir_instr, ISimulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src1_op = ir_instr.operands[1];
    const auto& src2_op = ir_instr.operands[2];
    float src1 = getFloatOperandValue(src1_op, simulator);
    float src2 = getFloatOperandValue(src2_op, simulator);
    float result = src1 / src2;
    setFloatRegisterValue(std::get<std::string>(dest_op), result, simulator);
}

void handle_ir_float_sqrt_s(const IRInstruction& ir_instr, ISimulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    float src = getFloatOperandValue(src_op, simulator);
    float result = std::sqrt(src);
    setFloatRegisterValue(std::get<std::string>(dest_op), result, simulator);
}

void handle_ir_float_add_d(const IRInstruction& ir_instr, ISimulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src1_op = ir_instr.operands[1];
    const auto& src2_op = ir_instr.operands[2];
    double src1 = getDoubleOperandValue(src1_op, simulator);
    double src2 = getDoubleOperandValue(src2_op, simulator);
    double result = src1 + src2;
    setDoubleRegisterValue(std::get<std::string>(dest_op), result, simulator);
}

void handle_ir_float_sub_d(const IRInstruction& ir_instr, ISimulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src1_op = ir_instr.operands[1];
    const auto& src2_op = ir_instr.operands[2];
    double src1 = getDoubleOperandValue(src1_op, simulator);
    double src2 = getDoubleOperandValue(src2_op, simulator);
    double result = src1 - src2;
    setDoubleRegisterValue(std::get<std::string>(dest_op), result, simulator);
}

void handle_ir_float_mul_d(const IRInstruction& ir_instr, ISimulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src1_op = ir_instr.operands[1];
    const auto& src2_op = ir_instr.operands[2];
    double src1 = getDoubleOperandValue(src1_op, simulator);
    double src2 = getDoubleOperandValue(src2_op, simulator);
    double result = src1 * src2;
    setDoubleRegisterValue(std::get<std::string>(dest_op), result, simulator);
}

void handle_ir_float_div_d(const IRInstruction& ir_instr, ISimulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src1_op = ir_instr.operands[1];
    const auto& src2_op = ir_instr.operands[2];
    double src1 = getDoubleOperandValue(src1_op, simulator);
    double src2 = getDoubleOperandValue(src2_op, simulator);
    double result = src1 / src2;
    setDoubleRegisterValue(std::get<std::string>(dest_op), result, simulator);
}

void handle_ir_float_sqrt_d(const IRInstruction& ir_instr, ISimulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    double src = getDoubleOperandValue(src_op, simulator);
    double result = std::sqrt(src);
    setDoubleRegisterValue(std::get<std::string>(dest_op), result, simulator);
}

void handle_ir_float_cmp_s(const IRInstruction& ir_instr, ISimulator& simulator) {
    const auto& src1_op = ir_instr.operands[0];
    const auto& src2_op = ir_instr.operands[1];
    float src1 = getFloatOperandValue(src1_op, simulator);
    float src2 = getFloatOperandValue(src2_op, simulator);
    if (src1 == src2) {
        simulator.set_ZF(true);
        simulator.set_CF(false);
    } else if (src1 < src2) {
        simulator.set_ZF(false);
        simulator.set_CF(true);
    } else {
        simulator.set_ZF(false);
        simulator.set_CF(false);
    }
}

void handle_ir_float_cmp_d(const IRInstruction& ir_instr, ISimulator& simulator) {
    const auto& src1_op = ir_instr.operands[0];
    const auto& src2_op = ir_instr.operands[1];
    double src1 = getDoubleOperandValue(src1_op, simulator);
    double src2 = getDoubleOperandValue(src2_op, simulator);
    if (src1 == src2) {
        simulator.set_ZF(true);
        simulator.set_CF(false);
    } else if (src1 < src2) {
        simulator.set_ZF(false);
        simulator.set_CF(true);
    } else {
        simulator.set_ZF(false);
        simulator.set_CF(false);
    }
}

void handle_ir_float_to_s(const IRInstruction& ir_instr, ISimulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    double src = getDoubleOperandValue(src_op, simulator);
    float result = static_cast<float>(src);
    setFloatRegisterValue(std::get<std::string>(dest_op), result, simulator);
}

void handle_ir_float_to_d(const IRInstruction& ir_instr, ISimulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    float src = getFloatOperandValue(src_op, simulator);
    double result = static_cast<double>(src);
    setDoubleRegisterValue(std::get<std::string>(dest_op), result, simulator);
}

void handle_ir_int_to_float_s(const IRInstruction& ir_instr, ISimulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    int64_t src = getOperandValue(src_op, simulator);
    float result = static_cast<float>(src);
    setFloatRegisterValue(std::get<std::string>(dest_op), result, simulator);
}

void handle_ir_int_to_float_d(const IRInstruction& ir_instr, ISimulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    int64_t src = getOperandValue(src_op, simulator);
    double result = static_cast<double>(src);
    setDoubleRegisterValue(std::get<std::string>(dest_op), result, simulator);
}

void handle_ir_float_to_int_s(const IRInstruction& ir_instr, ISimulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    float src = getFloatOperandValue(src_op, simulator);
    int64_t result = static_cast<int64_t>(src);
    setRegisterValue(std::get<std::string>(dest_op), result, simulator);
}

void handle_ir_float_to_int_d(const IRInstruction& ir_instr, ISimulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    double src = getDoubleOperandValue(src_op, simulator);
    int64_t result = static_cast<int64_t>(src);
    setRegisterValue(std::get<std::string>(dest_op), result, simulator);
}

