#include <gtest/gtest.h>
#include "../formatting_utils.h"

// A sample YMM register value for testing.
// Consists of 4 64-bit integers set in reverse order of memory layout:
// 3: 0x8000000000000000 (a large negative number if signed)
// 2: 0x0000000000000100 (256 in decimal)
// 1: 0x000000000000000A (10 in decimal)
// 0: 0x0000000000000001
const __m256i test_ymm_val = _mm256_set_epi64x(
    0x8000000000000000, // High qword
    0x0000000000000100,
    0x000000000000000A,
    0x0000000000000001  // Low qword
);

TEST(FormattingUtilsTest, Hex256View) {
    std::string expected = "0x80000000000000000000000000000100000000000000000a0000000000000001";
    std::string actual = format_ymm_register(test_ymm_val, YmmViewMode::HEX_256, DisplayBase::HEX);
    EXPECT_EQ(expected, actual);
}

TEST(FormattingUtilsTest, Ints4x64_Decimal) {
    // Note: 0x8... is treated as a large unsigned number by the formatter.
    std::string expected = "1 10 256 9223372036854775808";
    std::string actual = format_ymm_register(test_ymm_val, YmmViewMode::INTS_4x64, DisplayBase::DEC);
    EXPECT_EQ(expected, actual);
}

TEST(FormattingUtilsTest, Ints4x64_Hex) {
    std::string expected = "0x1 0xa 0x100 0x8000000000000000";
    std::string actual = format_ymm_register(test_ymm_val, YmmViewMode::INTS_4x64, DisplayBase::HEX);
    EXPECT_EQ(expected, actual);
}

TEST(FormattingUtilsTest, Ints8x32_Decimal) {
    std::string expected = "1 0 10 0 256 0 0 2147483648";
    std::string actual = format_ymm_register(test_ymm_val, YmmViewMode::INTS_8x32, DisplayBase::DEC);
    EXPECT_EQ(expected, actual);
}

TEST(FormattingUtilsTest, Ints32x8_Octal) {
    std::string actual = format_ymm_register(test_ymm_val, YmmViewMode::INTS_32x8, DisplayBase::OCT);
    // Just check the beginning of the string for correctness
    std::string expected_start = "01 00 00 00 00 00 00 00 012 00 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00";
    EXPECT_EQ(actual.rfind(expected_start, 0), 0);
}
