#ifndef I_FUNCTIONAL_UNIT_H
#define I_FUNCTIONAL_UNIT_H

#include "pipelined_instruction.h"
#include "ir.h"

class ISimulator;

class IFunctionalUnit {
public:
    virtual ~IFunctionalUnit() = default;
    virtual void execute(PipelinedInstruction& instruction, ISimulator& simulator) = 0;
    virtual bool is_busy() const = 0;
    virtual int latency() const = 0;
};

#endif // I_FUNCTIONAL_UNIT_H
