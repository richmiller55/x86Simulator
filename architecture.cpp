#include "architecture.h"

/**
 * @brief Populates and returns an Architecture object for the x86 ISA.
 */
Architecture create_x86_architecture() {
    Architecture arch;
    arch.isa = ISA::X86;
    arch.pointer_size_bits = 64;

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
    // XMM (128-bit) and YMM (256-bit)
    for (int i = 0; i < 16; ++i) {
        arch.register_map[{IRRegisterType::VECTOR, static_cast<uint32_t>(i), 128}] = "xmm" + std::to_string(i);
        arch.register_map[{IRRegisterType::VECTOR, static_cast<uint32_t>(i), 256}] = "ymm" + std::to_string(i);
    }

    // --- Flags Register ---
    arch.register_map[{IRRegisterType::FLAGS, 0, 64}] = "rflags";
    arch.register_map[{IRRegisterType::FLAGS, 0, 32}] = "eflags";
    arch.register_map[{IRRegisterType::FLAGS, 0, 16}] = "flags";

    return arch;
}

/**
 * @brief Populates and returns an Architecture object for the ARM Cortex-R8 ISA.
 */
Architecture create_arm_cortex_r8_architecture() {
    Architecture arch;
    arch.isa = ISA::ARM;
    arch.pointer_size_bits = 32;

    // --- General Purpose Registers (GPRs) ---
    // All GPRs in Cortex-R8 (AArch32) are 32-bit.
    for (int i = 0; i <= 12; ++i) {
        arch.register_map[{IRRegisterType::GPR, static_cast<uint32_t>(i), 32}] = "r" + std::to_string(i);
    }

    // --- Special Purpose GPRs ---
    arch.register_map[{IRRegisterType::GPR, 13, 32}] = "sp"; // Stack Pointer
    arch.register_map[{IRRegisterType::GPR, 14, 32}] = "lr"; // Link Register

    // --- Program Counter ---
    arch.register_map[{IRRegisterType::IP, 0, 32}] = "pc"; // Program Counter (r15)

    // --- Status Register ---
    arch.register_map[{IRRegisterType::FLAGS, 0, 32}] = "cpsr"; // Current Program Status Register

    // --- Vector/SIMD Registers (NEON) ---
    // Cortex-R8 can have 16 or 32 128-bit registers (d0-d31 or q0-q15)
    // We will map the 128-bit view (q registers)
    for (int i = 0; i < 16; ++i) {
        arch.register_map[{IRRegisterType::VECTOR, static_cast<uint32_t>(i), 128}] = "q" + std::to_string(i);
    }

    return arch;
}

const std::string& Architecture::get_register_name(const IRRegister& reg) const {
    IRRegisterKey key = {reg.type, reg.index, reg.size};
    auto it = register_map.find(key);
    if (it == register_map.end()) {
        throw std::runtime_error("Register mapping not found for the given IRRegister.");
    }
    return it->second;
}