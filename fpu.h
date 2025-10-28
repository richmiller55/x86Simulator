#ifndef FPU_H
#define FPU_H

#include "i_functional_unit.h"

class FPU : public IFunctionalUnit {
public:
    void execute(PipelinedInstruction& instruction, ISimulator& simulator) override;
    bool is_busy() const override;
    int latency() const override;

private:
    static void handle_ir_float_add_s(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_float_sub_s(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_float_mul_s(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_float_div_s(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_float_sqrt_s(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_float_add_d(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_float_sub_d(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_float_mul_d(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_float_div_d(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_float_sqrt_d(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_float_cmp_s(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_float_cmp_d(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_float_to_s(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_float_to_d(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_int_to_float_s(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_int_to_float_d(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_float_to_int_s(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_float_to_int_d(const IRInstruction& ir_instr, ISimulator& simulator);
};

#endif // FPU_H