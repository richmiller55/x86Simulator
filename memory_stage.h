#ifndef MEMORY_STAGE_H
#define MEMORY_STAGE_H

#include "i_stage.h"

class MemoryStage : public IStage {
public:
    void run(Pipeline& pipeline, ISimulator& simulator) override;
};

#endif // MEMORY_STAGE_H
