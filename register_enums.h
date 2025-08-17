// RegisterEnums.h

#ifndef X86SIMULATOR_REGISTER_ENUMS_H
#define X86SIMULATOR_REGISTER_ENUMS_H

#include <string>
#include <map>

// Using an enum class for stronger type safety
enum class RegisterEnum {
    UNKNOWN_REG = 0, // Should always be the first entry and indicate an error

    // 32-bit General Purpose Registers (GPRs)
    EAX,
    EBX,
    ECX,
    EDX,
    ESI,
    EDI,
    EBP,
    ESP,

    // 64-bit General Purpose Registers (GPRs)
    RAX,
    RBX,
    RCX,
    RDX,
    RSI,
    RDI,
    RBP,
    RSP,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,

    // Special Purpose Registers
    RIP,    // Instruction Pointer
    EFLAGS, // 32-bit Flags Register
    RFLAGS, // 64-bit Flags Register

    // Segment Registers
    CS, // Code Segment
    DS, // Data Segment
    ES, // Extra Segment
    FS, // F Segment
    GS, // G Segment
    SS  // Stack Segment
};

// Function to convert string representation of a register to its enum value
RegisterEnum stringToRegister(const std::string& reg_str);
#endif // X86SIMULATOR_REGISTER_ENUMS_H
