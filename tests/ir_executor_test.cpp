#include "gtest/gtest.h"
#include "../x86_simulator.h"
#include "../memory.h"
#include "mock_database_manager.h"
#include "../ir.h"

class IRExecutorTest : public ::testing::Test {
protected:
    MockDatabaseManager dbManager;
    Memory memory;
    X86Simulator simulator;

    IRExecutorTest() : memory(), simulator(dbManager, memory, 1, true) {}

    void SetUp() override {
        // Set up any necessary objects or state
    }

    void TearDown() override {
        // Clean up
    }
};

TEST_F(IRExecutorTest, HandleIrAnd) {
    auto& regs = simulator.getRegisterMapForTesting();
    regs.set32("eax", 0b1100);
    regs.set32("ebx", 0b1010);

    IRInstruction and_instr(IROpcode::And, {
        IRRegister{IRRegisterType::GPR, 0, 32}, // eax
        IRRegister{IRRegisterType::GPR, 3, 32}  // ebx
    });

    // simulator.execute_ir_instruction(and_instr);

    EXPECT_EQ(regs.get32("eax"), 0b1000);
    EXPECT_FALSE(simulator.get_ZF());
    EXPECT_FALSE(simulator.get_SF());
    EXPECT_FALSE(simulator.get_CF());
    EXPECT_FALSE(simulator.get_OF());
}

TEST_F(IRExecutorTest, HandleIrOr) {
    auto& regs = simulator.getRegisterMapForTesting();
    regs.set32("eax", 0b1100);
    regs.set32("ebx", 0b1010);

    IRInstruction or_instr(IROpcode::Or, {
        IRRegister{IRRegisterType::GPR, 0, 32}, // eax
        IRRegister{IRRegisterType::GPR, 3, 32}  // ebx
    });

    simulator.execute_ir_instruction(or_instr);

    EXPECT_EQ(regs.get32("eax"), 0b1110);
    EXPECT_FALSE(simulator.get_ZF());
    EXPECT_FALSE(simulator.get_SF());
}

TEST_F(IRExecutorTest, HandleIrNot) {
    auto& regs = simulator.getRegisterMapForTesting();
    regs.set32("eax", 0xFFFFFF00);

    IRInstruction not_instr(IROpcode::Not, {
        IRRegister{IRRegisterType::GPR, 0, 32} // eax
    });

    simulator.execute_ir_instruction(not_instr);

    EXPECT_EQ(regs.get32("eax"), 0x000000FF);
}

TEST_F(IRExecutorTest, HandleIrShl) {
    auto& regs = simulator.getRegisterMapForTesting();
    regs.set32("eax", 0b1011);

    IRInstruction shl_instr(IROpcode::Shl, {
        IRRegister{IRRegisterType::GPR, 0, 32}, // eax
        (uint64_t)2
    });

    simulator.execute_ir_instruction(shl_instr);

    EXPECT_EQ(regs.get32("eax"), 0b101100);
    EXPECT_TRUE(simulator.get_CF()); // Last bit shifted out was 1
}

TEST_F(IRExecutorTest, HandleIrShr) {
    auto& regs = simulator.getRegisterMapForTesting();
    regs.set32("eax", 0b1011);

    IRInstruction shr_instr(IROpcode::Shr, {
        IRRegister{IRRegisterType::GPR, 0, 32}, // eax
        (uint64_t)2
    });

    simulator.execute_ir_instruction(shr_instr);

    EXPECT_EQ(regs.get32("eax"), 0b10);
    EXPECT_TRUE(simulator.get_CF()); // Last bit shifted out was 1
}

TEST_F(IRExecutorTest, HandleIrSar) {
    auto& regs = simulator.getRegisterMapForTesting();
    regs.set32("eax", 0b10000000000000000000000000001011); // Negative number

    IRInstruction sar_instr(IROpcode::Sar, {
        IRRegister{IRRegisterType::GPR, 0, 32}, // eax
        (uint64_t)2
    });

    simulator.execute_ir_instruction(sar_instr);

    EXPECT_EQ(regs.get32("eax"), 0b11100000000000000000000000000010);
    EXPECT_TRUE(simulator.get_CF()); // Last bit shifted out was 1
}
