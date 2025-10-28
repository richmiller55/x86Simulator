#include "gtest/gtest.h"
#include "../fpu.h"
#include "../x86_simulator.h"
#include "../memory.h"
#include "mock_database_manager.h"

class FPUTest : public ::testing::Test {
protected:
    std::unique_ptr<FPU> fpu;
    MockDatabaseManager dbManager;
    Memory memory;
    X86Simulator simulator;

    FPUTest() : fpu(std::make_unique<FPU>()), simulator(dbManager, memory, 1, true) {}

    void SetUp() override {
        // Set up any necessary objects or state
    }

    void TearDown() override {
        // Clean up
    }
};

TEST_F(FPUTest, FaddExecution) {
    // FADD st(0), st(1)
    // Initialize st(0) and st(1) registers
    static_cast<RegisterMap&>(simulator.getRegisterMap()).set_st(0, 10.0);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).set_st(1, 20.0);

    // Create an IR instruction
    IRInstruction ir_instr(IROpcode::FloatAddD, { "st(0)", "st(1)" }, FunctionalUnitType::FPU);
    PipelinedInstruction p_instr(ir_instr, InstructionState::Executing);

    // Execute the instruction
    fpu->execute(p_instr, simulator);

    // Verify the result in st(0)
    EXPECT_EQ(static_cast<RegisterMap&>(simulator.getRegisterMap()).get_st(0), 30.0);
}