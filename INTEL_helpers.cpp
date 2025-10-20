#include "INTEL_helpers.h"
#include "ir.h"
#include "x86_simulator.h"
#include "ir_executor_helpers.h"
#include "avx_core.h"
#include <variant>
#include <string>
#include <cstdint>
#include <immintrin.h>
#include <stdexcept>

// Helper to resolve a vector operand (YMM register or memory)
static m256i_t get_vector_operand(const IROperand& op, ISimulator& simulator) {
    X86Simulator& x86_sim = static_cast<X86Simulator&>(simulator);
    auto& regs = x86_sim.getRegisterMap();
    const auto& arch = x86_sim.get_architecture();

    if (const IRRegister* reg = std::get_if<IRRegister>(&op)) {
        return regs.getYmm(arch.get_register_name(*reg));
    } else if (const IRMemoryOperand* mem_op = std::get_if<IRMemoryOperand>(&op)) {
        auto& mem = x86_sim.getMemory();
        address_t addr = mem_op->displacement;
        if (mem_op->base_reg) {
            addr += regs.get64(arch.get_register_name(*mem_op->base_reg));
        }
        if (mem_op->index_reg) {
            uint64_t index_val = regs.get64(arch.get_register_name(*mem_op->index_reg));
            addr += index_val * mem_op->scale;
        }
        return mem.read_ymm(addr);
    }
    throw std::runtime_error("Unsupported vector operand type");
}

// Helper to write to a vector operand (YMM register or memory)
static void set_vector_operand(const IROperand& op, m256i_t value, ISimulator& simulator) {
    X86Simulator& x86_sim = static_cast<X86Simulator&>(simulator);
    auto& regs = x86_sim.getRegisterMap();
    const auto& arch = x86_sim.get_architecture();

    if (const IRRegister* reg = std::get_if<IRRegister>(&op)) {
        regs.setYmm(arch.get_register_name(*reg), value);
    } else if (const IRMemoryOperand* mem_op = std::get_if<IRMemoryOperand>(&op)) {
        auto& mem = x86_sim.getMemory();
        address_t addr = mem_op->displacement;
        if (mem_op->base_reg) {
            addr += regs.get64(arch.get_register_name(*mem_op->base_reg));
        }
        if (mem_op->index_reg) {
            uint64_t index_val = regs.get64(arch.get_register_name(*mem_op->index_reg));
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

// --- Handlers ---

void handle_ir_packed_add_ps(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, _mm256_add_ps_sim);
}

void handle_ir_packed_sub_ps(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, _mm256_sub_ps_sim);
}

void handle_ir_packed_mul_ps(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, _mm256_mul_ps_sim);
}

void handle_ir_packed_div_ps(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, _mm256_div_ps_sim);
}

void handle_ir_packed_max_ps(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, _mm256_max_ps_sim);
}

void handle_ir_packed_min_ps(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, _mm256_min_ps_sim);
}

void handle_ir_packed_sqrt_ps(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_unary_packed_op(ir_instr, simulator, _mm256_sqrt_ps_sim);
}

void handle_ir_packed_reciprocal_ps(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_unary_packed_op(ir_instr, simulator, _mm256_rcp_ps_sim);
}

void handle_ir_packed_rsqrt_ps(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_unary_packed_op(ir_instr, simulator, _mm256_rsqrt_ps_sim);
}

void handle_ir_packed_add_i8(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, _mm256_add_epi8_sim);
}
void handle_ir_packed_add_i16(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, _mm256_add_epi16_sim);
}
void handle_ir_packed_add_i32(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, _mm256_add_epi32_sim);
}
void handle_ir_packed_add_i64(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, _mm256_add_epi64_sim);
}

void handle_ir_packed_sub_i8(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, _mm256_sub_epi8_sim);
}
void handle_ir_packed_sub_i16(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, _mm256_sub_epi16_sim);
}
void handle_ir_packed_sub_i32(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, _mm256_sub_epi32_sim);
}
void handle_ir_packed_sub_i64(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, _mm256_sub_epi64_sim);
}

void handle_ir_packed_mul_low_i16(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, [](m256i_t a, m256i_t b) {
        m256i_t res;
        res.m128[0] = _mm_mullo_epi16_sim(a.m128[0], b.m128[0]);
        res.m128[1] = _mm_mullo_epi16_sim(a.m128[1], b.m128[1]);
        return res;
    });
}

void handle_ir_packed_mul_low_i32(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, _mm256_mullo_epi32_sim);
}

void handle_ir_packed_mul_u32(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, _mm256_mul_epu32_sim);
}

void handle_ir_packed_and(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, [](m256i_t a, m256i_t b) {
        m256i_t res;
        res.m128[0] = _mm_and_si128_sim(a.m128[0], b.m128[0]);
        res.m128[1] = _mm_and_si128_sim(a.m128[1], b.m128[1]);
        return res;
    });
}

void handle_ir_packed_and_not(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, [](m256i_t a, m256i_t b) {
        m256i_t res;
        res.m128[0] = _mm_andnot_si128_sim(a.m128[0], b.m128[0]);
        res.m128[1] = _mm_andnot_si128_sim(a.m128[1], b.m128[1]);
        return res;
    });
}

void handle_ir_packed_or(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, [](m256i_t a, m256i_t b) {
        m256i_t res;
        res.m128[0] = _mm_or_si128_sim(a.m128[0], b.m128[0]);
        res.m128[1] = _mm_or_si128_sim(a.m128[1], b.m128[1]);
        return res;
    });
}

void handle_ir_packed_xor(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_packed_op(ir_instr, simulator, [](m256i_t a, m256i_t b) {
        m256i_t res;
        res.m128[0] = _mm_xor_si128_sim(a.m128[0], b.m128[0]);
        res.m128[1] = _mm_xor_si128_sim(a.m128[1], b.m128[1]);
        return res;
    });
}

void handle_ir_vector_move(const IRInstruction& ir_instr, ISimulator& simulator) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];
    m256i_t src_val = get_vector_operand(src_op, simulator);
    set_vector_operand(dest_op, src_val, simulator);
}

void handle_ir_vector_zero(const IRInstruction& ir_instr, ISimulator& simulator) {
    X86Simulator& x86_sim = static_cast<X86Simulator&>(simulator);
    auto& regs = x86_sim.getRegisterMap();
    m256i_t zero = _mm256_setzero_si256_sim();
    for (int i = 0; i < 16; ++i) {
        regs.setYmm("ymm" + std::to_string(i), zero);
    }
}

void handle_ir_vector_zero_upper(const IRInstruction& ir_instr, ISimulator& simulator) {
    X86Simulator& x86_sim = static_cast<X86Simulator&>(simulator);
    auto& regs = x86_sim.getRegisterMap();
    for (int i = 0; i < 16; ++i) {
        m256i_t val = regs.getYmm("ymm" + std::to_string(i));
        val.m128[1] = _mm_setzero_si128_sim();
        regs.setYmm("ymm" + std::to_string(i), val);
    }
}



void handle_x86_ir_div(const IRInstruction& ir_instr, ISimulator& simulator) {
    X86Simulator& x86_sim = static_cast<X86Simulator&>(simulator);
    auto& regs = x86_sim.getRegisterMap();
    const auto& src_op = ir_instr.operands[0];

    uint32_t size = 0;
    if (const IRRegister* reg = std::get_if<IRRegister>(&src_op)) {
        size = reg->size;
    } else if (const IRMemoryOperand* mem = std::get_if<IRMemoryOperand>(&src_op)) {
        size = mem->size;
    } else {
        x86_sim.getDatabaseManager().log(x86_sim.get_session_id(), "Invalid source operand for IR Div", "ERROR", 0, __FILE__, __LINE__);
        return;
    }

    auto halt_for_exception = [&]() {
        x86_sim.getDatabaseManager().log(x86_sim.get_session_id(),
					   "Divide Error Exception (#DE)", "ERROR", regs.get64("rip"), __FILE__, __LINE__);
        regs.set64("rip", x86_sim.getMemory().get_total_memory_size());
    };

    switch (size) {
        case 8: {
            int64_t divisor_val = getOperandValue(src_op, simulator);
            uint8_t divisor = static_cast<uint8_t>(divisor_val);
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
            int64_t divisor_val = getOperandValue(src_op, simulator);
            uint16_t divisor = static_cast<uint16_t>(divisor_val);
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
            int64_t divisor_val = getOperandValue(src_op, simulator);
            uint32_t divisor = static_cast<uint32_t>(divisor_val);
            if (divisor == 0) { return halt_for_exception(); }
            uint64_t dividend = (static_cast<uint64_t>(regs.get32("edx")) << 32) | regs.get32("eax");
            uint64_t quotient = dividend / divisor;
            if (quotient > 0xFFFFFFFF) { return halt_for_exception(); } // Check for overflow
            uint32_t remainder = dividend % divisor;
            regs.set32("eax", static_cast<uint32_t>(quotient));
            regs.set32("edx", remainder);
            break;
        }
        case 64: {
            int64_t divisor_val = getOperandValue(src_op, simulator);
            uint64_t divisor = static_cast<uint64_t>(divisor_val);
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
            x86_sim.getDatabaseManager().log(x86_sim.get_session_id(), "Unsupported operand size for IR Div", "ERROR", 0, __FILE__, __LINE__);
            break;
    }
}

void X86IRVisitor::visit(const IRInstruction& instr, ISimulator& simulator) {
    switch (instr.opcode) {
        // NOTE: Scalar handlers are in ir_executor_helpers.cpp
        case IROpcode::Move:            handle_ir_move(instr, simulator); break;
        case IROpcode::Add:             handle_ir_add(instr, simulator); break;
        case IROpcode::Sub:             handle_ir_sub(instr, simulator); break;
        case IROpcode::Cmp:             handle_ir_cmp(instr, simulator); break;
        case IROpcode::Jump:            handle_ir_jump(instr, simulator); break;
        case IROpcode::Branch:          handle_ir_branch(instr, simulator); break;
        case IROpcode::Call:            handle_ir_call(instr, simulator); break;
        case IROpcode::Push:            handle_ir_push(instr, simulator); break;
        case IROpcode::Pop:             handle_ir_pop(instr, simulator); break;
        case IROpcode::Dec:             handle_ir_dec(instr, simulator); break;
        case IROpcode::Inc:             handle_ir_inc(instr, simulator); break;
        case IROpcode::Xor:             handle_ir_xor(instr, simulator); break;
        case IROpcode::Syscall:         handle_ir_syscall(instr, simulator); break;
        case IROpcode::And:             handle_ir_and(instr, simulator); break;
        case IROpcode::Or:              handle_ir_or(instr, simulator); break;
        case IROpcode::Not:             handle_ir_not(instr, simulator); break;
        case IROpcode::Shl:             handle_ir_shl(instr, simulator); break;
        case IROpcode::Shr:             handle_ir_shr(instr, simulator); break;
        case IROpcode::Sar:             handle_ir_sar(instr, simulator); break;
        case IROpcode::In:              handle_ir_in(instr, simulator); break;
        case IROpcode::Out:             handle_ir_out(instr, simulator); break;

        // Handlers in this file
        case IROpcode::Ret:             handle_ir_ret(instr, simulator); break;
        case IROpcode::Div:             handle_x86_ir_div(instr, simulator); break;
        case IROpcode::VectorMove:      handle_ir_vector_move(instr, simulator); break;
        case IROpcode::VectorZero:      handle_ir_vector_zero(instr, simulator); break;
        case IROpcode::VectorZeroUpper: handle_ir_vector_zero_upper(instr, simulator); break;

        case IROpcode::PackedAnd:       handle_ir_packed_and(instr, simulator); break;
        case IROpcode::PackedAndNot:    handle_ir_packed_and_not(instr, simulator); break;
        case IROpcode::PackedOr:        handle_ir_packed_or(instr, simulator); break;
        case IROpcode::PackedXor:       handle_ir_packed_xor(instr, simulator); break;

        case IROpcode::PackedAddPS:     handle_ir_packed_add_ps(instr, simulator); break;
        case IROpcode::PackedSubPS:     handle_ir_packed_sub_ps(instr, simulator); break;
        case IROpcode::PackedMulPS:     handle_ir_packed_mul_ps(instr, simulator); break;
        case IROpcode::PackedDivPS:     handle_ir_packed_div_ps(instr, simulator); break;
        case IROpcode::PackedMaxPS:     handle_ir_packed_max_ps(instr, simulator); break;
        case IROpcode::PackedMinPS:     handle_ir_packed_min_ps(instr, simulator); break;
        case IROpcode::PackedSqrtPS:    handle_ir_packed_sqrt_ps(instr, simulator); break;
        case IROpcode::PackedReciprocalPS: handle_ir_packed_reciprocal_ps(instr, simulator); break;
        case IROpcode::PackedReciprocalSqrtPS: handle_ir_packed_rsqrt_ps(instr, simulator); break;

        case IROpcode::PackedAddI8:     handle_ir_packed_add_i8(instr, simulator); break;
        case IROpcode::PackedAddI16:    handle_ir_packed_add_i16(instr, simulator); break;
        case IROpcode::PackedAddI32:    handle_ir_packed_add_i32(instr, simulator); break;
        case IROpcode::PackedAddI64:    handle_ir_packed_add_i64(instr, simulator); break;
        case IROpcode::PackedSubI8:     handle_ir_packed_sub_i8(instr, simulator); break;
        case IROpcode::PackedSubI16:    handle_ir_packed_sub_i16(instr, simulator); break;
        case IROpcode::PackedSubI32:    handle_ir_packed_sub_i32(instr, simulator); break;
        case IROpcode::PackedSubI64:    handle_ir_packed_sub_i64(instr, simulator); break;

        case IROpcode::PackedMulLowI16: handle_ir_packed_mul_low_i16(instr, simulator); break;
        case IROpcode::PackedMulLowI32: handle_ir_packed_mul_low_i32(instr, simulator); break;
        case IROpcode::PackedMulU32:    handle_ir_packed_mul_u32(instr, simulator); break;

        default: {
            X86Simulator& x86_sim = static_cast<X86Simulator&>(simulator);
            std::string logmessage = "Unsupported IR Opcode in X86IRVisitor: " + std::to_string(static_cast<int>(instr.opcode));
            x86_sim.getDatabaseManager().log(x86_sim.get_session_id(), logmessage, "ERROR", x86_sim.get_instruction_pointer(), __FILE__, __LINE__);
            break;
        }
    }
}