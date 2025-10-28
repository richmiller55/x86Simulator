#ifndef IFUNCTIONALUNIT_H
#define IFUNCTIONALUNIT_H


#include "Instruction.h"

// IFunctionalUnit.h
class IFunctionalUnit {
public:
    virtual ~IFunctionalUnit() = default;
    virtual void execute(Instruction& instruction) = 0;
    virtual int getLatency() const = 0; // The latency for scoreboarding
};

#endif // IFUNCTIONALUNIT_H
