#ifndef PIPELINE_H
#define PIPELINE_H

#include "ir.h"
#include "instruction_state_enums.h"
#include <deque>
#include <memory>

// Forward declarations
class ISimulator;

/**
 * @brief Represents an instruction as it moves through the pipeline, holding the
 *        IR representation and its current execution state.
 */
struct PipelinedInstruction {
    IRInstruction ir_instruction;
    InstructionState state;
};

/**
 * @brief Manages the instruction pipeline, orchestrating the fetch, decode,
 *        execute, memory, and writeback stages.
 */
class Pipeline {
public:
    explicit Pipeline(ISimulator& simulator);

    /**
     * @brief Advances the entire pipeline by one clock cycle.
     */
    void cycle();

    /**
     * @brief Flushes all stages of the pipeline, typically after a control hazard
     *        like a taken branch.
     */
    void flush();

    /**
     * @brief Returns a constant reference to the current state of the pipeline,
     *        primarily for UI display and debugging.
     * @return A deque of instructions currently in the pipeline.
     */
    const std::deque<PipelinedInstruction>& get_pipeline_state() const;

private:
    void do_write_back_stage();
    void do_memory_access_stage();
    void do_execute_stage();
    void do_decode_stage();
    void do_fetch_stage();

    ISimulator& simulator_;
    std::deque<PipelinedInstruction> pipeline_stages_;
};

#endif // PIPELINE_H
