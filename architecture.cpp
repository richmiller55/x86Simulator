#include "architecture.h"
#include <iostream>

bool Architecture::is_register(const std::string& name) const {
    for (const auto& [type, file_def] : register_files) {
        for (const auto& phys_reg : file_def.registers) {
            for (const auto& alias : phys_reg.aliases) {
                if (alias.name == name) {
                    return true;
                }
            }
        }
    }
    return false;
}

uint32_t Architecture::get_register_size_bits(const std::string& name) const {
    for (const auto& [type, file_def] : register_files) {
        for (const auto& phys_reg : file_def.registers) {
            for (const auto& alias : phys_reg.aliases) {
                if (alias.name == name) {
                    return alias.size_bits;
                }
            }
        }
    }
    return 0; // Indicates not found
}

Architecture create_x86_architecture() {
    std::cout << "Creating x86 architecture..." << std::endl;
    Architecture arch;
    arch.isa = ISA::X86;
    arch.pointer_size_bits = 64;

    // --- General Purpose Registers ---
    RegisterFileDef gpr_file;
    gpr_file.type = IRRegisterType::GPR;
    gpr_file.registers.resize(16); // 16 GPRs in x86-64
    gpr_file.registers[0] = {"GPR0", 64, {{"rax", 64, 0}, {"eax", 32, 0}, {"ax", 16, 0}, {"al", 8, 0}, {"ah", 8, 8}}};
    gpr_file.registers[1] = {"GPR1", 64, {{"rcx", 64, 0}, {"ecx", 32, 0}, {"cx", 16, 0}, {"cl", 8, 0}, {"ch", 8, 8}}};
    gpr_file.registers[2] = {"GPR2", 64, {{"rdx", 64, 0}, {"edx", 32, 0}, {"dx", 16, 0}, {"dl", 8, 0}, {"dh", 8, 8}}};
    gpr_file.registers[3] = {"GPR3", 64, {{"rbx", 64, 0}, {"ebx", 32, 0}, {"bx", 16, 0}, {"bl", 8, 0}, {"bh", 8, 8}}};
    gpr_file.registers[4] = {"GPR4", 64, {{"rsp", 64, 0}, {"esp", 32, 0}, {"sp", 16, 0}, {"spl", 8, 0}}};
    gpr_file.registers[5] = {"GPR5", 64, {{"rbp", 64, 0}, {"ebp", 32, 0}, {"bp", 16, 0}, {"bpl", 8, 0}}};
    gpr_file.registers[6] = {"GPR6", 64, {{"rsi", 64, 0}, {"esi", 32, 0}, {"si", 16, 0}, {"sil", 8, 0}}};
    gpr_file.registers[7] = {"GPR7", 64, {{"rdi", 64, 0}, {"edi", 32, 0}, {"di", 16, 0}, {"dil", 8, 0}}};
    // R8-R15
    for (int i = 8; i < 16; ++i) {
        std::string r_64 = "r" + std::to_string(i);
        std::string r_32 = r_64 + "d";
        std::string r_16 = r_64 + "w";
        std::string r_8 = r_64 + "b";
        gpr_file.registers[i] = {"GPR" + std::to_string(i), 64, {{r_64, 64, 0}, {r_32, 32, 0}, {r_16, 16, 0}, {r_8, 8, 0}}};
    }
    arch.register_files[IRRegisterType::GPR] = gpr_file;

    // --- Instruction Pointer ---
    RegisterFileDef ip_file;
    ip_file.type = IRRegisterType::IP;
    ip_file.registers.resize(1);
    ip_file.registers[0] = {"IP", 64, {{"rip", 64, 0}, {"eip", 32, 0}, {"ip", 16, 0}}};
    arch.register_files[IRRegisterType::IP] = ip_file;

    // --- Vector Registers ---
    RegisterFileDef vector_file;
    vector_file.type = IRRegisterType::VECTOR;
    vector_file.registers.resize(16); // YMM0-YMM15
    for (int i = 0; i < 16; ++i) {
        vector_file.registers[i] = {"VEC" + std::to_string(i), 256, {{"ymm" + std::to_string(i), 256, 0}, {"xmm" + std::to_string(i), 128, 0}}};
    }
    arch.register_files[IRRegisterType::VECTOR] = vector_file;

    // --- Flags Register ---
    RegisterFileDef flags_file;
    flags_file.type = IRRegisterType::FLAGS;
    flags_file.registers.resize(1);
    flags_file.registers[0] = {"FLAGS", 64, {{"rflags", 64, 0}, {"eflags", 32, 0}, {"flags", 16, 0}}};
    arch.register_files[IRRegisterType::FLAGS] = flags_file;

    return arch;
}

Architecture create_arm_cortex_r8_architecture() {
    Architecture arch;
    arch.isa = ISA::ARM;
    arch.pointer_size_bits = 32;

    // --- General Purpose Registers ---
    RegisterFileDef gpr_file;
    gpr_file.type = IRRegisterType::GPR;
    gpr_file.registers.resize(16);
    for (int i = 0; i <= 12; ++i) {
        gpr_file.registers[i] = {"GPR" + std::to_string(i), 32, {{"r" + std::to_string(i), 32, 0}}};
    }
    gpr_file.registers[13] = {"GPR13", 32, {{"sp", 32, 0}}};
    gpr_file.registers[14] = {"GPR14", 32, {{"lr", 32, 0}}};
    gpr_file.registers[15] = {"GPR15", 32, {{"pc", 32, 0}}}; // PC is also a GPR in ARM
    arch.register_files[IRRegisterType::GPR] = gpr_file;



    // --- Status Register ---
    RegisterFileDef flags_file;
    flags_file.type = IRRegisterType::FLAGS;
    flags_file.registers.resize(1);
    flags_file.registers[0] = {"FLAGS", 32, {{"cpsr", 32, 0}}};
    arch.register_files[IRRegisterType::FLAGS] = flags_file;

    // --- Vector/FPU Registers ---
    RegisterFileDef vector_file;
    vector_file.type = IRRegisterType::VECTOR;
    vector_file.registers.resize(32); // 32 physical 64-bit registers (can be viewed as s, d, or q)
    for (int i = 0; i < 32; ++i) {
        // Each physical register is 64 bits (d register size)
        vector_file.registers[i] = {"VEC" + std::to_string(i), 64, 
            {{"d" + std::to_string(i), 64, 0}}};
        // Add s register aliases
        vector_file.registers[i].aliases.push_back({"s" + std::to_string(i*2), 32, 0});
        vector_file.registers[i].aliases.push_back({"s" + std::to_string(i*2+1), 32, 32});
    }
    // q registers are pairs of d registers
    for (int i = 0; i < 16; ++i) {
        // This is tricky because q registers span two physical d registers.
        // The model might need another level of abstraction for this.
        // For now, we will omit q registers from the new architecture definition.
    }
    arch.register_files[IRRegisterType::VECTOR] = vector_file;

    return arch;
}
