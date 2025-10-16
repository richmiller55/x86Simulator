#ifndef INTEL_HELPERS_H
#define INTEL_HELPERS_H

#include "ir.h"
#include "ir_visitor.h"

// Forward declarations
class ISimulator;

class X86IRVisitor : public IRVisitor {
public:
    void visit(const IRInstruction& instr, ISimulator& simulator) override;
};

/**
 * @class ArmIRVisitor
 * @brief Implements the visitor pattern for executing IR instructions on the ARM simulator.
 */
class ArmIRVisitor : public IRVisitor {
public:
    void visit(const IRInstruction& instr, ISimulator& simulator) override;
};

// --- Scalar Handlers ---
void handle_ir_ret(const IRInstruction& ir_instr, ISimulator& simulator);
void handle_ir_div(const IRInstruction& ir_instr, ISimulator& simulator);

// --- Vector Handlers ---
void handle_ir_vector_move(const IRInstruction& ir_instr, ISimulator& simulator);
void handle_ir_vector_zero(const IRInstruction& ir_instr, ISimulator& simulator);
void handle_ir_vector_zero_upper(const IRInstruction& ir_instr, ISimulator& simulator);

// FP
void handle_ir_packed_add_ps(const IRInstruction& ir_instr, ISimulator& simulator);
void handle_ir_packed_sub_ps(const IRInstruction& ir_instr, ISimulator& simulator);
void handle_ir_packed_mul_ps(const IRInstruction& ir_instr, ISimulator& simulator);
void handle_ir_packed_div_ps(const IRInstruction& ir_instr, ISimulator& simulator);
void handle_ir_packed_max_ps(const IRInstruction& ir_instr, ISimulator& simulator);
void handle_ir_packed_min_ps(const IRInstruction& ir_instr, ISimulator& simulator);
void handle_ir_packed_sqrt_ps(const IRInstruction& ir_instr, ISimulator& simulator);
void handle_ir_packed_reciprocal_ps(const IRInstruction& ir_instr, ISimulator& simulator);
void handle_ir_packed_rsqrt_ps(const IRInstruction& ir_instr, ISimulator& simulator);

// Integer
void handle_ir_packed_add_i8(const IRInstruction& ir_instr, ISimulator& simulator);
void handle_ir_packed_add_i16(const IRInstruction& ir_instr, ISimulator& simulator);
void handle_ir_packed_add_i32(const IRInstruction& ir_instr, ISimulator& simulator);
void handle_ir_packed_add_i64(const IRInstruction& ir_instr, ISimulator& simulator);
void handle_ir_packed_sub_i8(const IRInstruction& ir_instr, ISimulator& simulator);
void handle_ir_packed_sub_i16(const IRInstruction& ir_instr, ISimulator& simulator);
void handle_ir_packed_sub_i32(const IRInstruction& ir_instr, ISimulator& simulator);
void handle_ir_packed_sub_i64(const IRInstruction& ir_instr, ISimulator& simulator);
void handle_ir_packed_mul_low_i16(const IRInstruction& ir_instr, ISimulator& simulator);
void handle_ir_packed_mul_low_i32(const IRInstruction& ir_instr, ISimulator& simulator);
void handle_ir_packed_mul_u32(const IRInstruction& ir_instr, ISimulator& simulator);

// Logical
void handle_ir_packed_and(const IRInstruction& ir_instr, ISimulator& simulator);
void handle_ir_packed_and_not(const IRInstruction& ir_instr, ISimulator& simulator);
void handle_ir_packed_or(const IRInstruction& ir_instr, ISimulator& simulator);
void handle_ir_packed_xor(const IRInstruction& ir_instr, ISimulator& simulator);

#endif // INTEL_HELPERS_H