#ifndef IR_VISITOR_H
#define IR_VISITOR_H

#include "ir.h"

class ISimulator;

class IRVisitor {
public:
    virtual ~IRVisitor() = default;
    virtual void visit(const IRInstruction& instr, ISimulator& simulator) = 0;
};

#endif // IR_VISITOR_H
