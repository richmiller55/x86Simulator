#ifndef ALU_H
#define ALU_H

#include "i_functional_unit.h"

class ALU : public IFunctionalUnit {
public:
    void execute(PipelinedInstruction& instruction, ISimulator& simulator) override;
    bool is_busy() const override;
    int latency() const override;

private:
    static void handle_ir_add(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_sub(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_addc(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_subc(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_mul(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_imul(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_div(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_and(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_or(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_xor(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_not(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_shl(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_shr(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_sar(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_inc(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_dec(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_cmp(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_tst(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_teq(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_cmn(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_movenot(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_andnot(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_swap(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_move_to_system_register(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_move_from_system_register(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_count_leading_zeros(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_reverse_bits(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_reverse_bytes(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_reverse_bytes16(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_reverse_bytes_signed_halfword(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_saturating_add(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_saturating_sub(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_saturating_double_add(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_saturating_double_sub(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_multiply_accumulate(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_multiply_subtract(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_unsigned_multiply_long(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_signed_multiply_long(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_unsigned_multiply_accumulate_long(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_signed_multiply_accumulate_long(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_compare_and_branch_if_not_zero(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_move(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_load(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_store(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_push(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_pop(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_ret(const IRInstruction& ir_instr, ISimulator& simulator);
};

#endif // ALU_H