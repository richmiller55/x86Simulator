#ifndef DECODE_STAGE_H
#define DECODE_STAGE_H

#include "i_stage.h"

class DecodeStage : public IStage {
public:
    void run(Pipeline& pipeline, ISimulator& simulator) override;
};

#endif // DECODE_STAGE_H
