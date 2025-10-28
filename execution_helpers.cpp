#include "execution_helpers.h"
#include "i_simulator.h"
#include "architecture.h"
#include "i_register_map.h"
#include "memory.h"
#include <stdexcept>

int64_t getOperandValue(const IROperand& op, ISimulator& simulator) {
    if (std::holds_alternative<std::string>(op)) {
        const auto& reg_name = std::get<std::string>(op);
        const auto& arch = simulator.get_architecture();
        if (!arch.is_register(reg_name)) {
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
