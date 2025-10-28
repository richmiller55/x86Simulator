#ifndef PIPELINE_H
#define PIPELINE_H

#include "pipelined_instruction.h"
#include "latch.h"
#include "score_counter.h"
#include "scoreboard.h"
#include <deque>
#include <memory>
#include <vector>

// Forward declarations
class ISimulator;
class IStage;
class FetchStage;
class DecodeStage;
class ExecuteStage;
class MemoryStage;
class WriteBackStage;

/**
 * @brief Manages the instruction pipeline, orchestrating the fetch, decode,
 *        execute, memory, and writeback stages.
 */
class Pipeline {
public:
    explicit Pipeline(ISimulator& simulator);
    ~Pipeline();

    /**
     * @brief Advances the entire pipeline by one clock cycle.
     */
    void cycle();

    /**
     * @brief Flushes all stages of the pipeline, typically after a control hazard
     *        like a taken branch.
     */
    void flush();

    void retire_instruction(const PipelinedInstruction& instr);

    FetchStage& get_fetch_stage();

    const Latch<PipelinedInstruction>& get_if_id_latch() const { return if_id_latch_; }
    const Latch<PipelinedInstruction>& get_id_ex_latch() const { return id_ex_latch_; }
    const Latch<PipelinedInstruction>& get_ex_mem_latch() const { return ex_mem_latch_; }
    const Latch<PipelinedInstruction>& get_mem_wb_latch() const { return mem_wb_latch_; }

    Scoreboard& get_scoreboard() { return scoreboard_; }

    std::deque<PipelinedInstruction> get_pipeline_state() const;

private:
    friend class FetchStage;
    friend class DecodeStage;
    friend class ExecuteStage;
    friend class MemoryStage;
    friend class WriteBackStage;

    ISimulator& simulator_;
    ScoreCounter score_counter_;
    Scoreboard scoreboard_;
    std::vector<std::unique_ptr<IStage>> stages_;
    std::deque<PipelinedInstruction> retired_instructions_;
    
    Latch<PipelinedInstruction> if_id_latch_;
    Latch<PipelinedInstruction> id_ex_latch_;
    Latch<PipelinedInstruction> ex_mem_latch_;
    Latch<PipelinedInstruction> mem_wb_latch_;
};

#endif // PIPELINE_H
