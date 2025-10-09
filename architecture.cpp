#include "architecture.h"

/**
 * @brief Populates and returns an Architecture object for the x86 ISA.
 */
Architecture create_x86_architecture() {
    Architecture arch;

    // This map defines the translation from an abstract IRRegister 
    // (type, index, size) to a concrete x86 register name.

    // --- General Purpose Registers (GPRs) ---
    // 64-bit
    arch.register_map[{IRRegisterType::GPR, 0, 64}] = "rax";
    arch.register_map[{IRRegisterType::GPR, 1, 64}] = "rcx";
    arch.register_map[{IRRegisterType::GPR, 2, 64}] = "rdx";
    arch.register_map[{IRRegisterType::GPR, 3, 64}] = "rbx";
    arch.register_map[{IRRegisterType::GPR, 4, 64}] = "rsp";
    arch.register_map[{IRRegisterType::GPR, 5, 64}] = "rbp";
    arch.register_map[{IRRegisterType::GPR, 6, 64}] = "rsi";
    arch.register_map[{IRRegisterType::GPR, 7, 64}] = "rdi";

    // 32-bit
    arch.register_map[{IRRegisterType::GPR, 0, 32}] = "eax";
    arch.register_map[{IRRegisterType::GPR, 1, 32}] = "ecx";
    arch.register_map[{IRRegisterType::GPR, 2, 32}] = "edx";
    arch.register_map[{IRRegisterType::GPR, 3, 32}] = "ebx";
    arch.register_map[{IRRegisterType::GPR, 4, 32}] = "esp";
    arch.register_map[{IRRegisterType::GPR, 5, 32}] = "ebp";
    arch.register_map[{IRRegisterType::GPR, 6, 32}] = "esi";
    arch.register_map[{IRRegisterType::GPR, 7, 32}] = "edi";

    // 16-bit
    arch.register_map[{IRRegisterType::GPR, 0, 16}] = "ax";
    arch.register_map[{IRRegisterType::GPR, 1, 16}] = "cx";
    arch.register_map[{IRRegisterType::GPR, 2, 16}] = "dx";
    arch.register_map[{IRRegisterType::GPR, 3, 16}] = "bx";
    arch.register_map[{IRRegisterType::GPR, 4, 16}] = "sp";
    arch.register_map[{IRRegisterType::GPR, 5, 16}] = "bp";
    arch.register_map[{IRRegisterType::GPR, 6, 16}] = "si";
    arch.register_map[{IRRegisterType::GPR, 7, 16}] = "di";

    // 8-bit (low)
    arch.register_map[{IRRegisterType::GPR, 0, 8}] = "al";
    arch.register_map[{IRRegisterType::GPR, 1, 8}] = "cl";
    arch.register_map[{IRRegisterType::GPR, 2, 8}] = "dl";
    arch.register_map[{IRRegisterType::GPR, 3, 8}] = "bl";

    // --- Instruction Pointer ---
    arch.register_map[{IRRegisterType::IP, 0, 64}] = "rip";
    arch.register_map[{IRRegisterType::IP, 0, 32}] = "eip";
    arch.register_map[{IRRegisterType::IP, 0, 16}] = "ip";

    // --- Vector Registers ---
    // For simplicity, only adding a few here. A full implementation would have all.
    arch.register_map[{IRRegisterType::VECTOR, 0, 256}] = "ymm0";
    arch.register_map[{IRRegisterType::VECTOR, 1, 256}] = "ymm1";
    arch.register_map[{IRRegisterType::VECTOR, 0, 128}] = "xmm0";
    arch.register_map[{IRRegisterType::VECTOR, 1, 128}] = "xmm1";

    return arch;
}
