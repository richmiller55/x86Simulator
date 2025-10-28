#include "execute_stage.h"
#include "pipeline.h"
#include "i_simulator.h"
#include "alu.h"
#include "fpu.h"
#include "vpu.h"
#include "pipelined_instruction.h"

ExecuteStage::ExecuteStage() {
    functional_units_[FunctionalUnitType::ALU] = std::make_unique<ALU>();
    functional_units_[FunctionalUnitType::FPU] = std::make_unique<FPU>();
    functional_units_[FunctionalUnitType::VPU] = std::make_unique<VPU>();
}

void ExecuteStage::run(Pipeline& pipeline, ISimulator& simulator) {
    const auto& id_ex_latch = pipeline.get_id_ex_latch();
    if (id_ex_latch.read().has_value()) {
        auto instr = id_ex_latch.read().value();
        if (instr.state == InstructionState::Decoded) {
            instr.state = InstructionState::Executing;
            FunctionalUnitType unit_type = instr.ir_instruction.functional_unit_type;
            IFunctionalUnit* unit = functional_units_[unit_type].get();

            if (!unit->is_busy()) {
                unit->execute(instr, simulator);
                instr.state = InstructionState::Memory;
                pipeline.ex_mem_latch_.write(instr);
            } else {
                // Stall the pipeline by inserting a bubble
                pipeline.ex_mem_latch_.write(PipelinedInstruction::create_bubble());
            }
        }
    }
}
