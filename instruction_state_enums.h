#ifndef INSTRUCTION_STATE
#define INSTRUCTION_STATE

enum class InstructionState {
    Waiting,    // In instruction stream, not yet fetched
    Fetched,    // IF: Instruction fetched
    Decoded,    // ID: Instruction decoded
    Executing,  // EX: Executing the operation
    Memory,     // MEM: Memory access (load/store)
    Writeback,  // WB: Result written back to a register
    Completed,  // Finished execution
    Stalled,    // Halted due to a hazard
    Squashed    // Discarded (e.g., due to a mispredicted branch)
};

#endif // INSTRUCTION_STATE
