#ifndef X86_TO_IR_H
#define X86_TO_IR_H

#include "decoder.h" // For DecodedInstruction
#include "ir.h"      // For IRInstruction
#include <memory>

/**
 * @brief Translates a decoded x86 instruction into its abstract IR representation.
 *
 * @param decoded_instr The instruction decoded from the original x86 binary.
 * @return A unique_ptr to the new IRInstruction. Returns nullptr if the instruction
 *         is not supported or cannot be translated.
 */
std::unique_ptr<IRInstruction> translate_to_ir(const DecodedInstruction& decoded_instr);

#endif // X86_TO_IR_H
