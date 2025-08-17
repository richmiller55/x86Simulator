// In a header file, e.g., Operand.h

#include "register_enums.h" // Include your register enum

struct ParsedOperand {
    OperandType type;
    uint64_t value; // For immediate values or calculated memory addresses
    RegisterEnum reg;   // For register operands



  // For memory operands (more complex):
    // address_t base_register_value; // if base register is used
    // address_t index_register_value; // if index register is used
    // uint8_t scale_factor; // 1, 2, 4, 8 for index register
    // int32_t displacement; // offset from base + index
    // Note: You might represent memory operands with a separate struct if they become complex
    // For now, let's keep it simple and store the final calculated address in 'value' for memory operands
};
