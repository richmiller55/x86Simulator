// In simulator.h (or a new instruction_handler.h)
// You might define an enum for instruction types for better readability
enum class InstructionType {
    MOV,
    ADD,
    JMP,
    // ... other instructions
    UNKNOWN
};
