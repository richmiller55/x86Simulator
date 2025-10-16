#ifndef ARM_TO_IR_H
#define ARM_TO_IR_H

#include "ir.h"
#include "architecture.h"
#include "memory.h" // For address_t
#include <string>
#include <vector>
#include <map>

/**
 * @class ArmToIrConverter
 * @brief Converts ARM assembly or machine code into the architecture-agnostic IR.
 *
 * This class will serve as the "front-end" for the ARM ISA. It uses a two-pass
 * approach to handle labels and forward references in assembly code.
 * 1. First Pass: Scan the entire assembly file to build a symbol table, mapping
 *    all labels to their corresponding memory addresses.
 * 2. Second Pass: Parse each instruction and its operands, using the symbol table
 *    to resolve branch targets, and generate the final IRProgram.
 */
class ArmToIrConverter {
public:
    /**
     * @brief Constructs an ARM-to-IR converter for a specific ARM architecture.
     * @param arm_arch The ARM architecture description.
     */
    ArmToIrConverter(const Architecture& arm_arch);

    /**
     * @brief Converts a block of ARM assembly code into an IRProgram using a two-pass approach.
     *
     * @param arm_assembly A string containing the ARM assembly code.
     * @return An IRProgram, which is a vector of unique_ptrs to IRInstruction.
     */
    IRProgram convert(const std::string& arm_assembly);

private:
    const Architecture& architecture_;
    std::map<std::string, address_t> symbol_table_;
    address_t current_address_;

    /**
     * @brief First pass: Scans assembly to populate the symbol table with label addresses.
     */
    void first_pass(const std::string& arm_assembly);

    /**
     * @brief Second pass: Parses instructions and generates the IR program.
     */
    IRProgram second_pass(const std::string& arm_assembly);

    /**
     * @brief Parses a single line of assembly into an IRInstruction.
     * @return A unique_ptr to an IRInstruction, or nullptr if the line is not an instruction.
     */
    std::unique_ptr<IRInstruction> parse_line(const std::string& line);

    /**
     * @brief Parses a string representation of an operand into an IROperand variant.
     */
    IROperand parse_operand(const std::string& operand_str);

    /**
     * @brief Parses a register name (e.g., "r0", "sp") into an IRRegister struct.
     */
    IRRegister parse_register(const std::string& reg_str);

    /**
     * @brief Parses a memory operand string (e.g., "[r1, #4]") into an IRMemoryOperand.
     */
    IRMemoryOperand parse_memory_operand(const std::string& mem_str);

    // --- Instruction-type specific translation helpers ---

    /**
     * @brief Translates data processing instructions (e.g., ADD, SUB, MOV).
     */
    std::unique_ptr<IRInstruction> translate_data_processing(const std::string& mnemonic, const std::vector<std::string>& operands);

    /**
     * @brief Translates memory access instructions (e.g., LDR, STR).
     */
    std::unique_ptr<IRInstruction> translate_load_store(const std::string& mnemonic, const std::vector<std::string>& operands);

    /**
     * @brief Translates branch instructions (e.g., B, BL, BEQ).
     */
    std::unique_ptr<IRInstruction> translate_branch(const std::string& mnemonic, const std::vector<std::string>& operands);
};

#endif // ARM_TO_IR_H
