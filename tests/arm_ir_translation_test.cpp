#include "gtest/gtest.h"
#include "../arm_simulator.h"
#include "../arm_to_ir.h"
#include "../ir.h"
#include "../memory.h"
#include "mock_database_manager.h"
#include "../architecture.h"

#include "../file_system_device.h"

// Test fixture for ARM IR translation and execution tests
class ArmIRTranslationTest : public ::testing::Test {
protected:

  void SetUp() override {
        // Create a dummy ARM architecture for the converter
        arm_arch = create_arm_cortex_r8_architecture();
        simulator = std::make_unique<ArmSimulator>(dbManager, memory, 0, true);
    }

    Memory memory;
    MockDatabaseManager dbManager;
    Architecture arm_arch;
    std::unique_ptr<ArmToIrConverter> converter;
    std::unique_ptr<ArmSimulator> simulator;
};

TEST_F(ArmIRTranslationTest, BgtInstruction) {
    std::string arm_assembly = R"(
    cmp r0, #10
    bgt target_label
    mov r0, #0
target_label:
    mov r0, #1
)";

    simulator->loadProgramFromString(arm_assembly);
    simulator->firstPass();
    const auto& symbol_table = simulator->getSymbolTable();

    converter = std::make_unique<ArmToIrConverter>(arm_arch, &symbol_table);
    IRProgram ir_program = converter->convert(arm_assembly);

    // Find the IR instruction for BGT
    std::unique_ptr<IRInstruction> bgt_ir_instr = nullptr;
    for (const auto& instr : ir_program) {
        if (instr->opcode == IROpcode::Branch && std::holds_alternative<IRConditionCode>(instr->operands[1]) && std::get<IRConditionCode>(instr->operands[1]) == IRConditionCode::Greater) {
            bgt_ir_instr = std::make_unique<IRInstruction>(*instr); // Deep copy
            break;
        }
    }
    ASSERT_NE(bgt_ir_instr, nullptr) << "BGT instruction not found in IR program.";
}