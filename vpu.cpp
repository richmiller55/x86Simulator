#include "vpu.h"
#include "i_simulator.h"
#include "execution_helpers.h"
#include "i_database_manager.h"
#include "architecture.h"
#include "i_register_map.h"
#include "avx_core.h"
#include "register_map.h"
#include "x86_simulator.h"
#include "memory.h"
#include <stdexcept>

namespace {

m256i_t get_vector_operand(const IROperand& op, ISimulator& simulator) {
    X86Simulator& x86_sim = static_cast<X86Simulator&>(simulator);
    RegisterMap& regs = static_cast<RegisterMap&>(x86_sim.getRegisterMap());

    if (const std::string* reg_name = std::get_if<std::string>(&op)) {
        return regs.getYmm(*reg_name);
    } else if (const IRMemoryOperand* mem_op = std::get_if<IRMemoryOperand>(&op)) {
        auto& mem = x86_sim.getMemory();
        address_t addr = mem_op->displacement;
        if (mem_op->base_reg) {
            addr += regs.get64(*mem_op->base_reg);
        }
        if (mem_op->index_reg) {
            uint64_t index_val = regs.get64(*mem_op->index_reg);
            addr += index_val * mem_op->scale;
        }
        return mem.read_ymm(addr);
    }
    throw std::runtime_error("Unsupported vector operand type");
}

void set_vector_operand(const IROperand& op, m256i_t value, ISimulator& simulator) {
    X86Simulator& x86_sim = static_cast<X86Simulator&>(simulator);
    RegisterMap& regs = static_cast<RegisterMap&>(x86_sim.getRegisterMap());

    if (const std::string* reg_name = std::get_if<std::string>(&op)) {
        regs.setYmm(*reg_name, value);
    } else if (const IRMemoryOperand* mem_op = std::get_if<IRMemoryOperand>(&op)) {
        auto& mem = x86_sim.getMemory();
        address_t addr = mem_op->displacement;
        if (mem_op->base_reg) {
            addr += regs.get64(*mem_op->base_reg);
        }
        if (mem_op->index_reg) {
            uint64_t index_val = regs.get64(*mem_op->index_reg);
            addr += index_val * mem_op->scale;
        }
        mem.write_ymm(addr, value);
    } else {
        throw std::runtime_error("Unsupported vector operand type for write");
    }
}

template<typename Func>
void handle_binary_packed_op(const IRInstruction& ir_instr, ISimulator& simulator, Func op_func) {
    const auto& dest_op = ir_instr.operands[0];
    m256i_t src1_val, src2_val;

    if (ir_instr.operands.size() == 3) { // 3-operand VEX form (dest, src1, src2)
        src1_val = get_vector_operand(ir_instr.operands[1], simulator);
        src2_val = get_vector_operand(ir_instr.operands[2], simulator);
    } else { // 2-operand legacy/VEX form (dest, src)
        src1_val = get_vector_operand(ir_instr.operands[0], simulator); // dest is also src1
        src2_val = get_vector_operand(ir_instr.operands[1], simulator);
    }

    m256i_t result = op_func(src1_val, src2_val);
    set_vector_operand(dest_op, result, simulator);
}

template<typename Func>
void handle_unary_packed_op(const IRInstruction& ir_instr, ISimulator& simulator, Func op_func) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    
    m256i_t src_val = get_vector_operand(src_op, simulator);
    m256i_t result = op_func(src_val);
    set_vector_operand(dest_op, result, simulator);
}

} // anonymous namespace

void VPU::execute(PipelinedInstruction& instruction, ISimulator& simulator) {
    switch (instruction.ir_instruction.opcode) {
        case IROpcode::PackedAddPS:         handle_ir_packed_add_ps(instruction.ir_instruction, simulator); break;
        case IROpcode::PackedSubPS:         handle_ir_packed_sub_ps(instruction.ir_instruction, simulator); break;
        case IROpcode::PackedMulPS:         handle_ir_packed_mul_ps(instruction.ir_instruction, simulator); break;
        case IROpcode::PackedDivPS:         handle_ir_packed_div_ps(instruction.ir_instruction, simulator); break;
        case IROpcode::PackedSqrtPS:        handle_ir_packed_sqrt_ps(instruction.ir_instruction, simulator); break;
        case IROpcode::PackedMaxPS:         handle_ir_packed_max_ps(instruction.ir_instruction, simulator); break;
        case IROpcode::PackedMinPS:         handle_ir_packed_min_ps(instruction.ir_instruction, simulator); break;
        case IROpcode::PackedReciprocalPS:  handle_ir_packed_reciprocal_ps(instruction.ir_instruction, simulator); break;
        case IROpcode::PackedAnd:           handle_ir_packed_and(instruction.ir_instruction, simulator); break;
        case IROpcode::PackedAndNot:        handle_ir_packed_and_not(instruction.ir_instruction, simulator); break;
        case IROpcode::PackedOr:            handle_ir_packed_or(instruction.ir_instruction, simulator); break;
        case IROpcode::PackedXor:           handle_ir_packed_xor(instruction.ir_instruction, simulator); break;
        case IROpcode::PackedMulLowI16:     handle_ir_packed_mul_low_i16(instruction.ir_instruction, simulator); break;
        case IROpcode::VectorMove:          handle_ir_vector_move(instruction.ir_instruction, simulator); break;
        default:
            break;
    }
}

bool VPU::is_busy() const { return false; }
int VPU::latency() const { return 5; } // Example latency

void VPU::handle_ir_packed_add_ps(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, _mm256_add_ps_sim);
}

void VPU::handle_ir_packed_sub_ps(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, _mm256_sub_ps_sim);
}

void VPU::handle_ir_packed_mul_ps(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, _mm256_mul_ps_sim);
}

void VPU::handle_ir_packed_div_ps(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, _mm256_div_ps_sim);
}

void VPU::handle_ir_packed_max_ps(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, _mm256_max_ps_sim);
}

void VPU::handle_ir_packed_min_ps(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, _mm256_min_ps_sim);
}

void VPU::handle_ir_packed_sqrt_ps(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_unary_packed_op(ir_instr, simulator, _mm256_sqrt_ps_sim);
}

void VPU::handle_ir_packed_reciprocal_ps(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_unary_packed_op(ir_instr, simulator, _mm256_rcp_ps_sim);
}

void VPU::handle_ir_packed_and(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, [](m256i_t a, m256i_t b) {
        m256i_t res;
        res.m128[0] = _mm_and_si128_sim(a.m128[0], b.m128[0]);
        res.m128[1] = _mm_and_si128_sim(a.m128[1], b.m128[1]);
        return res;
    });
}

void VPU::handle_ir_packed_and_not(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, [](m256i_t a, m256i_t b) {
        m256i_t res;
        res.m128[0] = _mm_andnot_si128_sim(a.m128[0], b.m128[0]);
        res.m128[1] = _mm_andnot_si128_sim(a.m128[1], b.m128[1]);
        return res;
    });
}

void VPU::handle_ir_packed_or(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, [](m256i_t a, m256i_t b) {
        m256i_t res;
        res.m128[0] = _mm_or_si128_sim(a.m128[0], b.m128[0]);
        res.m128[1] = _mm_or_si128_sim(a.m128[1], b.m128[1]);
        return res;
    });
}

void VPU::handle_ir_packed_xor(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, [](m256i_t a, m256i_t b) {
        m256i_t res;
        res.m128[0] = _mm_xor_si128_sim(a.m128[0], b.m128[0]);
        res.m128[1] = _mm_xor_si128_sim(a.m128[1], b.m128[1]);
        return res;
    });
}

void VPU::handle_ir_packed_mul_low_i16(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, [](m256i_t a, m256i_t b) {
        m256i_t res;
        res.m128[0] = _mm_mullo_epi16_sim(a.m128[0], b.m128[0]);
        res.m128[1] = _mm_mullo_epi16_sim(a.m128[1], b.m128[1]);
        return res;
    });
}

void VPU::handle_ir_vector_move(const IRInstruction& ir_instr, ISimulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    m256i_t src_val = get_vector_operand(src_op, simulator);
    set_vector_operand(dest_op, src_val, simulator);
}
