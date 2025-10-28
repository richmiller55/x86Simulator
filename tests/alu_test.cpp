#include "gtest/gtest.h"
#include "../alu.h"
#include "../x86_simulator.h"
#include "../memory.h"
#include "mock_database_manager.h"

class ALUTest : public ::testing::Test {
protected:
    std::unique_ptr<ALU> alu;
    MockDatabaseManager dbManager;
    Memory memory;
    X86Simulator simulator;

    ALUTest() : alu(std::make_unique<ALU>()), simulator(dbManager, memory, 1, true) {}

    void SetUp() override {
        // Set up any necessary objects or state
    }

    void TearDown() override {
        // Clean up
    }
};

TEST_F(ALUTest, AddExecution) {
    // ADD eax, ebx
    // Initialize eax and ebx registers
    static_cast<RegisterMap&>(simulator.getRegisterMap()).set32("eax", 10);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).set32("ebx", 20);

    // Create an IR instruction
    IRInstruction ir_instr(IROpcode::Add, { "eax", "ebx" }, FunctionalUnitType::ALU);
    PipelinedInstruction p_instr(ir_instr, InstructionState::Executing);

    // Execute the instruction
    alu->execute(p_instr, simulator);

    // Verify the result in eax
    EXPECT_EQ(static_cast<RegisterMap&>(simulator.getRegisterMap()).get32("eax"), 30);
}