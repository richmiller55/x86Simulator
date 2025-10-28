#include "decode_stage.h"
#include "pipeline.h"
#include "i_simulator.h"

void DecodeStage::run(Pipeline& pipeline, ISimulator& simulator) {
    const auto& if_id_latch = pipeline.get_if_id_latch();
    if (if_id_latch.read().has_value()) {
        auto instr = if_id_latch.read().value();
        if (instr.state == InstructionState::Fetched) {
            HazardType hazard = pipeline.get_scoreboard().try_issue(instr);
            if (hazard == HazardType::None) {
                instr.state = InstructionState::Decoded;
                pipeline.id_ex_latch_.write(instr);
            } else {
                // Stall the instruction and insert a bubble
                pipeline.id_ex_latch_.write(PipelinedInstruction::create_bubble());
            }
        }
    }
}
