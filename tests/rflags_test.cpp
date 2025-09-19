#include "gtest/gtest.h"
#include <cstdint>

const uint64_t RFLAGS_CF_BIT = 0;
const uint64_t RFLAGS_ZF_BIT = 6;
const uint64_t RFLAGS_SF_BIT = 7;
const uint64_t RFLAGS_OF_BIT = 11;

class RFlagsTestHelper {
public:
    uint64_t rflags_ = 0;

    bool get_CF() const { return (rflags_ >> RFLAGS_CF_BIT) & 1; }
    void set_CF(bool value) {
      if (value) rflags_ |= (1ULL << RFLAGS_CF_BIT);
      else rflags_ &= ~(1ULL << RFLAGS_CF_BIT);
    }

    bool get_ZF() const { return (rflags_ >> RFLAGS_ZF_BIT) & 1; }
    void set_ZF(bool value) {
      if (value) rflags_ |= (1ULL << RFLAGS_ZF_BIT);
      else rflags_ &= ~(1ULL << RFLAGS_ZF_BIT);
    }

    bool get_SF() const { return (rflags_ >> RFLAGS_SF_BIT) & 1; }
    void set_SF(bool value) {
      if (value) rflags_ |= (1ULL << RFLAGS_SF_BIT);
      else rflags_ &= ~(1ULL << RFLAGS_SF_BIT);
    }

    bool get_OF() const { return (rflags_ >> RFLAGS_OF_BIT) & 1; }
    void set_OF(bool value) {
      if (value) rflags_ |= (1ULL << RFLAGS_OF_BIT);
      else rflags_ &= ~(1ULL << RFLAGS_OF_BIT);
    }
};

TEST(RFlagsTest, CarryFlag) {
    RFlagsTestHelper flags;
    flags.set_CF(true);
    EXPECT_TRUE(flags.get_CF());
    flags.set_CF(false);
    EXPECT_FALSE(flags.get_CF());
}

TEST(RFlagsTest, ZeroFlag) {
    RFlagsTestHelper flags;
    flags.set_ZF(true);
    EXPECT_TRUE(flags.get_ZF());
    flags.set_ZF(false);
    EXPECT_FALSE(flags.get_ZF());
}

TEST(RFlagsTest, SignFlag) {
    RFlagsTestHelper flags;
    flags.set_SF(true);
    EXPECT_TRUE(flags.get_SF());
    flags.set_SF(false);
    EXPECT_FALSE(flags.get_SF());
}

TEST(RFlagsTest, OverflowFlag) {
    RFlagsTestHelper flags;
    flags.set_OF(true);
    EXPECT_TRUE(flags.get_OF());
    flags.set_OF(false);
    EXPECT_FALSE(flags.get_OF());
}

TEST(RFlagsTest, MultipleFlags) {
    RFlagsTestHelper flags;
    flags.set_CF(true);
    flags.set_ZF(true);
    flags.set_SF(false);
    flags.set_OF(false);

    EXPECT_TRUE(flags.get_CF());
    EXPECT_TRUE(flags.get_ZF());
    EXPECT_FALSE(flags.get_SF());
    EXPECT_FALSE(flags.get_OF());

    flags.set_CF(false);
    flags.set_ZF(false);
    flags.set_SF(true);
    flags.set_OF(true);

    EXPECT_FALSE(flags.get_CF());
    EXPECT_FALSE(flags.get_ZF());
    EXPECT_TRUE(flags.get_SF());
    EXPECT_TRUE(flags.get_OF());
}
