#ifndef OPERAND_TYPES_DECODER_H
#define OPERAND_TYPES_DECODER_H
// In a header file, e.g., OperandTypes.h
enum class OperandType {
  IMMEDIATE,
    REGISTER,
    XMM_REGISTER,
    YMM_REGISTER,
    MEMORY,
    LABEL,
    UNKNOWN_OPERAND_TYPE
    };
#endif // DECODER_H
