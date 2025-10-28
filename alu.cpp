#include "alu.h"
#include "i_simulator.h"
#include "i_database_manager.h"
#include "architecture.h"
#include "i_register_map.h"
#include "memory.h"
#include <iostream>
#include <variant>
#include <limits>
#include <stdexcept>

#include "alu.h"
#include "i_simulator.h"
#include "i_database_manager.h"
#include "architecture.h"
#include "i_register_map.h"
#include "memory.h"
#include <iostream>
#include <variant>
#include <limits>
#include <stdexcept>
#include "execution_helpers.h"

namespace {

template<typename Func>
void handle_binary_op(const IRInstruction& ir_instr, ISimulator& simulator, Func op_func) {
    const auto& dest_op = ir_instr.operands[0];
    int64_t src1_val, src2_val;

    if (ir_instr.operands.size() == 3) { // 3-operand form (dest, src1, src2)
        src1_val = getOperandValue(ir_instr.operands[1], simulator);
        src2_val = getOperandValue(ir_instr.operands[2], simulator);
    } else { // 2-operand form (dest, src)
        src1_val = getOperandValue(ir_instr.operands[0], simulator); // dest is also src1
        src2_val = getOperandValue(ir_instr.operands[1], simulator);
    }

    uint32_t size = 0;
    if (const std::string* reg_name = std::get_if<std::string>(&dest_op)) {
        size = simulator.get_architecture().get_register_size_bits(*reg_name);
    } else if (const IRMemoryOperand* mem = std::get_if<IRMemoryOperand>(&dest_op)) {
        size = mem->size;
    }

    op_func(src1_val, src2_val, size, simulator);
}

template<typename Func>
void handle_unary_op(const IRInstruction& ir_instr, ISimulator& simulator, Func op_func) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[0];
    
    int64_t src_val = getOperandValue(src_op, simulator);
    uint32_t size = 0;
    if (const std::string* reg_name = std::get_if<std::string>(&dest_op)) {
        size = simulator.get_architecture().get_register_size_bits(*reg_name);
    } else if (const IRMemoryOperand* mem = std::get_if<IRMemoryOperand>(&dest_op)) {
        size = mem->size;
    }

    op_func(src_val, size, simulator);
}

} // anonymous namespace

void ALU::execute(PipelinedInstruction& instruction, ISimulator& simulator) {
    switch (instruction.ir_instruction.opcode) {
        case IROpcode::Add:             handle_ir_add(instruction.ir_instruction, simulator); break;
        case IROpcode::Sub:             handle_ir_sub(instruction.ir_instruction, simulator); break;
        case IROpcode::AddC:            handle_ir_addc(instruction.ir_instruction, simulator); break;
        case IROpcode::SubC:            handle_ir_subc(instruction.ir_instruction, simulator); break;
        case IROpcode::Mul:             handle_ir_mul(instruction.ir_instruction, simulator); break;
        case IROpcode::IMul:            handle_ir_imul(instruction.ir_instruction, simulator); break;
        case IROpcode::Div:             handle_ir_div(instruction.ir_instruction, simulator); break;
        case IROpcode::And:             handle_ir_and(instruction.ir_instruction, simulator); break;
        case IROpcode::Or:              handle_ir_or(instruction.ir_instruction, simulator); break;
        case IROpcode::Xor:             handle_ir_xor(instruction.ir_instruction, simulator); break;
        case IROpcode::Not:             handle_ir_not(instruction.ir_instruction, simulator); break;
        case IROpcode::Shl:             handle_ir_shl(instruction.ir_instruction, simulator); break;
        case IROpcode::Shr:             handle_ir_shr(instruction.ir_instruction, simulator); break;
        case IROpcode::Sar:             handle_ir_sar(instruction.ir_instruction, simulator); break;
        case IROpcode::Inc:             handle_ir_inc(instruction.ir_instruction, simulator); break;
        case IROpcode::Dec:             handle_ir_dec(instruction.ir_instruction, simulator); break;
        case IROpcode::Cmp:             handle_ir_cmp(instruction.ir_instruction, simulator); break;
        case IROpcode::Tst:             handle_ir_tst(instruction.ir_instruction, simulator); break;
        case IROpcode::Teq:             handle_ir_teq(instruction.ir_instruction, simulator); break;
        case IROpcode::Cmn:             handle_ir_cmn(instruction.ir_instruction, simulator); break;
        case IROpcode::MoveNot:         handle_ir_movenot(instruction.ir_instruction, simulator); break;
        case IROpcode::AndNot:          handle_ir_andnot(instruction.ir_instruction, simulator); break;
        case IROpcode::Swap:            handle_ir_swap(instruction.ir_instruction, simulator); break;
        case IROpcode::MoveToSystemRegister: handle_ir_move_to_system_register(instruction.ir_instruction, simulator); break;
        case IROpcode::MoveFromSystemRegister: handle_ir_move_from_system_register(instruction.ir_instruction, simulator); break;
        case IROpcode::CountLeadingZeros: handle_ir_count_leading_zeros(instruction.ir_instruction, simulator); break;
        case IROpcode::ReverseBits:     handle_ir_reverse_bits(instruction.ir_instruction, simulator); break;
        case IROpcode::ReverseBytes:    handle_ir_reverse_bytes(instruction.ir_instruction, simulator); break;
        case IROpcode::ReverseBytes16:  handle_ir_reverse_bytes16(instruction.ir_instruction, simulator); break;
        case IROpcode::ReverseBytesSignedHalfword: handle_ir_reverse_bytes_signed_halfword(instruction.ir_instruction, simulator); break;
        case IROpcode::SaturatingAdd:   handle_ir_saturating_add(instruction.ir_instruction, simulator); break;
        case IROpcode::SaturatingSub:   handle_ir_saturating_sub(instruction.ir_instruction, simulator); break;
        case IROpcode::SaturatingDoubleAdd: handle_ir_saturating_double_add(instruction.ir_instruction, simulator); break;
        case IROpcode::SaturatingDoubleSub: handle_ir_saturating_double_sub(instruction.ir_instruction, simulator); break;
        case IROpcode::MultiplyAccumulate: handle_ir_multiply_accumulate(instruction.ir_instruction, simulator); break;
        case IROpcode::MultiplySubtract: handle_ir_multiply_subtract(instruction.ir_instruction, simulator); break;
        case IROpcode::UnsignedMultiplyLong: handle_ir_unsigned_multiply_long(instruction.ir_instruction, simulator); break;
        case IROpcode::SignedMultiplyLong: handle_ir_signed_multiply_long(instruction.ir_instruction, simulator); break;
        case IROpcode::UnsignedMultiplyAccumulateLong: handle_ir_unsigned_multiply_accumulate_long(instruction.ir_instruction, simulator); break;
        case IROpcode::SignedMultiplyAccumulateLong: handle_ir_signed_multiply_accumulate_long(instruction.ir_instruction, simulator); break;
        case IROpcode::CompareAndBranchIfNotZero: handle_ir_compare_and_branch_if_not_zero(instruction.ir_instruction, simulator); break;
        case IROpcode::Move: handle_ir_move(instruction.ir_instruction, simulator); break;
        case IROpcode::Load: handle_ir_load(instruction.ir_instruction, simulator); break;
        case IROpcode::Store: handle_ir_store(instruction.ir_instruction, simulator); break;
        case IROpcode::Push: handle_ir_push(instruction.ir_instruction, simulator); break;
        case IROpcode::Pop: handle_ir_pop(instruction.ir_instruction, simulator); break;
        case IROpcode::Ret: handle_ir_ret(instruction.ir_instruction, simulator); break;
        default:
            break;
    }
}

bool ALU::is_busy() const { return false; }
int ALU::latency() const { return 1; }

void ALU::handle_ir_add(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_op(ir_instr, simulator, [&](int64_t val1, int64_t val2, uint32_t size, ISimulator& sim) {
        const auto& dest_op = ir_instr.operands[0];
        const auto& dest_name = std::get<std::string>(dest_op);
        switch (size) {
            case 32: {
                uint32_t u_val1 = static_cast<uint32_t>(val1);
                uint32_t u_val2 = static_cast<uint32_t>(val2);
                uint64_t result64 = static_cast<uint64_t>(u_val1) + static_cast<uint64_t>(u_val2);
                uint32_t result = static_cast<uint32_t>(result64);
                setRegisterValue(dest_name, result, sim);

                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x80000000) != 0);
                sim.set_CF(result64 > 0xFFFFFFFF);

                bool val1_sign = (u_val1 & 0x80000000) != 0;
                bool val2_sign = (u_val2 & 0x80000000) != 0;
                bool result_sign = (result & 0x80000000) != 0;
                sim.set_OF((val1_sign == val2_sign) && (val1_sign != result_sign));
                break;
            }
            default:
                sim.getDatabaseManager().log(sim.get_session_id(), "Unsupported register size for IR Add", "ERROR", 0, __FILE__, __LINE__);
                break;
        }
    });
}

void ALU::handle_ir_sub(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_op(ir_instr, simulator, [&](int64_t val1, int64_t val2, uint32_t size, ISimulator& sim) {
        const auto& dest_op = ir_instr.operands[0];
        const auto& dest_name = std::get<std::string>(dest_op);
        switch (size) {
            case 32: {
                int64_t result64 = val1 - val2;
                uint32_t result = static_cast<uint32_t>(result64);
                setRegisterValue(dest_name, result, sim);
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x80000000) != 0);
                sim.set_CF(static_cast<uint64_t>(val1) < static_cast<uint64_t>(val2));
                // TODO: Set OF, AF, PF for 32-bit
                break;
            }
            default:
                sim.getDatabaseManager().log(sim.get_session_id(), "Unsupported register size for IR Sub", "ERROR", 0, __FILE__, __LINE__);
                break;
        }
    });
}

void ALU::handle_ir_addc(const IRInstruction& ir_instr, ISimulator& simulator) {
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

void ALU::handle_ir_subc(const IRInstruction& ir_instr, ISimulator& simulator) {
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

void ALU::handle_ir_mul(const IRInstruction& ir_instr, ISimulator& simulator) {
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

void ALU::handle_ir_imul(const IRInstruction& ir_instr, ISimulator& simulator) {
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

void ALU::handle_ir_div(const IRInstruction& ir_instr, ISimulator& simulator) {
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

void ALU::handle_ir_and(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_op(ir_instr, simulator, [&](int64_t val1, int64_t val2, uint32_t size, ISimulator& sim) {
        const auto& dest_op = ir_instr.operands[0];
        const auto& dest_name = std::get<std::string>(dest_op);

        // AND instruction clears CF and OF.
        sim.set_CF(false);
        sim.set_OF(false);

        switch (size) {
            case 8: {
                uint8_t result = static_cast<uint8_t>(val1) & static_cast<uint8_t>(val2);
                setRegisterValue(dest_name, result, sim);
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x80) != 0);
                break;
            }
            case 16: {
                uint16_t result = static_cast<uint16_t>(val1) & static_cast<uint16_t>(val2);
                setRegisterValue(dest_name, result, sim);
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x8000) != 0);
                break;
            }
            case 32: {
                uint32_t result = static_cast<uint32_t>(val1) & static_cast<uint32_t>(val2);
                setRegisterValue(dest_name, result, sim);
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x80000000) != 0);
                break;
            }
            case 64: {
                uint64_t result = val1 & val2;
                setRegisterValue(dest_name, result, sim);
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x8000000000000000ULL) != 0);
                break;
            }
            default: return;
        }
    });
}

void ALU::handle_ir_or(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_op(ir_instr, simulator, [&](int64_t val1, int64_t val2, uint32_t size, ISimulator& sim) {
        const auto& dest_op = ir_instr.operands[0];
        const auto& dest_name = std::get<std::string>(dest_op);

        // OR instruction clears CF and OF.
        sim.set_CF(false);
        sim.set_OF(false);

        switch (size) {
            case 8: {
                uint8_t result = static_cast<uint8_t>(val1) | static_cast<uint8_t>(val2);
                setRegisterValue(dest_name, result, sim);
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x80) != 0);
                break;
            }
            case 16: {
                uint16_t result = static_cast<uint16_t>(val1) | static_cast<uint16_t>(val2);
                setRegisterValue(dest_name, result, sim);
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x8000) != 0);
                break;
            }
            case 32: {
                uint32_t result = static_cast<uint32_t>(val1) | static_cast<uint32_t>(val2);
                setRegisterValue(dest_name, result, sim);
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x80000000) != 0);
                break;
            }
            case 64: {
                uint64_t result = val1 | val2;
                setRegisterValue(dest_name, result, sim);
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x8000000000000000ULL) != 0);
                break;
            }
            default: return;
        }
    });
}

void ALU::handle_ir_xor(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_op(ir_instr, simulator, [&](int64_t val1, int64_t val2, uint32_t size, ISimulator& sim) {
        const auto& dest_op = ir_instr.operands[0];
        const auto& dest_name = std::get<std::string>(dest_op);

        // XOR instruction clears CF and OF.
        sim.set_CF(false);
        sim.set_OF(false);

        switch (size) {
            case 8: {
                uint8_t result = static_cast<uint8_t>(val1) ^ static_cast<uint8_t>(val2);
                setRegisterValue(dest_name, result, sim);
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x80) != 0);
                break;
            }
            case 16: {
                uint16_t result = static_cast<uint16_t>(val1) ^ static_cast<uint16_t>(val2);
                setRegisterValue(dest_name, result, sim);
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x8000) != 0);
                break;
            }
            case 32: {
                uint32_t result = static_cast<uint32_t>(val1) ^ static_cast<uint32_t>(val2);
                setRegisterValue(dest_name, result, sim);
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x80000000) != 0);
                break;
            }
            case 64: {
                uint64_t result = val1 ^ val2;
                setRegisterValue(dest_name, result, sim);
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x8000000000000000ULL) != 0);
                break;
            }
            default: return;
        }
    });
}

void ALU::handle_ir_not(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_unary_op(ir_instr, simulator, [&](int64_t val, uint32_t size, ISimulator& sim) {
        const auto& dest_op = ir_instr.operands[0];
        const auto& dest_name = std::get<std::string>(dest_op);
        uint64_t result = ~val;
        setRegisterValue(dest_name, result, sim);
    });
}

void ALU::handle_ir_shl(const IRInstruction& ir_instr, ISimulator& simulator) {
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

void ALU::handle_ir_shr(const IRInstruction& ir_instr, ISimulator& simulator) {
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

void ALU::handle_ir_sar(const IRInstruction& ir_instr, ISimulator& simulator) {
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

void ALU::handle_ir_inc(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_unary_op(ir_instr, simulator, [&](int64_t val, uint32_t size, ISimulator& sim) {
        const auto& dest_op = ir_instr.operands[0];
        const auto& dest_name = std::get<std::string>(dest_op);

        switch (size) {
            case 8: {
                int8_t value = val;
                uint8_t result = value + 1;
                setRegisterValue(dest_name, result, sim);
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x80) != 0);
                sim.set_OF(value == 0x7F);
                break;
            }
            case 16: {
                int16_t value = val;
                uint16_t result = value + 1;
                setRegisterValue(dest_name, result, sim);
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x8000) != 0);
                sim.set_OF(value == 0x7FFF);
                break;
            }
            case 32: {
                int32_t value = val;
                uint32_t result = value + 1;
                setRegisterValue(dest_name, result, sim);
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x80000000) != 0);
                sim.set_OF(value == 0x7FFFFFFF);
                break;
            }
            case 64: {
                int64_t value = val;
                uint64_t result = value + 1;
                setRegisterValue(dest_name, result, sim);
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x8000000000000000ULL) != 0);
                sim.set_OF(value == 0x7FFFFFFFFFFFFFFFULL);
                break;
            }
            default:
                sim.getDatabaseManager().log(sim.get_session_id(), "Unsupported operand size for IR Inc", "ERROR", 0, __FILE__, __LINE__);
                break;
        }
    });
}

void ALU::handle_ir_dec(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_unary_op(ir_instr, simulator, [&](int64_t val, uint32_t size, ISimulator& sim) {
        const auto& dest_op = ir_instr.operands[0];
        const auto& dest_name = std::get<std::string>(dest_op);

        switch (size) {
            case 8: {
                int8_t value = val;
                uint8_t result = value - 1;
                setRegisterValue(dest_name, result, sim);
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x80) != 0);
                sim.set_OF(value == 0x80);
                break;
            }
            case 16: {
                int16_t value = val;
                uint16_t result = value - 1;
                setRegisterValue(dest_name, result, sim);
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x8000) != 0);
                sim.set_OF(value == 0x8000);
                break;
            }
            case 32: {
                int32_t value = val;
                uint32_t result = value - 1;
                setRegisterValue(dest_name, result, sim);
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x80000000) != 0);
                sim.set_OF(static_cast<uint32_t>(value) == 0x80000000);
                break;
            }
            case 64: {
                int64_t value = val;
                uint64_t result = value - 1;
                setRegisterValue(dest_name, result, sim);
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x8000000000000000ULL) != 0);
                sim.set_OF(static_cast<uint64_t>(value) == 0x8000000000000000ULL);
                break;
            }
            default:
                sim.getDatabaseManager().log(sim.get_session_id(), "Unsupported operand size for IR Dec", "ERROR", 0, __FILE__, __LINE__);
                break;
        }
    });
}

void ALU::handle_ir_cmp(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_op(ir_instr, simulator, [&](int64_t val1, int64_t val2, uint32_t size, ISimulator& sim) {
        switch (size) {
            case 8: {
                int8_t result = val1 - val2;
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x80) != 0);
                sim.set_CF(static_cast<uint8_t>(val1) < static_cast<uint8_t>(val2));
                bool val1_sign = (val1 & 0x80) != 0;
                bool val2_sign = (val2 & 0x80) != 0;
                bool result_sign = (result & 0x80) != 0;
                sim.set_OF((val1_sign != val2_sign) && (result_sign != val1_sign));
                break;
            }
            case 16: {
                int16_t result = val1 - val2;
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x8000) != 0);
                sim.set_CF(static_cast<uint16_t>(val1) < static_cast<uint16_t>(val2));
                bool val1_sign = (val1 & 0x8000) != 0;
                bool val2_sign = (val2 & 0x8000) != 0;
                bool result_sign = (result & 0x8000) != 0;
                sim.set_OF((val1_sign != val2_sign) && (result_sign != val1_sign));
                break;
            }
            case 32: {
                int32_t result = val1 - val2;
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x80000000) != 0);
                sim.set_CF(static_cast<uint32_t>(val1) < static_cast<uint32_t>(val2));
                bool val1_sign = (val1 & 0x80000000) != 0;
                bool val2_sign = (val2 & 0x80000000) != 0;
                bool result_sign = (result & 0x80000000) != 0;
                sim.set_OF((val1_sign != val2_sign) && (result_sign != val1_sign));
                break;
            }
            case 64: {
                int64_t result = val1 - val2;
                sim.set_ZF(result == 0);
                sim.set_SF((result & 0x8000000000000000ULL) != 0);
                sim.set_CF(static_cast<uint64_t>(val1) < static_cast<uint64_t>(val2));
                bool val1_sign = (val1 & 0x8000000000000000ULL) != 0;
                bool val2_sign = (val2 & 0x8000000000000000ULL) != 0;
                bool result_sign = (result & 0x8000000000000000ULL) != 0;
                sim.set_OF((val1_sign != val2_sign) && (result_sign != val1_sign));
                break;
            }
            default:
                sim.getDatabaseManager().log(sim.get_session_id(), "Unsupported operand size for IR Cmp", "ERROR", 0, __FILE__, __LINE__);
                break;
        }
    });
}

void ALU::handle_ir_tst(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_op(ir_instr, simulator, [&](int64_t val1, int64_t val2, uint32_t size, ISimulator& sim) {
        uint32_t result = static_cast<uint32_t>(val1) & static_cast<uint32_t>(val2);
        sim.set_ZF(result == 0);
        sim.set_SF((result & 0x80000000) != 0);
    });
}

void ALU::handle_ir_teq(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_op(ir_instr, simulator, [&](int64_t val1, int64_t val2, uint32_t size, ISimulator& sim) {
        uint32_t result = static_cast<uint32_t>(val1) ^ static_cast<uint32_t>(val2);
        sim.set_ZF(result == 0);
        sim.set_SF((result & 0x80000000) != 0);
    });
}

void ALU::handle_ir_cmn(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_op(ir_instr, simulator, [&](int64_t val1, int64_t val2, uint32_t size, ISimulator& sim) {
        uint64_t result64 = static_cast<uint64_t>(val1) + static_cast<uint64_t>(val2);
        uint32_t result32 = static_cast<uint32_t>(result64);
        sim.set_ZF(result32 == 0);
        sim.set_SF((result32 & 0x80000000) != 0);
        sim.set_CF(result64 > 0xFFFFFFFF);
        // TODO: Set OF
    });
}

void ALU::handle_ir_movenot(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 2) { return; }
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    if (!std::holds_alternative<std::string>(dest_op)) { return; }
    const auto& dest_name = std::get<std::string>(dest_op);
    int64_t src_val = getOperandValue(src_op, simulator);
    uint64_t result = ~static_cast<uint64_t>(src_val);
    setRegisterValue(dest_name, result, simulator);
}

void ALU::handle_ir_andnot(const IRInstruction& ir_instr, ISimulator& simulator) {
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

void ALU::handle_ir_swap(const IRInstruction& ir_instr, ISimulator& simulator) {
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

void ALU::handle_ir_move_to_system_register(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 2) { return; }

    const auto& dest_op = ir_instr.operands[0]; // System register name (string)
    const auto& src_op = ir_instr.operands[1];  // Source GPR

    if (!std::holds_alternative<std::string>(dest_op)) { return; }

    const std::string& sys_reg_name = std::get<std::string>(dest_op);
    uint64_t value = getOperandValue(src_op, simulator);

    simulator.set_system_register(sys_reg_name, value);
}

void ALU::handle_ir_move_from_system_register(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 2) { return; }

    const auto& dest_op = ir_instr.operands[0]; // Destination GPR
    const auto& src_op = ir_instr.operands[1];  // System register name (string)

    if (!std::holds_alternative<std::string>(dest_op) || !std::holds_alternative<std::string>(src_op)) { return; }

    const auto& dest_name = std::get<std::string>(dest_op);
    const std::string& sys_reg_name = std::get<std::string>(src_op);

    uint64_t value = simulator.get_system_register(sys_reg_name);
    setRegisterValue(dest_name, value, simulator);
}

void ALU::handle_ir_count_leading_zeros(const IRInstruction& ir_instr, ISimulator& simulator) {
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

void ALU::handle_ir_reverse_bits(const IRInstruction& ir_instr, ISimulator& simulator) {
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

void ALU::handle_ir_reverse_bytes(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 2) { return; }
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    if (!std::holds_alternative<std::string>(dest_op) || !std::holds_alternative<std::string>(src_op)) { return; }

    const auto& dest_name = std::get<std::string>(dest_op);
    uint32_t src_val = getOperandValue(src_op, simulator);
    uint32_t result = __builtin_bswap32(src_val);
    setRegisterValue(dest_name, result, simulator);
}

void ALU::handle_ir_reverse_bytes16(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 2) { return; }
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    if (!std::holds_alternative<std::string>(dest_op) || !std::holds_alternative<std::string>(src_op)) { return; }

    const auto& dest_name = std::get<std::string>(dest_op);
    uint32_t src_val = getOperandValue(src_op, simulator);
    uint32_t result = ((src_val & 0xFF00FF00) >> 8) | ((src_val & 0x00FF00FF) << 8);
    setRegisterValue(dest_name, result, simulator);
}

void ALU::handle_ir_reverse_bytes_signed_halfword(const IRInstruction& ir_instr, ISimulator& simulator) {
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

void ALU::handle_ir_saturating_add(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 3) return;
    const auto& dest_op = ir_instr.operands[0];
    const auto& src1_op = ir_instr.operands[1];
    const auto& src2_op = ir_instr.operands[2];

    int32_t src1_val = static_cast<int32_t>(getOperandValue(src1_op, simulator));
    int32_t src2_val = static_cast<int32_t>(getOperandValue(src2_op, simulator));

    int64_t sum = static_cast<int64_t>(src1_val) + static_cast<int64_t>(src2_val);
    int32_t result;
    if (sum > std::numeric_limits<int32_t>::max()) {
        result = std::numeric_limits<int32_t>::max();
    } else if (sum < std::numeric_limits<int32_t>::min()) {
        result = std::numeric_limits<int32_t>::min();
    } else {
        result = static_cast<int32_t>(sum);
    }

    if (const std::string* dest_name = std::get_if<std::string>(&dest_op)) {
        setRegisterValue(*dest_name, result, simulator);
    }
    // Note: This simplified implementation does not set the Q flag.
}

void ALU::handle_ir_saturating_sub(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 3) return;
    const auto& dest_op = ir_instr.operands[0];
    const auto& src1_op = ir_instr.operands[1];
    const auto& src2_op = ir_instr.operands[2];

    int32_t src1_val = static_cast<int32_t>(getOperandValue(src1_op, simulator));
    int32_t src2_val = static_cast<int32_t>(getOperandValue(src2_op, simulator));

    int64_t diff = static_cast<int64_t>(src1_val) - static_cast<int64_t>(src2_val);
    int32_t result;
    if (diff > std::numeric_limits<int32_t>::max()) {
        result = std::numeric_limits<int32_t>::max();
    } else if (diff < std::numeric_limits<int32_t>::min()) {
        result = std::numeric_limits<int32_t>::min();
    } else {
        result = static_cast<int32_t>(diff);
    }

    if (const std::string* dest_name = std::get_if<std::string>(&dest_op)) {
        setRegisterValue(*dest_name, result, simulator);
    }
    // Note: This simplified implementation does not set the Q flag.
}

void ALU::handle_ir_saturating_double_add(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 3) return;
    const auto& dest_op = ir_instr.operands[0];
    const auto& src1_op = ir_instr.operands[1];
    const auto& src2_op = ir_instr.operands[2];

    int32_t src1_val = static_cast<int32_t>(getOperandValue(src1_op, simulator));
    int32_t src2_val = static_cast<int32_t>(getOperandValue(src2_op, simulator));

    int64_t sum = static_cast<int64_t>(src2_val) + static_cast<int64_t>(src2_val);
    int32_t doubled_src2;
    if (sum > std::numeric_limits<int32_t>::max()) {
        doubled_src2 = std::numeric_limits<int32_t>::max();
    } else if (sum < std::numeric_limits<int32_t>::min()) {
        doubled_src2 = std::numeric_limits<int32_t>::min();
    } else {
        doubled_src2 = static_cast<int32_t>(sum);
    }

    sum = static_cast<int64_t>(src1_val) + static_cast<int64_t>(doubled_src2);
    int32_t result;
    if (sum > std::numeric_limits<int32_t>::max()) {
        result = std::numeric_limits<int32_t>::max();
    } else if (sum < std::numeric_limits<int32_t>::min()) {
        result = std::numeric_limits<int32_t>::min();
    } else {
        result = static_cast<int32_t>(sum);
    }

    if (const std::string* dest_name = std::get_if<std::string>(&dest_op)) {
        setRegisterValue(*dest_name, result, simulator);
    }
    // Note: This simplified implementation does not set the Q flag.
}

void ALU::handle_ir_saturating_double_sub(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 3) return;
    const auto& dest_op = ir_instr.operands[0];
    const auto& src1_op = ir_instr.operands[1];
    const auto& src2_op = ir_instr.operands[2];

    int32_t src1_val = static_cast<int32_t>(getOperandValue(src1_op, simulator));
    int32_t src2_val = static_cast<int32_t>(getOperandValue(src2_op, simulator));

    int64_t sum = static_cast<int64_t>(src2_val) + static_cast<int64_t>(src2_val);
    int32_t doubled_src2;
    if (sum > std::numeric_limits<int32_t>::max()) {
        doubled_src2 = std::numeric_limits<int32_t>::max();
    } else if (sum < std::numeric_limits<int32_t>::min()) {
        doubled_src2 = std::numeric_limits<int32_t>::min();
    } else {
        doubled_src2 = static_cast<int32_t>(sum);
    }

    int64_t diff = static_cast<int64_t>(src1_val) - static_cast<int64_t>(doubled_src2);
    int32_t result;
    if (diff > std::numeric_limits<int32_t>::max()) {
        result = std::numeric_limits<int32_t>::max();
    } else if (diff < std::numeric_limits<int32_t>::min()) {
        result = std::numeric_limits<int32_t>::min();
    } else {
        result = static_cast<int32_t>(diff);
    }

    if (const std::string* dest_name = std::get_if<std::string>(&dest_op)) {
        setRegisterValue(*dest_name, result, simulator);
    }
    // Note: This simplified implementation does not set the Q flag.
}

void ALU::handle_ir_multiply_accumulate(const IRInstruction& ir_instr, ISimulator& simulator) {
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

void ALU::handle_ir_multiply_subtract(const IRInstruction& ir_instr, ISimulator& simulator) {
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

void ALU::handle_ir_unsigned_multiply_long(const IRInstruction& ir_instr, ISimulator& simulator) {
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

void ALU::handle_ir_signed_multiply_long(const IRInstruction& ir_instr, ISimulator& simulator) {
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

void ALU::handle_ir_unsigned_multiply_accumulate_long(const IRInstruction& ir_instr, ISimulator& simulator) {
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

void ALU::handle_ir_signed_multiply_accumulate_long(const IRInstruction& ir_instr, ISimulator& simulator) {
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

void ALU::handle_ir_compare_and_branch_if_not_zero(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 2) {
        return;
    }

    const auto& src_op = ir_instr.operands[0];
    const auto& target_op = ir_instr.operands[1];

    int64_t src_val = getOperandValue(src_op, simulator);

    if (src_val != 0) {
        uint64_t target_address = getOperandValue(target_op, simulator);
        simulator.getRegisterMap().set32(simulator.get_instruction_pointer_name(), target_address);
    }
}

void ALU::handle_ir_move(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 2) {
        return;
    }

    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];

    if (!std::holds_alternative<std::string>(dest_op)) {
        return;
    }

    const auto& dest_name = std::get<std::string>(dest_op);
    uint64_t src_val = getOperandValue(src_op, simulator);

    setRegisterValue(dest_name, src_val, simulator);
}

void ALU::handle_ir_load(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 2) {
        return;
    }

    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];

    if (!std::holds_alternative<std::string>(dest_op)) {
        return;
    }

    const auto& dest_name = std::get<std::string>(dest_op);
    uint64_t src_val = getOperandValue(src_op, simulator);

    setRegisterValue(dest_name, src_val, simulator);
}

void ALU::handle_ir_store(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 2) {
        return;
    }

    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];

    uint64_t src_val = getOperandValue(src_op, simulator);
    setMemoryValue(std::get<IRMemoryOperand>(dest_op), src_val, simulator);
}

void ALU::handle_ir_push(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 1) {
        return;
    }

    const auto& src_op = ir_instr.operands[0];
    uint64_t src_val = getOperandValue(src_op, simulator);

    auto& regs = simulator.getRegisterMap();
    uint64_t rsp = regs.get64("rsp");
    rsp -= 4;
    regs.set64("rsp", rsp);
    simulator.getMemory().write_dword(rsp, src_val);
}

void ALU::handle_ir_pop(const IRInstruction& ir_instr, ISimulator& simulator) {
    if (ir_instr.operands.size() != 1) {
        return;
    }

    const auto& dest_op = ir_instr.operands[0];
    if (!std::holds_alternative<std::string>(dest_op)) {
        return;
    }

    auto& regs = simulator.getRegisterMap();
    uint64_t rsp = regs.get64("rsp");
    uint32_t val = simulator.getMemory().read_dword(rsp);
    rsp += 4;
    regs.set64("rsp", rsp);

    const auto& dest_name = std::get<std::string>(dest_op);
    setRegisterValue(dest_name, val, simulator);
}

void ALU::handle_ir_ret(const IRInstruction& ir_instr, ISimulator& simulator) {
    auto& regs = simulator.getRegisterMap();
    uint64_t rsp = regs.get64("rsp");
    uint64_t return_addr = simulator.getMemory().read_qword(rsp);
    rsp += 8;
    regs.set64("rsp", rsp);
    regs.set64(simulator.get_instruction_pointer_name(), return_addr);
}