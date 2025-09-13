// RegisterEnums.h

#ifndef X86SIMULATOR_REGISTER_ENUMS_H
#define X86SIMULATOR_REGISTER_ENUMS_H

#include <string>
#include <map>

// Using an enum class for stronger type safety
enum class RegAll {
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
// register_enums.h
enum Reg64 {
    RAX, RBX, RCX, RDX, RSI, RDI, RBP, RSP,
    R8, R9, R10, R11, R12, R13, R14, R15,
    RIP, RFLAGS,
    NUM_REG64
};
enum Reg32 {
    EAX, EBX, ECX, EDX, ESI, EDI, EBP, ESP,
    EFLAGS,
    NUM_REG32
};
enum RegSeg {
    CS, DS, ES, FS, GS, SS
};


// Define the display order for each register group.
static const std::vector<std::string> RegisterDisplayOrder64 = {
    "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp", "rsp",
    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
    "rip", "rflags"
};

static const std::vector<std::string> RegisterDisplayOrder32 = {
    "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp", "esp",
    "eflags"
};

static const std::vector<std::string> RegisterDisplayOrderSeg = {
    "cs", "ds", "es", "fs", "gs", "ss"
};

// 64-bit General Purpose Registers (GPRs)
#endif // X86SIMULATOR_REGISTER_ENUMS_H
