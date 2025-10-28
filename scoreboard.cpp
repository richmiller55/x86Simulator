#include "scoreboard.h"
#include <algorithm>

Scoreboard::Scoreboard(ScoreCounter& score_counter)
    : score_counter_(score_counter) {
    // Initialize functional unit status (assuming they are all available at the start)
    functional_unit_status_[FunctionalUnitType::ALU] = true;
    functional_unit_status_[FunctionalUnitType::FPU] = true;
    functional_unit_status_[FunctionalUnitType::VPU] = true;
}

HazardType Scoreboard::try_issue(const PipelinedInstruction& instruction) {
    // Check for structural hazards
    if (!functional_unit_status_[instruction.ir_instruction.functional_unit_type]) {
        score_counter_.record_structural_stall();
        return HazardType::Structural;
    }

    // Check for WAW hazards (Write-After-Write)
    if (instruction.ir_instruction.operands.size() > 0) {
        const auto& dest_op = instruction.ir_instruction.operands.back();
        if (std::holds_alternative<std::string>(dest_op)) {
            if (register_write_status_.count(std::get<std::string>(dest_op))) {
                score_counter_.record_waw_stall();
                return HazardType::WAW;
            }
        }
    }

    // Check for RAW hazards (Read-After-Write)
    for (size_t i = 0; i < instruction.ir_instruction.operands.size() - 1; ++i) {
        const auto& src_op = instruction.ir_instruction.operands[i];
        if (std::holds_alternative<std::string>(src_op)) {
            if (register_write_status_.count(std::get<std::string>(src_op))) {
                score_counter_.record_raw_stall();
                return HazardType::RAW;
            }
        }
    }

    // No hazards, issue the instruction
    functional_unit_status_[instruction.ir_instruction.functional_unit_type] = false;
    if (instruction.ir_instruction.operands.size() > 0) {
        const auto& dest_op = instruction.ir_instruction.operands.back();
        if (std::holds_alternative<std::string>(dest_op)) {
            register_write_status_[std::get<std::string>(dest_op)] = const_cast<PipelinedInstruction*>(&instruction);
        }
    }
    in_flight_instructions_.push_back(instruction);
    score_counter_.record_win();

    return HazardType::None;
}

void Scoreboard::mark_complete(const PipelinedInstruction& instruction) {
    // Free the functional unit
    functional_unit_status_[instruction.ir_instruction.functional_unit_type] = true;

    // Remove the instruction from the in-flight list
    in_flight_instructions_.erase(
        std::remove_if(in_flight_instructions_.begin(), in_flight_instructions_.end(),
                       [&](const PipelinedInstruction& instr) {
                           return &instr == &instruction;
                       }),
        in_flight_instructions_.end());

    // Update register write status
    if (instruction.ir_instruction.operands.size() > 0) {
        const auto& dest_op = instruction.ir_instruction.operands.back();
        if (std::holds_alternative<std::string>(dest_op)) {
            register_write_status_.erase(std::get<std::string>(dest_op));
        }
    }
}

void Scoreboard::cycle() {
    // Stub implementation - could be used for time-based state updates if needed.
}
