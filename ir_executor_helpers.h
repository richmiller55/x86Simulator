#ifndef IR_EXECUTOR_HELPERS_H
#define IR_EXECUTOR_HELPERS_H

#include "ir.h"

// Forward declarations to avoid circular dependencies.
// These helpers need access to the simulator's state.
class X86Simulator;
class RegisterMap;
class Memory;

/**
 * @brief Gets the value of an IR operand, resolving registers or memory.
 */
uint64_t getOperandValue(const IROperand& op, RegisterMap& regs, Memory& mem);

/**
 * @brief Sets the value of an abstract IR register.
 */
void setRegisterValue(const IRRegister& reg, uint64_t value, RegisterMap& regs);

/**
 * @brief Executes an IR 'Add' instruction and updates simulator state.
 */
void handle_ir_add(const IRInstruction& ir_instr, X86Simulator& simulator);

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

#endif // IR_EXECUTOR_HELPERS_H