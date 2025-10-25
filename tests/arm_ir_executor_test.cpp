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
        std::string("r0"),
        std::string("r1"),
        std::string("r2")
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
        std::string("r0"),
        std::string("r1"),
        std::string("r2")
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
        std::string("r0"),
        std::string("r2"),
        std::string("r1")
    });

    simulator.execute_ir_instruction(rsc_instr);

    EXPECT_EQ(regs.get32("r0"), 10);
}

TEST_F(ArmIRExecutorTest, HandleIrTst) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r0", 0b1010);
    regs.set32("r1", 0b0101);

    IRInstruction tst_instr(IROpcode::Tst, {
        std::string("r0"),
        std::string("r1")
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
        std::string("r0"),
        std::string("r1")
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
        std::string("r0"),
        std::string("r1")
    });

    simulator.execute_ir_instruction(cmn_instr);

    EXPECT_TRUE(simulator.get_ZF());
    EXPECT_FALSE(simulator.get_SF());
}

TEST_F(ArmIRExecutorTest, HandleIrMvn) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 0x12345678);

    IRInstruction mvn_instr(IROpcode::MoveNot, {
        std::string("r0"),
        std::string("r1")
    });

    simulator.execute_ir_instruction(mvn_instr);

    EXPECT_EQ(regs.get32("r0"), 0xEDCBA987);
}

TEST_F(ArmIRExecutorTest, HandleIrBic) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 0b1111);
    regs.set32("r2", 0b1010);

    IRInstruction bic_instr(IROpcode::AndNot, {
        std::string("r0"),
        std::string("r1"),
        std::string("r2")
    });

    simulator.execute_ir_instruction(bic_instr);

    EXPECT_EQ(regs.get32("r0"), 0b0101);
}

TEST_F(ArmIRExecutorTest, HandleIrQadd) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 0x7FFFFF00);
    regs.set32("r2", 0x00000100);

    IRInstruction qadd_instr(IROpcode::SaturatingAdd, {
        std::string("r0"),
        std::string("r1"),
        std::string("r2")
    });

    simulator.execute_ir_instruction(qadd_instr);

    EXPECT_EQ(regs.get32("r0"), 0x7FFFFFFF);
}

TEST_F(ArmIRExecutorTest, HandleIrQsub) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 0x80000000);
    regs.set32("r2", 0x00000001);

    IRInstruction qsub_instr(IROpcode::SaturatingSub, {
        std::string("r0"),
        std::string("r1"),
        std::string("r2")
    });

    simulator.execute_ir_instruction(qsub_instr);

    EXPECT_EQ(static_cast<int32_t>(regs.get32("r0")), 0x80000000);
}

TEST_F(ArmIRExecutorTest, HandleIrCbnz_BranchTaken) {
    auto& regs = simulator.getRegisterMap();
    const char* ip_name = simulator.get_instruction_pointer_name();
    regs.set32("r1", 123); // Non-zero value
    regs.set32(ip_name, 0x1000);

    IRInstruction cbnz_instr(IROpcode::CompareAndBranchIfNotZero, {
        std::string("r1"),
        static_cast<uint64_t>(0x2000) // Target address
    });

    simulator.execute_ir_instruction(cbnz_instr);

    EXPECT_EQ(regs.get32(ip_name), 0x2000);
}

TEST_F(ArmIRExecutorTest, HandleIrCbnz_BranchNotTaken) {
    auto& regs = simulator.getRegisterMap();
    const char* ip_name = simulator.get_instruction_pointer_name();
    regs.set32("r1", 0); // Zero value
    regs.set32(ip_name, 0x1000);

    IRInstruction cbnz_instr(IROpcode::CompareAndBranchIfNotZero, {
        std::string("r1"),
        static_cast<uint64_t>(0x2000) // Target address
    });

    simulator.execute_ir_instruction(cbnz_instr);

    EXPECT_EQ(regs.get32(ip_name), 0x1000);
}

TEST_F(ArmIRExecutorTest, HandleIrMovw) {
    auto& regs = simulator.getRegisterMap();

    IRInstruction movw_instr(IROpcode::Move, {
        std::string("r0"),
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
        std::string("r0"),
        std::string("r1"),
        std::string("r2")
    });

    simulator.execute_ir_instruction(div_instr);

    EXPECT_EQ(regs.get32("r0"), 10);
}

TEST_F(ArmIRExecutorTest, HandleIrMrs) {
    auto& regs = simulator.getRegisterMap();
    simulator.set_NZCV(true, false, true, false); // Set some flags

    IRInstruction mrs_instr(IROpcode::MoveFromSystemRegister, {
        std::string("r0"),      // r0
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
        std::string("r1")       // r1
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
        std::string("r0"), // r0
        std::string("r2"), // r2
        std::string("r1")  // r1
    });

    simulator.execute_ir_instruction(rsb_instr);

    EXPECT_EQ(regs.get32("r0"), 10);
}

TEST_F(ArmIRExecutorTest, HandleIrOrr) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 0b1010);
    regs.set32("r2", 0b0101);

    IRInstruction orr_instr(IROpcode::Or, {
        std::string("r0"), // r0
        std::string("r1"), // r1
        std::string("r2")  // r2
    });

    simulator.execute_ir_instruction(orr_instr);

    EXPECT_EQ(regs.get32("r0"), 0b1111);
}

TEST_F(ArmIRExecutorTest, HandleIrEor) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 0b1010);
    regs.set32("r2", 0b1100);

    IRInstruction eor_instr(IROpcode::Xor, {
        std::string("r0"), // r0
        std::string("r1"), // r1
        std::string("r2")  // r2
    });

    simulator.execute_ir_instruction(eor_instr);

    EXPECT_EQ(regs.get32("r0"), 0b0110);
}

TEST_F(ArmIRExecutorTest, HandleIrMovReg) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 0xABCDEF12);

    IRInstruction mov_instr(IROpcode::Move, {
        std::string("r0"), // r0
        std::string("r1")  // r1
    });

    simulator.execute_ir_instruction(mov_instr);

    EXPECT_EQ(regs.get32("r0"), 0xABCDEF12);
}

TEST_F(ArmIRExecutorTest, HandleIrCmp) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 20);
    regs.set32("r2", 10);

    IRInstruction cmp_instr(IROpcode::Cmp, {
        std::string("r1"), // r1
        std::string("r2")  // r2
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
    mem_op.base_reg = std::string("r1");
    mem_op.size = 32;

    IRInstruction ldr_instr(IROpcode::Load, {
        std::string("r0"), // r0
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
    mem_op.base_reg = std::string("r1");
    mem_op.size = 32;

    IRInstruction str_instr(IROpcode::Store, {
        mem_op,
        std::string("r0") // r0
    });

    simulator.execute_ir_instruction(str_instr);

    EXPECT_EQ(memory.read_dword(addr), value);
}

TEST_F(ArmIRExecutorTest, HandleIrVaddF32) {
    auto& regs = simulator.getRegisterMap();
    float val1 = 1.5f;
    float val2 = 2.75f;
    float expected = val1 + val2;
    regs.set32("s1", *reinterpret_cast<uint32_t*>(&val1));
    regs.set32("s2", *reinterpret_cast<uint32_t*>(&val2));

    IRInstruction vadd_instr(IROpcode::FloatAddS, {
        std::string("s0"), // s0
        std::string("s1"), // s1
        std::string("s2")  // s2
    });

    simulator.execute_ir_instruction(vadd_instr);

    uint32_t result_bits = regs.get32("s0");
    float result_float = *reinterpret_cast<float*>(&result_bits);
    EXPECT_FLOAT_EQ(result_float, expected);
}

TEST_F(ArmIRExecutorTest, HandleIrQdadd) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 0x40000000); // 2^30
    regs.set32("r2", 0x40000000); // 2^30

    IRInstruction qdadd_instr(IROpcode::SaturatingDoubleAdd, {
        std::string("r0"), // r0
        std::string("r1"), // r1
        std::string("r2")  // r2
    });

    simulator.execute_ir_instruction(qdadd_instr);

    // r0 = r1 + 2*r2 = 2^30 + 2*(2^30) = 3 * 2^30, which should saturate
    EXPECT_EQ(regs.get32("r0"), 0x7FFFFFFF);
}

TEST_F(ArmIRExecutorTest, HandleIrQdsub) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 0x80000000); // -2^31
    regs.set32("r2", 0x40000000); // 2^30

    IRInstruction qdsub_instr(IROpcode::SaturatingDoubleSub, {
        std::string("r0"), // r0
        std::string("r1"), // r1
        std::string("r2")  // r2
    });

    simulator.execute_ir_instruction(qdsub_instr);

    // r0 = r1 - 2*r2 = -2^31 - 2*(2^30) = -2^31 - 2^31, which should saturate to min int
    EXPECT_EQ(static_cast<int32_t>(regs.get32("r0")), 0x80000000);
}

TEST_F(ArmIRExecutorTest, HandleIrMla) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 10); // rm
    regs.set32("r2", 20); // rs
    regs.set32("r3", 5);  // rn

    IRInstruction mla_instr(IROpcode::MultiplyAccumulate, {
        std::string("r0"), // rd
        std::string("r1"), // rm
        std::string("r2"), // rs
        std::string("r3")  // rn
    });

    simulator.execute_ir_instruction(mla_instr);

    // r0 = (r1 * r2) + r3 = (10 * 20) + 5 = 205
    EXPECT_EQ(regs.get32("r0"), 205);
}

TEST_F(ArmIRExecutorTest, HandleIrMls) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 10); // rm
    regs.set32("r2", 20); // rs
    regs.set32("r3", 250); // rn

    IRInstruction mls_instr(IROpcode::MultiplySubtract, {
        std::string("r0"), // rd
        std::string("r1"), // rm
        std::string("r2"), // rs
        std::string("r3")  // rn
    });

    simulator.execute_ir_instruction(mls_instr);

    // r0 = r3 - (r1 * r2) = 250 - (10 * 20) = 50
    EXPECT_EQ(regs.get32("r0"), 50);
}

TEST_F(ArmIRExecutorTest, HandleIrUmull) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r2", 0xFFFFFFFF); // rm
    regs.set32("r3", 2);          // rs

    IRInstruction umull_instr(IROpcode::UnsignedMultiplyLong, {
        std::string("r0"), // rdlo
        std::string("r1"), // rdhi
        std::string("r2"), // rm
        std::string("r3")  // rs
    });

    simulator.execute_ir_instruction(umull_instr);

    // {r1, r0} = r2 * r3 = 0xFFFFFFFF * 2 = 0x1FFFFFFFE
    EXPECT_EQ(regs.get32("r0"), 0xFFFFFFFE);
    EXPECT_EQ(regs.get32("r1"), 1);
}

TEST_F(ArmIRExecutorTest, HandleIrSmull) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r2", -2); // rm
    regs.set32("r3", 0x7FFFFFFF); // rs

    IRInstruction smull_instr(IROpcode::SignedMultiplyLong, {
        std::string("r0"), // rdlo
        std::string("r1"), // rdhi
        std::string("r2"), // rm
        std::string("r3")  // rs
    });

    simulator.execute_ir_instruction(smull_instr);

    // {r1, r0} = -2 * 0x7FFFFFFF = -4294967294 = 0xFFFFFFFF00000002
    int64_t result = static_cast<int64_t>(static_cast<int32_t>(regs.get32("r1"))) << 32 | regs.get32("r0");
    EXPECT_EQ(result, -4294967294LL);
}

TEST_F(ArmIRExecutorTest, HandleIrUmlal) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r0", 10); // rdlo
    regs.set32("r1", 0);  // rdhi
    regs.set32("r2", 0xFFFFFFFF); // rm
    regs.set32("r3", 2);          // rs

    IRInstruction umlal_instr(IROpcode::UnsignedMultiplyAccumulateLong, {
        std::string("r0"), // rdlo
        std::string("r1"), // rdhi
        std::string("r2"), // rm
        std::string("r3")  // rs
    });

    simulator.execute_ir_instruction(umlal_instr);

    // {r1, r0} = (r2 * r3) + {r1, r0} = (0xFFFFFFFF * 2) + 10 = 0x1FFFFFFFE + 10 = 0x200000008
    EXPECT_EQ(regs.get32("r0"), 0x00000008);
    EXPECT_EQ(regs.get32("r1"), 2);
}

TEST_F(ArmIRExecutorTest, HandleIrSmlal) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r0", -10); // rdlo
    regs.set32("r1", -1);  // rdhi, so {r1,r0} is -10
    regs.set32("r2", -2); // rm
    regs.set32("r3", 5); // rs

    IRInstruction smlal_instr(IROpcode::SignedMultiplyAccumulateLong, {
        std::string("r0"), // rdlo
        std::string("r1"), // rdhi
        std::string("r2"), // rm
        std::string("r3")  // rs
    });

    simulator.execute_ir_instruction(smlal_instr);

    // {r1, r0} = (r2 * r3) + {r1, r0} = (-2 * 5) + (-10) = -20
    int64_t result = static_cast<int64_t>(static_cast<int32_t>(regs.get32("r1"))) << 32 | regs.get32("r0");
    EXPECT_EQ(result, -20);
}

TEST_F(ArmIRExecutorTest, HandleIrClz) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 0x0000FFFF); // 16 leading zeros

    IRInstruction clz_instr(IROpcode::CountLeadingZeros, {
        std::string("r0"), // r0
        std::string("r1")  // r1
    });

    simulator.execute_ir_instruction(clz_instr);

    EXPECT_EQ(regs.get32("r0"), 16);
}

TEST_F(ArmIRExecutorTest, HandleIrRbit) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 0x12345678);

    IRInstruction rbit_instr(IROpcode::ReverseBits, {
        std::string("r0"), // r0
        std::string("r1")  // r1
    });

    simulator.execute_ir_instruction(rbit_instr);

    EXPECT_EQ(regs.get32("r0"), 0x1E6A2C48); // Pre-calculated
}

TEST_F(ArmIRExecutorTest, HandleIrRev) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 0x12345678);

    IRInstruction rev_instr(IROpcode::ReverseBytes, {
        std::string("r0"), // r0
        std::string("r1")  // r1
    });

    simulator.execute_ir_instruction(rev_instr);

    EXPECT_EQ(regs.get32("r0"), 0x78563412);
}

TEST_F(ArmIRExecutorTest, HandleIrRev16) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 0x12345678);

    IRInstruction rev16_instr(IROpcode::ReverseBytes16, {
        std::string("r0"), // r0
        std::string("r1")  // r1
    });

    simulator.execute_ir_instruction(rev16_instr);

    EXPECT_EQ(regs.get32("r0"), 0x34127856);
}

TEST_F(ArmIRExecutorTest, HandleIrRevsh) {
    auto& regs = simulator.getRegisterMap();
    regs.set32("r1", 0x000012F0);

    IRInstruction revsh_instr(IROpcode::ReverseBytesSignedHalfword, {
        std::string("r0"), // r0
        std::string("r1")  // r1
    });

    simulator.execute_ir_instruction(revsh_instr);
    EXPECT_EQ(static_cast<int32_t>(regs.get32("r0")), static_cast<int32_t>(0xFFFFF012));
}

TEST_F(ArmIRExecutorTest, HandleIrVsubF32) {
    auto& regs = simulator.getRegisterMap();
    float val1 = 5.5f;
    float val2 = 2.25f;
    float expected = val1 - val2;
    regs.set32("s1", *reinterpret_cast<uint32_t*>(&val1));
    regs.set32("s2", *reinterpret_cast<uint32_t*>(&val2));

    IRInstruction vsub_instr(IROpcode::FloatSubS, {
        std::string("s0"), // s0
        std::string("s1"), // s1
        std::string("s2")  // s2
    });

    simulator.execute_ir_instruction(vsub_instr);
    uint32_t result_bits = regs.get32("s0");
    float result_float = *reinterpret_cast<float*>(&result_bits);
    EXPECT_FLOAT_EQ(result_float, expected);
}

TEST_F(ArmIRExecutorTest, HandleIrVmulF32) {
    auto& regs = simulator.getRegisterMap();
    float val1 = 2.5f;
    float val2 = 3.0f;
    float expected = val1 * val2;
    regs.set32("s1", *reinterpret_cast<uint32_t*>(&val1));
    regs.set32("s2", *reinterpret_cast<uint32_t*>(&val2));

    IRInstruction vmul_instr(IROpcode::FloatMulS, {
        std::string("s0"), // s0
        std::string("s1"), // s1
        std::string("s2")  // s2
    });

    simulator.execute_ir_instruction(vmul_instr);
    uint32_t result_bits = regs.get32("s0");
    float result_float = *reinterpret_cast<float*>(&result_bits);
    EXPECT_FLOAT_EQ(result_float, expected);
}

TEST_F(ArmIRExecutorTest, HandleIrVdivF32) {
    auto& regs = simulator.getRegisterMap();
    float val1 = 7.5f;
    float val2 = 2.5f;
    float expected = val1 / val2;
    regs.set32("s1", *reinterpret_cast<uint32_t*>(&val1));
    regs.set32("s2", *reinterpret_cast<uint32_t*>(&val2));

    IRInstruction vdiv_instr(IROpcode::FloatDivS, {
        std::string("s0"), // s0
        std::string("s1"), // s1
        std::string("s2")  // s2
    });

    simulator.execute_ir_instruction(vdiv_instr);
    uint32_t result_bits = regs.get32("s0");
    float result_float = *reinterpret_cast<float*>(&result_bits);
    EXPECT_FLOAT_EQ(result_float, expected);
}

TEST_F(ArmIRExecutorTest, HandleIrVsqrtF32) {
    auto& regs = simulator.getRegisterMap();
    float val1 = 9.0f;
    float expected = 3.0f;
    regs.set32("s1", *reinterpret_cast<uint32_t*>(&val1));

    IRInstruction vsqrt_instr(IROpcode::FloatSqrtS, {
        std::string("s0"), // s0
        std::string("s1")  // s1
    });

    simulator.execute_ir_instruction(vsqrt_instr);
    uint32_t result_bits = regs.get32("s0");
    float result_float = *reinterpret_cast<float*>(&result_bits);
    EXPECT_FLOAT_EQ(result_float, expected);
}
