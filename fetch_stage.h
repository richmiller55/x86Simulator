#ifndef FETCH_STAGE_H
#define FETCH_STAGE_H

#include "i_stage.h"
#include "ir.h"

class FetchStage : public IStage {
public:
    void run(Pipeline& pipeline, ISimulator& simulator) override;
    void inject_ir_instruction(const IRInstruction& ir_instr, Pipeline& pipeline);
};

#endif // FETCH_STAGE_H
