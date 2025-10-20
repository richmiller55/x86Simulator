#ifndef ARCHITECTURE_H
#define ARCHITECTURE_H

#include "ir.h"
#include <string>
#include <map>
#include <stdexcept>

// Enum to identify the Instruction Set Architecture
enum class ISA {
    X86,
    ARM
};

// A helper struct to allow IRRegister to be used as a key in std::map.
struct IRRegisterKey {
    IRRegisterType type;
    uint32_t index;
    uint32_t size;

    bool operator<(const IRRegisterKey& other) const {
        if (type != other.type) return type < other.type;
        if (index != other.index) return index < other.index;
        return size < other.size;
    }
};

/**
 * @brief Describes the properties of a specific ISA, like its register set.
 */
class Architecture {
public:
    ISA isa;
    uint32_t pointer_size_bits;

    // Maps an abstract IRRegister to its concrete ISA-specific name (e.g., "eax").
    std::map<IRRegisterKey, std::string> register_map;

    /**
     * @brief Gets the ISA-specific name for a given abstract register.
     * @throws std::runtime_error if no mapping is found.
     */
    const std::string& get_register_name(const IRRegister& reg) const;

    uint32_t get_pointer_size_bits() const { return pointer_size_bits; }

    // In the future, this class could also hold other ISA-specific details,
    // such as endianness, address size, etc.
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