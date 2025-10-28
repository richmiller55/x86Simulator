#include "write_back_stage.h"
#include "pipeline.h"
#include "i_simulator.h"

void WriteBackStage::run(Pipeline& pipeline, ISimulator& simulator) {
    const auto& mem_wb_latch = pipeline.get_mem_wb_latch();
    if (mem_wb_latch.read().has_value()) {
        auto instr = mem_wb_latch.read().value();
        if (instr.state == InstructionState::Writeback) {
            instr.state = InstructionState::Completed;
            pipeline.retire_instruction(instr);
        }
    }
}
