#ifndef VPU_H
#define VPU_H

#include "i_functional_unit.h"

class VPU : public IFunctionalUnit {
public:
    void execute(PipelinedInstruction& instruction, ISimulator& simulator) override;
    bool is_busy() const override;
    int latency() const override;

private:
    static void handle_ir_packed_add_ps(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_packed_sub_ps(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_packed_mul_ps(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_packed_div_ps(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_packed_sqrt_ps(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_packed_max_ps(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_packed_min_ps(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_packed_reciprocal_ps(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_packed_and(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_packed_and_not(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_packed_or(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_packed_xor(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_packed_mul_low_i16(const IRInstruction& ir_instr, ISimulator& simulator);
    static void handle_ir_vector_move(const IRInstruction& ir_instr, ISimulator& simulator);
};

#endif // VPU_H