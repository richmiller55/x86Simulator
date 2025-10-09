#ifndef IR_EXECUTOR_HELPERS_H
#define IR_EXECUTOR_HELPERS_H

#include "ir.h"
#include "architecture.h"

// Forward declarations to avoid circular dependencies.
// These helpers need access to the simulator's state.
class X86Simulator;
class RegisterMap;
class Memory;

/**
 * @brief Gets the value of an IR operand, resolving registers or memory.
 */
uint64_t getOperandValue(const IROperand& op, X86Simulator& simulator);

/**
 * @brief Sets the value of an abstract IR register.
 */
void setRegisterValue(const IRRegister& reg, uint64_t value, X86Simulator& simulator);

/**
 * @brief Executes an IR 'Add' instruction and updates simulator state.
 */
void handle_ir_add(const IRInstruction& ir_instr, X86Simulator& simulator);

/**
 * @brief Executes an IR 'Sub' instruction and updates simulator state.
 */
void handle_ir_sub(const IRInstruction& ir_instr, X86Simulator& simulator);

/**
 * @brief Executes an IR 'Move' instruction (reg to reg) and updates simulator state.
 */
void handle_ir_move(const IRInstruction& ir_instr, X86Simulator& simulator);

/**
 * @brief Executes an IR 'Load' instruction (mem to reg) and updates simulator state.
 */
void handle_ir_load(const IRInstruction& ir_instr, X86Simulator& simulator);

/**
 * @brief Executes an IR 'Store' instruction (reg to mem) and updates simulator state.
 */
void handle_ir_store(const IRInstruction& ir_instr, X86Simulator& simulator);

/**
 * @brief Executes an IR 'Jump' instruction and updates the instruction pointer.
 */
void handle_ir_jump(const IRInstruction& ir_instr, X86Simulator& simulator);

/**
 * @brief Executes an IR 'Branch' instruction based on a condition.
 */
void handle_ir_branch(const IRInstruction& ir_instr, X86Simulator& simulator);

/**
 * @brief Executes an IR 'Cmp' instruction and updates the status flags.
 */
void handle_ir_cmp(const IRInstruction& ir_instr, X86Simulator& simulator);

/**
 * @brief Executes an IR 'Add' instruction with one operand (inc) and updates status flags.
 */
void handle_ir_inc(const IRInstruction& ir_instr, X86Simulator& simulator);

/**
 * @brief Executes an IR 'Syscall' instruction (like INT).
 */
void handle_ir_syscall(const IRInstruction& ir_instr, X86Simulator& simulator);

/**
 * @brief Executes an IR 'Mul' instruction (unsigned) and updates simulator state.
 */
void handle_ir_mul(const IRInstruction& ir_instr, X86Simulator& simulator);

/**
 * @brief Executes an IR 'IMul' instruction (signed) and updates simulator state.
 */
void handle_ir_imul(const IRInstruction& ir_instr, X86Simulator& simulator);

/**
 * @brief Executes an IR 'Sub' instruction with one operand (dec) and updates status flags.
 */
void handle_ir_dec(const IRInstruction& ir_instr, X86Simulator& simulator);

void handle_ir_call(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_xor(const IRInstruction& ir_instr, X86Simulator& simulator);

void handle_ir_and(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_or(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_not(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_shl(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_shr(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_sar(const IRInstruction& ir_instr, X86Simulator& simulator);

void handle_ir_packed_and(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_packed_and_not(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_packed_or(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_packed_xor(const IRInstruction& ir_instr, X86Simulator& simulator);

void handle_ir_packed_add_ps(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_packed_sub_ps(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_packed_mul_ps(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_packed_div_ps(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_packed_max_ps(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_packed_min_ps(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_packed_sqrt_ps(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_packed_reciprocal_ps(const IRInstruction& ir_instr, X86Simulator& simulator);

void handle_ir_packed_mul_low_i16(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_vector_zero(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_ret(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_div(const IRInstruction& ir_instr, X86Simulator& simulator);

#endif // IR_EXECUTOR_HELPERS_H