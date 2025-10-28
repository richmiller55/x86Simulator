#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include <map>
#include <string>
#include <vector>
#include "pipelined_instruction.h"
#include "i_functional_unit.h"
#include "score_counter.h"

// Tracks the result of a hazard check when trying to issue an instruction.
enum class HazardType {
    None,
    RAW, // Read-After-Write (Data dependency)
    WAW, // Write-After-Write (Output dependency)
    WAR, // Write-After-Read (Anti-dependency)
    Structural // A required functional unit is busy
};

class Scoreboard {
public:
    Scoreboard(ScoreCounter& score_counter);

    // Checks for hazards and reserves resources for an instruction.
    HazardType try_issue(const PipelinedInstruction& instruction);

    // Marks an instruction as complete, freeing its resources.
    void mark_complete(const PipelinedInstruction& instruction);

    // Notifies the scoreboard that a cycle has passed.
    void cycle();

private:
    ScoreCounter& score_counter_;
    // Tracks which instruction is writing to a register.
    std::map<std::string, PipelinedInstruction*> register_write_status_;

    // Tracks the status of all functional units.
    std::map<FunctionalUnitType, bool> functional_unit_status_;
    std::vector<PipelinedInstruction> in_flight_instructions_;
};

#endif // SCOREBOARD_H
