#include "gtest/gtest.h"
#include "../arm_simulator.h"
#include "../memory.h"
#include "mock_database_manager.h"
#include "../ir.h"

class ArmIRExecutorTest : public ::testing::Test {
protected:
    MockDatabaseManager dbManager;
    Memory memory;
    ArmSimulator simulator;

    ArmIRExecutorTest() : memory(), simulator(dbManager, memory, 1, true) {}

    void SetUp() override {
        // Set up any necessary objects or state
    }

    void TearDown() override {
        // Clean up
    }
};

TEST_F(ArmIRExecutorTest, HandleIrAdc) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 10);
    regs.set32("r2", 20);
    simulator.set_CF(true);

    IRInstruction adc_instr(IROpcode::AddC, {
        IRRegister{IRRegisterType::GPR, 0, 32}, // r0
        IRRegister{IRRegisterType::GPR, 1, 32}, // r1
        IRRegister{IRRegisterType::GPR, 2, 32}  // r2
    });

    simulator.execute_ir_instruction(adc_instr);

    EXPECT_EQ(regs.get32("r0"), 31);
}

TEST_F(ArmIRExecutorTest, HandleIrSbc) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 20);
    regs.set32("r2", 10);
    simulator.set_CF(true); // No borrow

    IRInstruction sbc_instr(IROpcode::SubC, {
        IRRegister{IRRegisterType::GPR, 0, 32}, // r0
        IRRegister{IRRegisterType::GPR, 1, 32}, // r1
        IRRegister{IRRegisterType::GPR, 2, 32}  // r2
    });

    simulator.execute_ir_instruction(sbc_instr);

    EXPECT_EQ(regs.get32("r0"), 10);
}

TEST_F(ArmIRExecutorTest, HandleIrRsc) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 10);
    regs.set32("r2", 20);
    simulator.set_CF(true); // No borrow

    IRInstruction rsc_instr(IROpcode::SubC, {
        IRRegister{IRRegisterType::GPR, 0, 32}, // r0
        IRRegister{IRRegisterType::GPR, 2, 32}, // r2
        IRRegister{IRRegisterType::GPR, 1, 32}  // r1
    });

    simulator.execute_ir_instruction(rsc_instr);

    EXPECT_EQ(regs.get32("r0"), 10);
}

TEST_F(ArmIRExecutorTest, HandleIrTst) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r0", 0b1010);
    regs.set32("r1", 0b0101);

    IRInstruction tst_instr(IROpcode::Tst, {
        IRRegister{IRRegisterType::GPR, 0, 32}, // r0
        IRRegister{IRRegisterType::GPR, 1, 32}  // r1
    });

    simulator.execute_ir_instruction(tst_instr);

    EXPECT_TRUE(simulator.get_ZF());
    EXPECT_FALSE(simulator.get_SF());
}

TEST_F(ArmIRExecutorTest, HandleIrTeq) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r0", 0b1010);
    regs.set32("r1", 0b1010);

    IRInstruction teq_instr(IROpcode::Teq, {
        IRRegister{IRRegisterType::GPR, 0, 32}, // r0
        IRRegister{IRRegisterType::GPR, 1, 32}  // r1
    });

    simulator.execute_ir_instruction(teq_instr);

    EXPECT_TRUE(simulator.get_ZF());
    EXPECT_FALSE(simulator.get_SF());
}

TEST_F(ArmIRExecutorTest, HandleIrCmn) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r0", 10);
    regs.set32("r1", -10);

    IRInstruction cmn_instr(IROpcode::Cmn, {
        IRRegister{IRRegisterType::GPR, 0, 32}, // r0
        IRRegister{IRRegisterType::GPR, 1, 32}  // r1
    });

    simulator.execute_ir_instruction(cmn_instr);

    EXPECT_TRUE(simulator.get_ZF());
    EXPECT_FALSE(simulator.get_SF());
}

TEST_F(ArmIRExecutorTest, HandleIrMvn) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 0x12345678);

    IRInstruction mvn_instr(IROpcode::MoveNot, {
        IRRegister{IRRegisterType::GPR, 0, 32}, // r0
        IRRegister{IRRegisterType::GPR, 1, 32}  // r1
    });

    simulator.execute_ir_instruction(mvn_instr);

    EXPECT_EQ(regs.get32("r0"), 0xEDCBA987);
}

TEST_F(ArmIRExecutorTest, HandleIrBic) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 0b1111);
    regs.set32("r2", 0b1010);

    IRInstruction bic_instr(IROpcode::AndNot, {
        IRRegister{IRRegisterType::GPR, 0, 32}, // r0
        IRRegister{IRRegisterType::GPR, 1, 32}, // r1
        IRRegister{IRRegisterType::GPR, 2, 32}  // r2
    });

    simulator.execute_ir_instruction(bic_instr);

    EXPECT_EQ(regs.get32("r0"), 0b0101);
}