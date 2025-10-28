#include "pipeline.h"
#include "i_simulator.h"
#include "i_stage.h"
#include "fetch_stage.h"
#include "decode_stage.h"
#include "execute_stage.h"
#include "memory_stage.h"
#include "write_back_stage.h"

FetchStage& Pipeline::get_fetch_stage() {
    return static_cast<FetchStage&>(*stages_[0]);
}

Pipeline::Pipeline(ISimulator& simulator) : simulator_(simulator), scoreboard_(score_counter_) {
    stages_.push_back(std::make_unique<FetchStage>());
    stages_.push_back(std::make_unique<DecodeStage>());
    stages_.push_back(std::make_unique<ExecuteStage>());
    stages_.push_back(std::make_unique<MemoryStage>());
    stages_.push_back(std::make_unique<WriteBackStage>());
}

Pipeline::~Pipeline() = default;

void Pipeline::cycle() {
    for (auto& stage : stages_) {
        stage->run(*this, simulator_);
    }

    if_id_latch_.commit();
    id_ex_latch_.commit();
    ex_mem_latch_.commit();
    mem_wb_latch_.commit();
}

void Pipeline::flush() {
    if_id_latch_.flush();
    id_ex_latch_.flush();
    ex_mem_latch_.flush();
    mem_wb_latch_.flush();
}

void Pipeline::retire_instruction(const PipelinedInstruction& instr) {
    retired_instructions_.push_back(instr);
}

std::deque<PipelinedInstruction> Pipeline::get_pipeline_state() const {
    std::deque<PipelinedInstruction> state;
    if (if_id_latch_.read().has_value()) {
        state.push_back(if_id_latch_.read().value());
    }
    if (id_ex_latch_.read().has_value()) {
        state.push_back(id_ex_latch_.read().value());
    }
    if (ex_mem_latch_.read().has_value()) {
        state.push_back(ex_mem_latch_.read().value());
    }
    if (mem_wb_latch_.read().has_value()) {
        state.push_back(mem_wb_latch_.read().value());
    }
    return state;
}
