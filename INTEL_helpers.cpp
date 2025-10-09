#include "INTEL_helpers.h"
#include "x86_simulator.h"
#include <variant>

// NOTE: This implementation assumes that the X86Simulator class has a method:
// const Architecture& get_architecture() const;

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

        // 1. Calculate the effective address
        address_t addr = mem_op.displacement;
        if (mem_op.base_reg) {
            const std::string& reg_name = arch.get_register_name(*mem_op.base_reg);
            addr += regs.get64(reg_name); // Assume 64-bit address calculation
        }
        if (mem_op.index_reg) {
            const std::string& reg_name = arch.get_register_name(*mem_op.index_reg);
            uint64_t index_val = regs.get64(reg_name);
            addr += index_val * mem_op.scale;
        }

        // 2. Read from memory using the size specified in the operand
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

    // 1. Calculate the effective address
    address_t addr = mem_op.displacement;
    if (mem_op.base_reg) {
        const std::string& reg_name = arch.get_register_name(*mem_op.base_reg);
        addr += regs.get64(reg_name); // Assume 64-bit address calculation
    }
    if (mem_op.index_reg) {
        const std::string& reg_name = arch.get_register_name(*mem_op.index_reg);
        uint64_t index_val = regs.get64(reg_name);
        addr += index_val * mem_op.scale;
    }

    // 2. Write to memory using the size specified in the operand
    switch (mem_op.size) {
        case 8:   mem.write_byte(addr, value); break;
        case 16:  mem.write_word(addr, value); break;
        case 32:  mem.write_dword(addr, value); break;
        case 64:  mem.write_qword(addr, value); break;
        default:
            throw std::runtime_error("Unsupported memory access size in setMemoryValue: " + std::to_string(mem_op.size));
    }
}


void handle_ir_add(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    const auto& dest_reg = std::get<IRRegister>(dest_op);

    uint32_t destValue = getOperandValue(dest_op, simulator);
    uint32_t sourceValue = getOperandValue(src_op, simulator);
    uint32_t result = destValue + sourceValue;
    setRegisterValue(dest_reg, result, simulator);

    simulator.set_ZF(result == 0);
    simulator.set_SF((result & 0x80000000) != 0);
    simulator.set_CF(result < destValue);
}

void handle_ir_move(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    const auto& dest_reg = std::get<IRRegister>(dest_op);
    uint64_t sourceValue = getOperandValue(src_op, simulator);
    setRegisterValue(dest_reg, sourceValue, simulator);
}

void handle_ir_load(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    const auto& dest_reg = std::get<IRRegister>(dest_op);
    uint64_t sourceValue = getOperandValue(src_op, simulator);
    setRegisterValue(dest_reg, sourceValue, simulator);
}

void handle_ir_store(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    const auto& dest_mem = std::get<IRMemoryOperand>(dest_op);
    uint64_t sourceValue = getOperandValue(src_op, simulator);
    setMemoryValue(dest_mem, sourceValue, simulator);
}

void handle_ir_jump(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& target_op = ir_instr.operands[0];
    address_t target_address = std::get<uint64_t>(target_op);
    simulator.getRegisterMap().set64("rip", target_address);
}

void handle_ir_branch(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& target_op = ir_instr.operands[0];
    const auto& cond_op = ir_instr.operands[1];
    address_t target_address = std::get<uint64_t>(target_op);
    const auto condition = std::get<IRConditionCode>(cond_op);
    bool should_jump = false;
    switch (condition) {
        case IRConditionCode::NotEqual: should_jump = !simulator.get_ZF(); break;
        case IRConditionCode::Equal: should_jump = simulator.get_ZF(); break;
        case IRConditionCode::Greater: should_jump = (simulator.get_SF() == simulator.get_OF()) && !simulator.get_ZF(); break;
        case IRConditionCode::GreaterOrEqual: should_jump = (simulator.get_SF() == simulator.get_OF()); break;
        default: break;
    }
    if (should_jump) {
        simulator.getRegisterMap().set64("rip", target_address);
    }
}

void handle_ir_cmp(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& op1 = ir_instr.operands[0];
    const auto& op2 = ir_instr.operands[1];
    uint32_t val1 = getOperandValue(op1, simulator);
    uint32_t val2 = getOperandValue(op2, simulator);
    uint32_t result = val1 - val2;

    simulator.set_ZF(result == 0);
    simulator.set_SF((result & 0x80000000) != 0);
    simulator.set_CF(val1 < val2);
    bool val1_sign = (val1 & 0x80000000);
    bool val2_sign = (val2 & 0x80000000);
    bool result_sign = (result & 0x80000000);
    simulator.set_OF((val1_sign != val2_sign) && (val1_sign != result_sign));
    simulator.set_AF((val1 & 0xF) < (val2 & 0xF));
    uint8_t lsb = result & 0xFF;
    int set_bits = 0;
    for (int i = 0; i < 8; ++i) { if ((lsb >> i) & 1) { set_bits++; } }
    simulator.set_PF((set_bits % 2) == 0);
}

void handle_ir_inc(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& op = ir_instr.operands[0];
    const auto& dest_reg = std::get<IRRegister>(op);
    uint32_t value = getOperandValue(op, simulator);
    uint32_t result = value + 1;
    setRegisterValue(dest_reg, result, simulator);

    simulator.set_ZF(result == 0);
    simulator.set_SF((result & 0x80000000) != 0);
    simulator.set_OF(value == 0x7FFFFFFF);
    simulator.set_AF(((value & 0xF) + 1) > 0xF);
    uint8_t lsb = result & 0xFF;
    int set_bits = 0;
    for (int i = 0; i < 8; ++i) { if ((lsb >> i) & 1) { set_bits++; } }
    simulator.set_PF((set_bits % 2) == 0);
}

void handle_ir_syscall(const IRInstruction& ir_instr, X86Simulator& simulator) {
    uint8_t interrupt_vector = std::get<uint64_t>(ir_instr.operands[0]);
    if (interrupt_vector == 0x80) { // Linux syscall
        auto& regs = simulator.getRegisterMap();
        uint32_t syscall_num = regs.get32("eax");
        if (syscall_num == 1) { // sys_exit
            uint32_t exit_code = regs.get32("ebx");
            simulator.getDatabaseManager().log(simulator.get_session_id(), "Program exited via sys_exit with code: " + std::to_string(exit_code), "INFO", 0, __FILE__, __LINE__);
            regs.set64("rip", simulator.getMemory().get_total_memory_size());
        }
    }
}

void handle_ir_mul(const IRInstruction& ir_instr, X86Simulator& simulator) {
    auto& regs = simulator.getRegisterMap();
    const auto& src_op = ir_instr.operands[0];
    uint32_t src_val = getOperandValue(src_op, simulator);
    uint64_t val_eax = regs.get32("eax");
    uint64_t result = val_eax * static_cast<uint64_t>(src_val);
    regs.set32("eax", static_cast<uint32_t>(result & 0xFFFFFFFF));
    regs.set32("edx", static_cast<uint32_t>(result >> 32));
    bool overflow = (regs.get32("edx") != 0);
    simulator.set_CF(overflow);
    simulator.set_OF(overflow);
}

void handle_ir_imul(const IRInstruction& ir_instr, X86Simulator& simulator) {
    auto& regs = simulator.getRegisterMap();
    const auto& src_op = ir_instr.operands[0];
    int32_t src_val = getOperandValue(src_op, simulator);
    int64_t val_eax = static_cast<int32_t>(regs.get32("eax"));
    int64_t result = val_eax * static_cast<int64_t>(src_val);
    uint32_t result_low = static_cast<uint32_t>(result & 0xFFFFFFFF);
    uint32_t result_high = static_cast<uint32_t>(result >> 32);
    regs.set32("eax", result_low);
    regs.set32("edx", result_high);
    bool fits = (result_high == 0 && (result_low & 0x80000000) == 0) || (result_high == 0xFFFFFFFF && (result_low & 0x80000000) != 0);
    simulator.set_CF(!fits);
    simulator.set_OF(!fits);
}

void handle_ir_dec(const IRInstruction& ir_instr, X86Simulator& simulator) {
    const auto& op = ir_instr.operands[0];
    const auto& dest_reg = std::get<IRRegister>(op);
    uint32_t value = getOperandValue(op, simulator);
    uint32_t result = value - 1;
    setRegisterValue(dest_reg, result, simulator);

    simulator.set_ZF(result == 0);
    simulator.set_SF((result & 0x80000000) != 0);
    simulator.set_OF(value == 0x80000000);
    simulator.set_AF((value & 0xF) < 1);
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
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    const auto& dest_reg = std::get<IRRegister>(dest_op);

    uint32_t destValue = getOperandValue(dest_op, simulator);
    uint32_t sourceValue = getOperandValue(src_op, simulator);
    uint32_t result = destValue ^ sourceValue;
    setRegisterValue(dest_reg, result, simulator);

    simulator.set_ZF(result == 0);
    simulator.set_SF((result & 0x80000000) != 0);
    simulator.set_CF(false);
    simulator.set_OF(false);

    // Calculate Parity Flag
    uint8_t lsb = result & 0xFF;
    int set_bits = 0;
    for (int i = 0; i < 8; ++i) {
        if ((lsb >> i) & 1) {
            set_bits++;
        }
    }
    simulator.set_PF((set_bits % 2) == 0);
}