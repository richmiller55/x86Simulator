#ifndef EXECUTE_STAGE_H
#define EXECUTE_STAGE_H

#include "i_stage.h"
#include "i_functional_unit.h"
#include <map>
#include <memory>

class ExecuteStage : public IStage {
public:
    ExecuteStage();
    void run(Pipeline& pipeline, ISimulator& simulator) override;

private:
    std::map<FunctionalUnitType, std::unique_ptr<IFunctionalUnit>> functional_units_;
};

#endif // EXECUTE_STAGE_H
