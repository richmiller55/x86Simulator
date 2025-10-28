#ifndef I_STAGE_H
#define I_STAGE_H

class ISimulator;
class Pipeline;

class IStage {
public:
    virtual ~IStage() = default;
    virtual void run(Pipeline& pipeline, ISimulator& simulator) = 0;
};

#endif // I_STAGE_H
