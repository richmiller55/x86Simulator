#ifndef PIPELINED_INSTRUCTION_H
#define PIPELINED_INSTRUCTION_H

#include "ir.h"
#include "instruction_state_enums.h"
#include <cstdint>

class PipelinedInstruction {
public:
    PipelinedInstruction() : state(InstructionState::Bubble), unique_id_(next_id_++) {}
    PipelinedInstruction(const IRInstruction& ir, InstructionState s)
        : ir_instruction(ir), state(s), unique_id_(next_id_++) {}

    static PipelinedInstruction create_bubble() {
        return PipelinedInstruction();
    }

    IRInstruction ir_instruction;
    InstructionState state;
    uint64_t unique_id_;
    int64_t execution_result; // To hold the result after the execute stage

private:
    static uint64_t next_id_;
};

#endif // PIPELINED_INSTRUCTION_H
