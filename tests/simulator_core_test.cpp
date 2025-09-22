#include "gtest/gtest.h"
#include "../x86_simulator.h"
#include "../decoder.h"
#include "../memory.h"
#include "mock_database_manager.h"
#include <vector>
#include <immintrin.h>

class SimulatorCoreTest : public ::testing::Test {
protected:
    MockDatabaseManager dbManager;
    X86Simulator simulator;

    SimulatorCoreTest() : simulator(dbManager, 1) {}

    void SetUp() override {
        // Set up any necessary objects or state
    }

    void TearDown() override {
        // Clean up
    }
};

TEST_F(SimulatorCoreTest, VaddpsExecution) {
    // VADDPS ymm0, ymm1, ymm2
    // Initialize ymm1 and ymm2 registers
    __m256 val1 = _mm256_set_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 val2 = _mm256_set_ps(8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
    simulator.getRegisterMapForTesting().setYmm("ymm1", _mm256_castps_si256(val1));
    simulator.getRegisterMapForTesting().setYmm("ymm2", _mm256_castps_si256(val2));

    // Create a decoded instruction object
    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vaddps";
    decoded_instr.length_in_bytes = 4;
    
    DecodedOperand dest, src1, src2;
    dest.type = OperandType::YMM_REGISTER;
    dest.text = "ymm0";
    src1.type = OperandType::YMM_REGISTER;
    src1.text = "ymm1";
    src2.type = OperandType::YMM_REGISTER;
    src2.text = "ymm2";
    decoded_instr.operands.push_back(dest);
    decoded_instr.operands.push_back(src1);
    decoded_instr.operands.push_back(src2);

    // Execute the instruction
    simulator.executeInstruction(decoded_instr);

    // Verify the result in ymm0
    __m256i result_i = simulator.getRegisterMapForTesting().getYmm("ymm0");
    __m256 result_ps = _mm256_castsi256_ps(result_i);

    __m256 expected_ps = _mm256_add_ps(val1, val2);

    float expected_floats[8];
    _mm256_storeu_ps(expected_floats, expected_ps);

    float actual_floats[8];
    _mm256_storeu_ps(actual_floats, result_ps);

    for (int i = 0; i < 8; ++i) {
        EXPECT_FLOAT_EQ(actual_floats[i], expected_floats[i]);
    }
}

TEST_F(SimulatorCoreTest, VmaxpsExecution) {
    // VMAXPS ymm0, ymm1, ymm2
    // Initialize ymm1 and ymm2 registers
    __m256 val1 = _mm256_set_ps(1.0f, 8.0f, 3.0f, 6.0f, 5.0f, 4.0f, 7.0f, 2.0f);
    __m256 val2 = _mm256_set_ps(8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
    simulator.getRegisterMapForTesting().setYmm("ymm1", _mm256_castps_si256(val1));
    simulator.getRegisterMapForTesting().setYmm("ymm2", _mm256_castps_si256(val2));

    // Create a decoded instruction object
    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vmaxps";
    decoded_instr.length_in_bytes = 4;
    
    DecodedOperand dest, src1, src2;
    dest.type = OperandType::YMM_REGISTER;
    dest.text = "ymm0";
    src1.type = OperandType::YMM_REGISTER;
    src1.text = "ymm1";
    src2.type = OperandType::YMM_REGISTER;
    src2.text = "ymm2";
    decoded_instr.operands.push_back(dest);
    decoded_instr.operands.push_back(src1);
    decoded_instr.operands.push_back(src2);

    // Execute the instruction
    simulator.executeInstruction(decoded_instr);

    // Verify the result in ymm0
    __m256i result_i = simulator.getRegisterMapForTesting().getYmm("ymm0");
    __m256 result_ps = _mm256_castsi256_ps(result_i);

    __m256 expected_ps = _mm256_max_ps(val1, val2);

    float expected_floats[8], actual_floats[8];
    _mm256_storeu_ps(expected_floats, expected_ps);
    _mm256_storeu_ps(actual_floats, result_ps);

    for (int i = 0; i < 8; ++i) {
        EXPECT_FLOAT_EQ(actual_floats[i], expected_floats[i]);
    }
}

TEST_F(SimulatorCoreTest, VpandnExecution) {
    // VPANDN ymm0, ymm1, ymm2
    // Initialize ymm1 and ymm2 registers
    __m256i val1 = _mm256_set_epi32(0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xF0F0F0F0, 0x0F0F0F0F, 0xAAAAAAAA, 0x55555555);
    __m256i val2 = _mm256_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFF00FF00, 0xFF00FF00, 0xCCCCCCCC, 0CCCCCCCC);
    simulator.getRegisterMapForTesting().setYmm("ymm1", val1);
    simulator.getRegisterMapForTesting().setYmm("ymm2", val2);

    // Create a decoded instruction object
    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vpandn";
    decoded_instr.length_in_bytes = 4;
    
    DecodedOperand dest, src1, src2;
    dest.type = OperandType::YMM_REGISTER;
    dest.text = "ymm0";
    src1.type = OperandType::YMM_REGISTER;
    src1.text = "ymm1";
    src2.type = OperandType::YMM_REGISTER;
    src2.text = "ymm2";
    decoded_instr.operands.push_back(dest);
    decoded_instr.operands.push_back(src1);
    decoded_instr.operands.push_back(src2);

    // Execute the instruction
    simulator.executeInstruction(decoded_instr);

    // Verify the result in ymm0
    __m256i result = simulator.getRegisterMapForTesting().getYmm("ymm0");

    // Expected result is (~val1) & val2.
    // We must emulate this with AVX1/SSE intrinsics to match the simulator's implementation
    // and ensure compatibility with CPUs that do not support AVX2.
    __m128i val1_low  = _mm256_extractf128_si256(val1, 0);
    __m128i val1_high = _mm256_extractf128_si256(val1, 1);
    __m128i val2_low  = _mm256_extractf128_si256(val2, 0);
    __m128i val2_high = _mm256_extractf128_si256(val2, 1);
    __m128i expected_low = _mm_andnot_si128(val1_low, val2_low);
    __m128i expected_high = _mm_andnot_si128(val1_high, val2_high);
    __m256i expected = _mm256_set_m128i(expected_high, expected_low);

    uint32_t expected_ints[8], actual_ints[8];
    _mm256_storeu_si256((__m256i*)expected_ints, expected);
    _mm256_storeu_si256((__m256i*)actual_ints, result);

    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(actual_ints[i], expected_ints[i]);
    }
}

TEST_F(SimulatorCoreTest, VpandExecution) {
    // VPAND ymm0, ymm1, ymm2
    // Initialize ymm1 and ymm2 registers
    __m256i val1 = _mm256_set_epi32(0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xF0F0F0F0, 0x0F0F0F0F, 0xAAAAAAAA, 0x55555555);
    __m256i val2 = _mm256_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFF00FF00, 0xFF00FF00, 0xCCCCCCCC, 0xCCCCCCCC);
    simulator.getRegisterMapForTesting().setYmm("ymm1", val1);
    simulator.getRegisterMapForTesting().setYmm("ymm2", val2);

    // Create a decoded instruction object
    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vpand";
    decoded_instr.length_in_bytes = 4;
    
    DecodedOperand dest, src1, src2;
    dest.type = OperandType::YMM_REGISTER;
    dest.text = "ymm0";
    src1.type = OperandType::YMM_REGISTER;
    src1.text = "ymm1";
    src2.type = OperandType::YMM_REGISTER;
    src2.text = "ymm2";
    decoded_instr.operands.push_back(dest);
    decoded_instr.operands.push_back(src1);
    decoded_instr.operands.push_back(src2);

    // Execute the instruction
    simulator.executeInstruction(decoded_instr);

    // Verify the result in ymm0
    __m256i result = simulator.getRegisterMapForTesting().getYmm("ymm0");

    // Expected result is val1 & val2, emulated with SSE.
    __m128i val1_low  = _mm256_extractf128_si256(val1, 0);
    __m128i val1_high = _mm256_extractf128_si256(val1, 1);
    __m128i val2_low  = _mm256_extractf128_si256(val2, 0);
    __m128i val2_high = _mm256_extractf128_si256(val2, 1);
    __m128i expected_low = _mm_and_si128(val1_low, val2_low);
    __m128i expected_high = _mm_and_si128(val1_high, val2_high);
    __m256i expected = _mm256_set_m128i(expected_high, expected_low);

    uint32_t expected_ints[8], actual_ints[8];
    _mm256_storeu_si256((__m256i*)expected_ints, expected);
    _mm256_storeu_si256((__m256i*)actual_ints, result);

    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(actual_ints[i], expected_ints[i]);
    }
}

TEST_F(SimulatorCoreTest, VpmullwExecution) {
    // VPMULLW ymm0, ymm1, ymm2
    // Initialize ymm1 and ymm2 registers with 16-bit values
    __m256i val1 = _mm256_set_epi16(
        1, 2, 3, 4, 5, 6, 7, 8,
        1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000
    );
    __m256i val2 = _mm256_set_epi16(
        10, 20, 30, 40, 50, 60, 70, 80,
        1, 2, 3, 4, 5, 6, 7, 8
    );
    simulator.getRegisterMapForTesting().setYmm("ymm1", val1);
    simulator.getRegisterMapForTesting().setYmm("ymm2", val2);

    // Create a decoded instruction object
    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vpmullw";
    decoded_instr.length_in_bytes = 4;
    
    DecodedOperand dest, src1, src2;
    dest.type = OperandType::YMM_REGISTER; dest.text = "ymm0";
    src1.type = OperandType::YMM_REGISTER; src1.text = "ymm1";
    src2.type = OperandType::YMM_REGISTER; src2.text = "ymm2";
    decoded_instr.operands.push_back(dest);
    decoded_instr.operands.push_back(src1);
    decoded_instr.operands.push_back(src2);

    // Execute the instruction
    simulator.executeInstruction(decoded_instr);

    // Verify the result in ymm0
    __m256i result = simulator.getRegisterMapForTesting().getYmm("ymm0");

    // Expected result is val1 * val2 (low 16 bits), emulated with SSE.
    __m128i val1_low  = _mm256_extractf128_si256(val1, 0);
    __m128i val1_high = _mm256_extractf128_si256(val1, 1);
    __m128i val2_low  = _mm256_extractf128_si256(val2, 0);
    __m128i val2_high = _mm256_extractf128_si256(val2, 1);
    __m128i expected_low = _mm_mullo_epi16(val1_low, val2_low);
    __m128i expected_high = _mm_mullo_epi16(val1_high, val2_high);
    __m256i expected = _mm256_set_m128i(expected_high, expected_low);

    uint16_t expected_shorts[16], actual_shorts[16];
    _mm256_storeu_si256((__m256i*)expected_shorts, expected);
    _mm256_storeu_si256((__m256i*)actual_shorts, result);

    for (int i = 0; i < 16; ++i) {
        EXPECT_EQ(actual_shorts[i], expected_shorts[i]);
    }
}

TEST_F(SimulatorCoreTest, VminpsExecution) {
    // VMINPS ymm0, ymm1, ymm2
    // Initialize ymm1 and ymm2 registers
    __m256 val1 = _mm256_set_ps(1.0f, 8.0f, 3.0f, 6.0f, 5.0f, 4.0f, 7.0f, 2.0f);
    __m256 val2 = _mm256_set_ps(8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
    simulator.getRegisterMapForTesting().setYmm("ymm1", _mm256_castps_si256(val1));
    simulator.getRegisterMapForTesting().setYmm("ymm2", _mm256_castps_si256(val2));

    // Create a decoded instruction object
    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vminps";
    decoded_instr.length_in_bytes = 4;
    
    DecodedOperand dest, src1, src2;
    dest.type = OperandType::YMM_REGISTER;
    dest.text = "ymm0";
    src1.type = OperandType::YMM_REGISTER;
    src1.text = "ymm1";
    src2.type = OperandType::YMM_REGISTER;
    src2.text = "ymm2";
    decoded_instr.operands.push_back(dest);
    decoded_instr.operands.push_back(src1);
    decoded_instr.operands.push_back(src2);

    // Execute the instruction
    simulator.executeInstruction(decoded_instr);

    // Verify the result in ymm0
    __m256i result_i = simulator.getRegisterMapForTesting().getYmm("ymm0");
    __m256 result_ps = _mm256_castsi256_ps(result_i);

    __m256 expected_ps = _mm256_min_ps(val1, val2);

    float expected_floats[8], actual_floats[8];
    _mm256_storeu_ps(expected_floats, expected_ps);
    _mm256_storeu_ps(actual_floats, result_ps);

    for (int i = 0; i < 8; ++i) {
        EXPECT_FLOAT_EQ(actual_floats[i], expected_floats[i]);
    }
}

TEST_F(SimulatorCoreTest, VpxorExecution) {
    // VPXOR ymm0, ymm1, ymm2
    // Initialize ymm1 and ymm2 registers
    __m256i val1 = _mm256_set_epi32(0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xF0F0F0F0, 0x0F0F0F0F, 0xAAAAAAAA, 0x55555555);
    __m256i val2 = _mm256_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFF00FF00, 0xFF00FF00, 0xCCCCCCCC, 0xCCCCCCCC);
    simulator.getRegisterMapForTesting().setYmm("ymm1", val1);
    simulator.getRegisterMapForTesting().setYmm("ymm2", val2);

    // Create a decoded instruction object
    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vpxor";
    decoded_instr.length_in_bytes = 4;
    
    DecodedOperand dest, src1, src2;
    dest.type = OperandType::YMM_REGISTER; dest.text = "ymm0";
    src1.type = OperandType::YMM_REGISTER; src1.text = "ymm1";
    src2.type = OperandType::YMM_REGISTER; src2.text = "ymm2";
    decoded_instr.operands.push_back(dest);
    decoded_instr.operands.push_back(src1);
    decoded_instr.operands.push_back(src2);

    // Execute the instruction
    simulator.executeInstruction(decoded_instr);

    // Verify the result in ymm0
    __m256i result = simulator.getRegisterMapForTesting().getYmm("ymm0");

    // Expected result is val1 ^ val2, emulated with SSE.
    __m128i val1_low  = _mm256_extractf128_si256(val1, 0);
    __m128i val1_high = _mm256_extractf128_si256(val1, 1);
    __m128i val2_low  = _mm256_extractf128_si256(val2, 0);
    __m128i val2_high = _mm256_extractf128_si256(val2, 1);
    __m128i expected_low = _mm_xor_si128(val1_low, val2_low);
    __m128i expected_high = _mm_xor_si128(val1_high, val2_high);
    __m256i expected = _mm256_set_m128i(expected_high, expected_low);

    uint32_t expected_ints[8], actual_ints[8];
    _mm256_storeu_si256((__m256i*)expected_ints, expected);
    _mm256_storeu_si256((__m256i*)actual_ints, result);

    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(actual_ints[i], expected_ints[i]);
    }
}

TEST_F(SimulatorCoreTest, VrcppsExecution) {
    // VRCPPS ymm0, ymm1
    // Initialize ymm1 register
    __m256 val1 = _mm256_set_ps(1.0f, 2.0f, 4.0f, 8.0f, 0.5f, 0.25f, -2.0f, -4.0f);
    simulator.getRegisterMapForTesting().setYmm("ymm1", _mm256_castps_si256(val1));

    // Create a decoded instruction object
    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vrcpps";
    decoded_instr.length_in_bytes = 4;
    
    DecodedOperand dest, src;
    dest.type = OperandType::YMM_REGISTER; dest.text = "ymm0";
    src.type = OperandType::YMM_REGISTER; src.text = "ymm1";
    decoded_instr.operands.push_back(dest);
    decoded_instr.operands.push_back(src);

    // Execute the instruction
    simulator.executeInstruction(decoded_instr);

    // Verify the result in ymm0
    __m256i result_i = simulator.getRegisterMapForTesting().getYmm("ymm0");
    __m256 result_ps = _mm256_castsi256_ps(result_i);

    // The result is an approximation, so we can't do an exact comparison.
    // We'll calculate the expected result and check if the actual result is close.
    __m256 expected_ps = _mm256_rcp_ps(val1);

    float expected_floats[8], actual_floats[8];
    _mm256_storeu_ps(expected_floats, expected_ps);
    _mm256_storeu_ps(actual_floats, result_ps);

    // VRCPPS has a maximum relative error of 1.5*2^-12. We'll use a larger tolerance for the test.
    for (int i = 0; i < 8; ++i) {
        EXPECT_NEAR(actual_floats[i], expected_floats[i], 0.001);
    }
}

TEST_F(SimulatorCoreTest, VsqrtpsExecution) {
    // VSQRTPS ymm0, ymm1
    // Initialize ymm1 register
    __m256 val1 = _mm256_set_ps(1.0f, 4.0f, 9.0f, 16.0f, 25.0f, 100.0f, 0.25f, 0.04f);
    simulator.getRegisterMapForTesting().setYmm("ymm1", _mm256_castps_si256(val1));

    // Create a decoded instruction object
    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vsqrtps";
    decoded_instr.length_in_bytes = 4;
    
    DecodedOperand dest, src;
    dest.type = OperandType::YMM_REGISTER; dest.text = "ymm0";
    src.type = OperandType::YMM_REGISTER; src.text = "ymm1";
    decoded_instr.operands.push_back(dest);
    decoded_instr.operands.push_back(src);

    // Execute the instruction
    simulator.executeInstruction(decoded_instr);

    // Verify the result in ymm0
    __m256i result_i = simulator.getRegisterMapForTesting().getYmm("ymm0");
    __m256 result_ps = _mm256_castsi256_ps(result_i);

    __m256 expected_ps = _mm256_sqrt_ps(val1);

    float expected_floats[8], actual_floats[8];
    _mm256_storeu_ps(expected_floats, expected_ps);
    _mm256_storeu_ps(actual_floats, result_ps);

    // VSQRTPS is more precise than VRCPPS, so we can use a smaller tolerance.
    for (int i = 0; i < 8; ++i) {
        EXPECT_NEAR(actual_floats[i], expected_floats[i], 1e-6);
    }
}

TEST_F(SimulatorCoreTest, VsubpsExecution) {
    // VSUBPS ymm0, ymm1, ymm2
    // Initialize ymm1 and ymm2 registers
    __m256 val1 = _mm256_set_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 val2 = _mm256_set_ps(8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
    simulator.getRegisterMapForTesting().setYmm("ymm1", _mm256_castps_si256(val1));
    simulator.getRegisterMapForTesting().setYmm("ymm2", _mm256_castps_si256(val2));

    // Create a decoded instruction object
    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vsubps";
    decoded_instr.length_in_bytes = 4;
    
    DecodedOperand dest, src1, src2;
    dest.type = OperandType::YMM_REGISTER; dest.text = "ymm0";
    src1.type = OperandType::YMM_REGISTER; src1.text = "ymm1";
    src2.type = OperandType::YMM_REGISTER; src2.text = "ymm2";
    decoded_instr.operands.push_back(dest);
    decoded_instr.operands.push_back(src1);
    decoded_instr.operands.push_back(src2);

    // Execute the instruction
    simulator.executeInstruction(decoded_instr);

    // Verify the result in ymm0
    __m256i result_i = simulator.getRegisterMapForTesting().getYmm("ymm0");
    __m256 result_ps = _mm256_castsi256_ps(result_i);

    __m256 expected_ps = _mm256_sub_ps(val1, val2);

    float expected_floats[8], actual_floats[8];
    _mm256_storeu_ps(expected_floats, expected_ps);
    _mm256_storeu_ps(actual_floats, result_ps);

    for (int i = 0; i < 8; ++i) {
        EXPECT_FLOAT_EQ(actual_floats[i], expected_floats[i]);
    }
}

TEST_F(SimulatorCoreTest, VporExecution) {
    // VPOR ymm0, ymm1, ymm2
    // Initialize ymm1 and ymm2 registers
    __m256i val1 = _mm256_set_epi32(0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xF0F0F0F0, 0x0F0F0F0F, 0xAAAAAAAA, 0x55555555);
    __m256i val2 = _mm256_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFF00FF00, 0xFF00FF00, 0xCCCCCCCC, 0xCCCCCCCC);
    simulator.getRegisterMapForTesting().setYmm("ymm1", val1);
    simulator.getRegisterMapForTesting().setYmm("ymm2", val2);

    // Create a decoded instruction object
    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vpor";
    decoded_instr.length_in_bytes = 4;
    
    DecodedOperand dest, src1, src2;
    dest.type = OperandType::YMM_REGISTER; dest.text = "ymm0";
    src1.type = OperandType::YMM_REGISTER; src1.text = "ymm1";
    src2.type = OperandType::YMM_REGISTER; src2.text = "ymm2";
    decoded_instr.operands.push_back(dest);
    decoded_instr.operands.push_back(src1);
    decoded_instr.operands.push_back(src2);

    // Execute the instruction
    simulator.executeInstruction(decoded_instr);

    // Verify the result in ymm0
    __m256i result = simulator.getRegisterMapForTesting().getYmm("ymm0");

    // Expected result is val1 | val2, emulated with SSE.
    __m128i val1_low  = _mm256_extractf128_si256(val1, 0);
    __m128i val1_high = _mm256_extractf128_si256(val1, 1);
    __m128i val2_low  = _mm256_extractf128_si256(val2, 0);
    __m128i val2_high = _mm256_extractf128_si256(val2, 1);
    __m128i expected_low = _mm_or_si128(val1_low, val2_low);
    __m128i expected_high = _mm_or_si128(val1_high, val2_high);
    __m256i expected = _mm256_set_m128i(expected_high, expected_low);

    uint32_t expected_ints[8], actual_ints[8];
    _mm256_storeu_si256((__m256i*)expected_ints, expected);
    _mm256_storeu_si256((__m256i*)actual_ints, result);

    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(actual_ints[i], expected_ints[i]);
    }
}
