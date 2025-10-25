#include "gtest/gtest.h"
#include "../register_map.h"
#include "../architecture.h"

class ArmRegisterMapTest : public ::testing::Test {
protected:
    ArmRegisterMapTest() {}
    void SetUp() override {
    }

    static Architecture arch_;
    static RegisterMap regs;
};

Architecture ArmRegisterMapTest::arch_(create_arm_cortex_r8_architecture());
RegisterMap ArmRegisterMapTest::regs(ArmRegisterMapTest::arch_);

TEST_F(ArmRegisterMapTest, GetSet32) {
    uint32_t value = 0x12345678;
    regs.set32("r0", value);
    EXPECT_EQ(regs.get32("r0"), value);
}

TEST_F(ArmRegisterMapTest, GetSetSP) {
    uint32_t value = 0xABCDEF01;
    regs.set32("sp", value);
    EXPECT_EQ(regs.get32("sp"), value);
}

TEST_F(ArmRegisterMapTest, GetSetLR) {
    uint32_t value = 0xDEADBEEF;
    regs.set32("lr", value);
    EXPECT_EQ(regs.get32("lr"), value);
}

TEST_F(ArmRegisterMapTest, GetSetPC) {
    uint32_t value = 0xFEEDBEEF;
    regs.set32("pc", value);
    EXPECT_EQ(regs.get32("pc"), value);
}

TEST_F(ArmRegisterMapTest, InvalidRegister) {
    EXPECT_THROW(regs.get32("invalid_reg"), std::out_of_range);
    EXPECT_THROW(regs.set32("invalid_reg", 0), std::out_of_range);
}

TEST_F(ArmRegisterMapTest, GetSetCPSR) {
    uint32_t value = 0xF0000000;
    regs.set32("cpsr", value);
    EXPECT_EQ(regs.get32("cpsr"), value);
}

// Test for vector/FPU registers
TEST_F(ArmRegisterMapTest, GetSetDRegister) {
    uint64_t value = 0x1122334455667788;
    regs.set64("d0", value);
    EXPECT_EQ(regs.get64("d0"), value);
}

TEST_F(ArmRegisterMapTest, GetSetSRegister) {
    uint32_t value = 0x99887766;
    regs.set32("s1", value);
    EXPECT_EQ(regs.get32("s1"), value);

    // Check that it correctly updated the upper part of d0
    uint64_t d0_val = regs.get64("d0");
    EXPECT_EQ((d0_val >> 32), value);
}

TEST_F(ArmRegisterMapTest, FullRegisterCoverage) {
    for (const auto& [type, file_def] : arch_.register_files) {
        for (const auto& phys_reg : file_def.registers) {
            for (const auto& alias : phys_reg.aliases) {
                switch (alias.size_bits) {
                    case 64: {
                        uint64_t test_val = 0xDEADBEEFCAFEBABE;
                        regs.set64(alias.name, test_val);
                        EXPECT_EQ(regs.get64(alias.name), test_val) << "Failed for register " << alias.name;
                        break;
                    }
                    case 32: {
                        uint32_t test_val = 0x12345678;
                        regs.set32(alias.name, test_val);
                        EXPECT_EQ(regs.get32(alias.name), test_val) << "Failed for register " << alias.name;
                        break;
                    }
                    case 16: {
                        uint16_t test_val = 0xABCD;
                        regs.set16(alias.name, test_val);
                        EXPECT_EQ(regs.get16(alias.name), test_val) << "Failed for register " << alias.name;
                        break;
                    }
                    case 8: {
                        uint8_t test_val = 0xEF;
                        regs.set8(alias.name, test_val);
                        EXPECT_EQ(regs.get8(alias.name), test_val) << "Failed for register " << alias.name;
                        break;
                    }
                }
            }
        }
    }
}
