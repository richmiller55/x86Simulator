#include "gtest/gtest.h"
#include "../register_map.h"

class RegisterMapTest : public ::testing::Test {
protected:
    void SetUp() override {
        regs.init();
    }

    RegisterMap regs;
};

TEST_F(RegisterMapTest, GetSet64) {
    uint64_t value = 0x123456789ABCDEF0;
    regs.set64("rax", value);
    EXPECT_EQ(regs.get64("rax"), value);
}

TEST_F(RegisterMapTest, GetSet32) {
    uint32_t value = 0x12345678;
    regs.set32("eax", value);
    EXPECT_EQ(regs.get32("eax"), value);
}

TEST_F(RegisterMapTest, Set32ZerosUpperBits) {
    uint64_t value64 = 0xFFFFFFFFFFFFFFFF;
    regs.set64("rax", value64);
    
    uint32_t value32 = 0x12345678;
    regs.set32("eax", value32);
    
    EXPECT_EQ(regs.get64("rax"), value32);
}

TEST_F(RegisterMapTest, InvalidRegister64) {
    EXPECT_THROW(regs.get64("invalid_reg"), std::out_of_range);
    EXPECT_THROW(regs.set64("invalid_reg", 0), std::out_of_range);
}

TEST_F(RegisterMapTest, InvalidRegister32) {
    EXPECT_THROW(regs.get32("invalid_reg"), std::out_of_range);
    EXPECT_THROW(regs.set32("invalid_reg", 0), std::out_of_range);
}
