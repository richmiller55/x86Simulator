#ifndef ARM_TO_IR_H
#define ARM_TO_IR_H

#include "ir.h"
#include "architecture.h"
#include <string>

/**
 * @class ArmToIrConverter
 * @brief Converts ARM assembly or machine code into the architecture-agnostic IR.
 *
 * This class will serve as the "front-end" for the ARM ISA. It will take ARM
 * instructions and translate them into a sequence of IRInstructions that the core
 * simulator engine can execute.
 */
class ArmToIrConverter {
public:
    /**
     * @brief Constructs an ARM-to-IR converter for a specific ARM architecture.
     * @param arm_arch The ARM architecture description.
     */
    ArmToIrConverter(const Architecture& arm_arch);

    /**
     * @brief Converts a block of ARM assembly code into an IRProgram.
     *
     * @param arm_assembly A string containing the ARM assembly code.
     * @return An IRProgram, which is a vector of unique_ptrs to IRInstruction.
     */
    IRProgram convert(const std::string& arm_assembly);

private:
    const Architecture& architecture_;
};

#endif // ARM_TO_IR_H
