#ifndef WRITE_BACK_STAGE_H
#define WRITE_BACK_STAGE_H

#include "i_stage.h"

class WriteBackStage : public IStage {
public:
    void run(Pipeline& pipeline, ISimulator& simulator) override;
};

#endif // WRITE_BACK_STAGE_H
