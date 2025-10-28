#include "memory_stage.h"
#include "pipeline.h"
#include "i_simulator.h"
#include "i_register_map.h"
#include "memory.h"
#include "execution_helpers.h"

void MemoryStage::run(Pipeline& pipeline, ISimulator& simulator) {
    const auto& ex_mem_latch = pipeline.get_ex_mem_latch();
    if (ex_mem_latch.read().has_value()) {
        auto instr = ex_mem_latch.read().value();
        if (instr.state == InstructionState::Memory) {
            if (instr.ir_instruction.opcode == IROpcode::Load) {
                // Handle Load
                const auto& dest_op = instr.ir_instruction.operands[0];
                const auto& src_op = instr.ir_instruction.operands[1];
                uint64_t src_val = getOperandValue(src_op, simulator);
                setRegisterValue(std::get<std::string>(dest_op), src_val, simulator);
            } else if (instr.ir_instruction.opcode == IROpcode::Store) {
                // Handle Store
                const auto& dest_op = instr.ir_instruction.operands[0];
                const auto& src_op = instr.ir_instruction.operands[1];
                uint64_t src_val = getOperandValue(src_op, simulator);
                setMemoryValue(std::get<IRMemoryOperand>(dest_op), src_val, simulator);
            }
            instr.state = InstructionState::Writeback;
            pipeline.mem_wb_latch_.write(instr);
        }
    }
}
