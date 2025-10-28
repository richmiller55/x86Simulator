#include "fetch_stage.h"
#include "pipeline.h"
#include "i_simulator.h"
#include "program_decoder.h"
#include "register_map.h"
#include "x86_to_ir.h"

void FetchStage::run(Pipeline& pipeline, ISimulator& simulator) {
    if (pipeline.get_if_id_latch().read().has_value()) {
        return; // Latch is full
    }

    auto* decoder = simulator.getProgramDecoder();
    if (!decoder) return;

    address_t rip = simulator.getRegisterMap().get64("rip");

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
        PipelinedInstruction new_instr(*ir_instr, InstructionState::Fetched);
        pipeline.if_id_latch_.write(new_instr);

        // Advance RIP to the next instruction
        simulator.getRegisterMap().set64("rip", rip + decoded_instr.length_in_bytes);
    } else {
        // Could not translate. Skip this instruction to avoid an infinite loop.
        simulator.getRegisterMap().set64("rip", rip + 1);
    }
}

void FetchStage::inject_ir_instruction(const IRInstruction& ir_instr, Pipeline& pipeline) {
    PipelinedInstruction new_instr(ir_instr, InstructionState::Fetched);
    pipeline.if_id_latch_.write(new_instr);
}
