#ifndef EXECUTION_HELPERS_H
#define EXECUTION_HELPERS_H

#include "ir.h"

class ISimulator;

int64_t getOperandValue(const IROperand& op, ISimulator& simulator);
void setRegisterValue(const std::string& reg_name, int64_t value, ISimulator& simulator);
void setMemoryValue(const IRMemoryOperand& mem_op, int64_t value, ISimulator& simulator);

#endif // EXECUTION_HELPERS_H
