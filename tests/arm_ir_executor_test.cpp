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

TEST_F(ArmIRExecutorTest, HandleIrQadd) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 0x7FFFFF00);
    regs.set32("r2", 0x00000100);

    IRInstruction qadd_instr(IROpcode::SaturatingAdd, {
        IRRegister{IRRegisterType::GPR, 0, 32}, // r0
        IRRegister{IRRegisterType::GPR, 1, 32}, // r1
        IRRegister{IRRegisterType::GPR, 2, 32}  // r2
    });

    simulator.execute_ir_instruction(qadd_instr);

    EXPECT_EQ(regs.get32("r0"), 0x7FFFFFFF);
}

TEST_F(ArmIRExecutorTest, HandleIrQsub) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 0x80000000);
    regs.set32("r2", 0x00000001);

    IRInstruction qsub_instr(IROpcode::SaturatingSub, {
        IRRegister{IRRegisterType::GPR, 0, 32}, // r0
        IRRegister{IRRegisterType::GPR, 1, 32}, // r1
        IRRegister{IRRegisterType::GPR, 2, 32}  // r2
    });

    simulator.execute_ir_instruction(qsub_instr);

    EXPECT_EQ(static_cast<int32_t>(regs.get32("r0")), 0x80000000);
}

TEST_F(ArmIRExecutorTest, HandleIrCbnz_BranchTaken) {
    auto& regs = simulator.getRegisterMap();
    const char* ip_name = simulator.get_instruction_pointer_name();
    regs.set32("r1", 123); // Non-zero value
    regs.set64(ip_name, 0x1000);

    IRInstruction cbnz_instr(IROpcode::CompareAndBranchIfNotZero, {
        IRRegister{IRRegisterType::GPR, 1, 32}, // r1
        static_cast<uint64_t>(0x2000) // Target address
    });

    simulator.execute_ir_instruction(cbnz_instr);

    EXPECT_EQ(regs.get64(ip_name), 0x2000);
}

TEST_F(ArmIRExecutorTest, HandleIrCbnz_BranchNotTaken) {
    auto& regs = simulator.getRegisterMap();
    const char* ip_name = simulator.get_instruction_pointer_name();
    regs.set32("r1", 0); // Zero value
    regs.set64(ip_name, 0x1000);

    IRInstruction cbnz_instr(IROpcode::CompareAndBranchIfNotZero, {
        IRRegister{IRRegisterType::GPR, 1, 32}, // r1
        static_cast<uint64_t>(0x2000) // Target address
    });

    simulator.execute_ir_instruction(cbnz_instr);

    EXPECT_EQ(regs.get64(ip_name), 0x1000);
}

TEST_F(ArmIRExecutorTest, HandleIrMovw) {
    auto& regs = simulator.getRegisterMap();

    IRInstruction movw_instr(IROpcode::Move, {
        IRRegister{IRRegisterType::GPR, 0, 32}, // r0
        static_cast<uint64_t>(0x1234) // Immediate value
    });

    simulator.execute_ir_instruction(movw_instr);

    EXPECT_EQ(regs.get32("r0"), 0x1234);
}

TEST_F(ArmIRExecutorTest, HandleIrDiv) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 100);
    regs.set32("r2", 10);

    IRInstruction div_instr(IROpcode::Div, {
        IRRegister{IRRegisterType::GPR, 0, 32}, // r0
        IRRegister{IRRegisterType::GPR, 1, 32}, // r1
        IRRegister{IRRegisterType::GPR, 2, 32}  // r2
    });

        simulator.execute_ir_instruction(div_instr);

    

        EXPECT_EQ(regs.get32("r0"), 10);

    }

    

    TEST_F(ArmIRExecutorTest, HandleIrMrs) {

        auto& regs = simulator.getRegisterMap();

        simulator.set_NZCV(true, false, true, false); // Set some flags

    

        IRInstruction mrs_instr(IROpcode::MoveFromSystemRegister, {

            IRRegister{IRRegisterType::GPR, 0, 32},      // r0

            std::string("cpsr")     // CPSR

        });

    

        simulator.execute_ir_instruction(mrs_instr);

    

        uint32_t cpsr_val = regs.get32("r0");

        EXPECT_EQ((cpsr_val >> 31) & 1, 1); // N

        EXPECT_EQ((cpsr_val >> 30) & 1, 0); // Z

        EXPECT_EQ((cpsr_val >> 29) & 1, 1); // C

        EXPECT_EQ((cpsr_val >> 28) & 1, 0); // V

    }

    

    TEST_F(ArmIRExecutorTest, HandleIrMsr) {

        auto& regs = simulator.getRegisterMap();

        regs.set32("r1", 0b1010 << 28); // Set N and C flags

    

        IRInstruction msr_instr(IROpcode::MoveToSystemRegister, {

            std::string("cpsr"),     // CPSR

            IRRegister{IRRegisterType::GPR, 1, 32}       // r1

        });

    

        simulator.execute_ir_instruction(msr_instr);

    

        EXPECT_TRUE(simulator.get_SF());

        EXPECT_FALSE(simulator.get_ZF());

        EXPECT_TRUE(simulator.get_CF());

            EXPECT_FALSE(simulator.get_OF());

        }

        

        TEST_F(ArmIRExecutorTest, HandleIrRsb) {

            auto& regs = simulator.getRegisterMap();

            regs.set32("r1", 10);

            regs.set32("r2", 20);

        

            IRInstruction rsb_instr(IROpcode::Sub, {

                IRRegister{IRRegisterType::GPR, 0, 32}, // r0

                IRRegister{IRRegisterType::GPR, 2, 32}, // r2

                IRRegister{IRRegisterType::GPR, 1, 32}  // r1

            });

        

            simulator.execute_ir_instruction(rsb_instr);

        

            EXPECT_EQ(regs.get32("r0"), 10);

        }

        

        TEST_F(ArmIRExecutorTest, HandleIrOrr) {

            auto& regs = simulator.getRegisterMap();

            regs.set32("r1", 0b1010);

            regs.set32("r2", 0b0101);

        

            IRInstruction orr_instr(IROpcode::Or, {

                IRRegister{IRRegisterType::GPR, 0, 32}, // r0

                IRRegister{IRRegisterType::GPR, 1, 32}, // r1

                IRRegister{IRRegisterType::GPR, 2, 32}  // r2

            });

        

            simulator.execute_ir_instruction(orr_instr);

        

            EXPECT_EQ(regs.get32("r0"), 0b1111);

        }

        

        TEST_F(ArmIRExecutorTest, HandleIrEor) {

            auto& regs = simulator.getRegisterMap();

            regs.set32("r1", 0b1010);

            regs.set32("r2", 0b1100);

        

            IRInstruction eor_instr(IROpcode::Xor, {

                IRRegister{IRRegisterType::GPR, 0, 32}, // r0

                IRRegister{IRRegisterType::GPR, 1, 32}, // r1

                IRRegister{IRRegisterType::GPR, 2, 32}  // r2

            });

        

            simulator.execute_ir_instruction(eor_instr);

        

                EXPECT_EQ(regs.get32("r0"), 0b0110);

        

            }

        

            

        

            TEST_F(ArmIRExecutorTest, HandleIrMovReg) {

        

                auto& regs = simulator.getRegisterMap();

        

                regs.set32("r1", 0xABCDEF12);

        

            

        

                IRInstruction mov_instr(IROpcode::Move, {

        

                    IRRegister{IRRegisterType::GPR, 0, 32}, // r0

        

                    IRRegister{IRRegisterType::GPR, 1, 32}  // r1

        

                });

        

            

        

                simulator.execute_ir_instruction(mov_instr);

        

            

        

                EXPECT_EQ(regs.get32("r0"), 0xABCDEF12);

        

            }

        

            

        

            TEST_F(ArmIRExecutorTest, HandleIrCmp) {

        

                auto& regs = simulator.getRegisterMap();

        

                regs.set32("r1", 20);

        

                regs.set32("r2", 10);

        

            

        

                IRInstruction cmp_instr(IROpcode::Cmp, {

        

                    IRRegister{IRRegisterType::GPR, 1, 32}, // r1

        

                    IRRegister{IRRegisterType::GPR, 2, 32}  // r2

        

                });

        

            

        

                simulator.execute_ir_instruction(cmp_instr);

        

            

        

                EXPECT_FALSE(simulator.get_ZF());

        

                EXPECT_FALSE(simulator.get_SF());

        

                EXPECT_FALSE(simulator.get_CF());

        

            }

        

            

        

            TEST_F(ArmIRExecutorTest, HandleIrLdr) {

        

                auto& regs = simulator.getRegisterMap();

        

                address_t addr = 0x1000;

        

                uint32_t value = 0xDEADBEEF;

        

                memory.write_dword(addr, value);

        

                regs.set32("r1", addr);

        

            

        

                IRMemoryOperand mem_op;

        

                mem_op.base_reg = IRRegister{IRRegisterType::GPR, 1, 32};

        

                mem_op.size = 32;

        

            

        

                IRInstruction ldr_instr(IROpcode::Load, {

        

                    IRRegister{IRRegisterType::GPR, 0, 32}, // r0

        

                    mem_op

        

                });

        

            

        

                simulator.execute_ir_instruction(ldr_instr);

        

            

        

                EXPECT_EQ(regs.get32("r0"), value);

        

            }

        

            

        

            TEST_F(ArmIRExecutorTest, HandleIrStr) {

        

                auto& regs = simulator.getRegisterMap();

        

                address_t addr = 0x2000;

        

                uint32_t value = 0xCAFEBABE;

        

                regs.set32("r0", value);

        

                regs.set32("r1", addr);

        

            

        

                IRMemoryOperand mem_op;

        

                mem_op.base_reg = IRRegister{IRRegisterType::GPR, 1, 32};

        

                mem_op.size = 32;

        

            

        

                IRInstruction str_instr(IROpcode::Store, {

        

                    mem_op,

        

                    IRRegister{IRRegisterType::GPR, 0, 32} // r0

        

                });

        

            

        

                simulator.execute_ir_instruction(str_instr);

        

            

        

                EXPECT_EQ(memory.read_dword(addr), value);

        

            }

        

            

        

    