#ifndef INTEL_HELPERS_H
#define INTEL_HELPERS_H

#include "ir.h"
#include "architecture.h"

// Forward declarations to avoid circular dependencies.
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
 * @brief Sets the value of a memory location based on an IRMemoryOperand.
 */
void setMemoryValue(const IRMemoryOperand& mem_op, uint64_t value, X86Simulator& simulator);

// --- IR-based Instruction Handlers ---

void handle_ir_add(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_move(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_load(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_store(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_jump(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_call(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_xor(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_branch(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_cmp(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_inc(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_syscall(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_mul(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_imul(const IRInstruction& ir_instr, X86Simulator& simulator);
void handle_ir_dec(const IRInstruction& ir_instr, X86Simulator& simulator);

#endif // INTEL_HELPERS_H
