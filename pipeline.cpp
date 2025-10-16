#include "pipeline.h"
#include "i_simulator.h"
#include "ir_executor_helpers.h"
#include "program_decoder.h"
#include "register_map.h"
#include "x86_to_ir.h"

Pipeline::Pipeline(ISimulator& simulator) : simulator_(simulator) {}

void Pipeline::cycle() {
    // Process stages backwards to prevent an instruction from advancing more than one stage per cycle.
    // This simulates the parallel nature of a real pipeline within a sequential process.
    do_write_back_stage();
    do_memory_access_stage();
    do_execute_stage();
    do_decode_stage();
    do_fetch_stage();
}

void Pipeline::flush() {
    // Clear all instructions currently in flight.
    pipeline_stages_.clear();
}

const std::deque<PipelinedInstruction>& Pipeline::get_pipeline_state() const {
    return pipeline_stages_;
}

void Pipeline::do_fetch_stage() {
    if (pipeline_stages_.size() >= 5) {
        return; // Pipeline is full
    }

    auto* decoder = simulator_.getProgramDecoder();
    if (!decoder) return;

    address_t rip = simulator_.getRegisterMap().get64("rip");

    const auto& address_map = decoder->getAddressToIndexMap();
    auto it = address_map.find(rip);
    if (it == address_map.end()) {
        // End of program or invalid RIP
        return;
    }

    const auto& decoded_program = decoder->getDecodedProgram();
    size_t instruction_index = it->second;
    if (instruction_index >= decoded_program.size()) {
        return; // Invalid index
    }

    const auto& decoded_instr = *decoded_program[instruction_index];
    auto ir_instr = translate_to_ir(decoded_instr);

    if (ir_instr) {
        PipelinedInstruction new_instr = {*ir_instr, InstructionState::Fetched};
        pipeline_stages_.push_back(new_instr);

        // Advance RIP to the next instruction
        simulator_.getRegisterMap().set64("rip", rip + decoded_instr.length_in_bytes);
    } else {
        // Could not translate. Skip this instruction to avoid an infinite loop.
        simulator_.getRegisterMap().set64("rip", rip + 1);
    }
}

void Pipeline::do_decode_stage() {
    for (auto& instr : pipeline_stages_) {
        if (instr.state == InstructionState::Fetched) {
            // TODO: Add hazard detection here.
            instr.state = InstructionState::Decoded;
            break; // Process one instruction per stage per cycle
        }
    }
}

void Pipeline::do_execute_stage() {
    for (auto& instr : pipeline_stages_) {
        if (instr.state == InstructionState::Decoded) {
            // Use the existing visitor-based execution logic in the simulator
            simulator_.execute_ir_instruction(instr.ir_instruction);
            instr.state = InstructionState::Executing;
            break;
        }
    }
}

void Pipeline::do_memory_access_stage() {
    for (auto& instr : pipeline_stages_) {
        if (instr.state == InstructionState::Executing) {
            // In our current design, memory access is handled within the execute stage.
            // This stage is a placeholder for a more complex pipeline.
            // We simply transition the state.
            instr.state = InstructionState::Memory;
            break;
        }
    }
}

void Pipeline::do_write_back_stage() {
    if (!pipeline_stages_.empty() && pipeline_stages_.front().state == InstructionState::Memory) {
        // The instruction is finished. In a more complex pipeline, this is where
        // results would be written back to the register file.
        pipeline_stages_.front().state = InstructionState::Completed;
        pipeline_stages_.pop_front();
    }
}
