#ifndef ARCHITECTURE_H
#define ARCHITECTURE_H

#include "ir.h"
#include <string>
#include <vector>
#include <map>
#include <cstdint>

// Enum to identify the Instruction Set Architecture
enum class ISA {
    X86,
    ARM
};

// Struct to define a register alias (a named view into a physical register)
struct RegisterAlias {
    std::string name;
    uint32_t size_bits;
    uint32_t offset_bits;
};

// Struct to define a physical register and all its aliases
struct PhysicalRegisterDef {
    std::string name; // e.g., "GPR0"
    uint32_t size_bits;
    std::vector<RegisterAlias> aliases;
};

// Struct to define a complete register file (e.g., GPRs, Vector Regs)
struct RegisterFileDef {
    IRRegisterType type;
    std::vector<PhysicalRegisterDef> registers;
};

/**
 * @brief Describes the properties of a specific ISA, like its register set and endianness.
 */
class Architecture {
public:
    ISA isa;
    uint32_t pointer_size_bits;
    // Endianness endianness; // TODO: Add endianness

    // A map of all the register files in the architecture
    std::map<IRRegisterType, RegisterFileDef> register_files;

    bool is_register(const std::string& name) const;
    uint32_t get_register_size_bits(const std::string& name) const;
};

/**
 * @brief Factory function to create a description for the x86 architecture.
 */
Architecture create_x86_architecture();

/**
 * @brief Factory function to create a description for the ARM Cortex-R8 architecture.
 */
Architecture create_arm_cortex_r8_architecture();

#endif // ARCHITECTURE_H
