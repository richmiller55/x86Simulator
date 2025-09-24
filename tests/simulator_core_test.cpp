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
    __m256i val2 = _mm256_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFF00FF00, 0xFF00FF00, 0xCCCCCCCC, 0xCCCCCCCC);
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

TEST_F(SimulatorCoreTest, RflagsUpdateOnAdd) {
    // This test verifies that rflags (and by extension eflags) are updated correctly.
    // We will perform an ADD that results in a negative number, setting the Sign Flag (SF).
    // We will also perform an ADD that results in zero, setting the Zero Flag (ZF).

    // --- Test 1: Setting the Sign Flag (SF) ---
    // ADD eax, ebx where eax = -10 and ebx = 5. Result should be -5.
    simulator.getRegisterMapForTesting().set32("eax", static_cast<uint32_t>(-10));
    simulator.getRegisterMapForTesting().set32("ebx", 5);

    DecodedInstruction decoded_instr_sf;
    decoded_instr_sf.mnemonic = "add";
    decoded_instr_sf.operands.push_back({ "eax", 0, OperandType::REGISTER });
    decoded_instr_sf.operands.push_back({ "ebx", 0, OperandType::REGISTER });

    simulator.executeInstruction(decoded_instr_sf);
    // Manually sync flags from the internal rflags_ to the register map for testing.
    // In normal execution, runSingleInstruction handles this.
    simulator.update_rflags_in_register_map();

    uint64_t rflags_after_sf = simulator.getRegisterMapForTesting().get64("rflags");
    EXPECT_EQ(simulator.getRegisterMapForTesting().get32("eax"), static_cast<uint32_t>(-5));
    EXPECT_NE((rflags_after_sf & (1ULL << RFLAGS_SF_BIT)), 0); // Check if SF is set
    EXPECT_EQ((rflags_after_sf & (1ULL << RFLAGS_ZF_BIT)), 0);  // Check if ZF is not set

    // --- Test 2: Setting the Zero Flag (ZF) ---
    // ADD eax, ebx where eax = -5 and ebx = 5. Result should be 0.
    simulator.getRegisterMapForTesting().set32("eax", static_cast<uint32_t>(-5));
    simulator.getRegisterMapForTesting().set32("ebx", 5);

    DecodedInstruction decoded_instr_zf;
    decoded_instr_zf.mnemonic = "add";
    decoded_instr_zf.operands.push_back({ "eax", 0, OperandType::REGISTER });
    decoded_instr_zf.operands.push_back({ "ebx", 0, OperandType::REGISTER });

    simulator.executeInstruction(decoded_instr_zf);

    // Manually sync flags from the internal rflags_ to the register map for testing.
    simulator.update_rflags_in_register_map();

    uint64_t rflags_after_zf = simulator.getRegisterMapForTesting().get64("rflags");
    EXPECT_EQ(simulator.getRegisterMapForTesting().get32("eax"), 0);
    EXPECT_EQ((rflags_after_zf & (1ULL << RFLAGS_SF_BIT)), 0);  // Check if SF is not set
    EXPECT_NE((rflags_after_zf & (1ULL << RFLAGS_ZF_BIT)), 0); // Check if ZF is set
}


TEST_F(SimulatorCoreTest, PushPopExecution) {
    auto& register_map = simulator.getRegisterMapForTesting();
    auto& memory = simulator.getMemoryForTesting();
    uint64_t initial_rsp = memory.get_stack_bottom();

    // --- Test 64-bit PUSH/POP ---
    const uint64_t rax_val = 0x1122334455667788;
    register_map.set64("rsp", initial_rsp);
    register_map.set64("rax", rax_val);
    register_map.set64("rbx", 0); // Clear rbx before pop

    // PUSH RAX
    DecodedInstruction push_rax_instr;
    push_rax_instr.mnemonic = "push";
    push_rax_instr.operands.push_back({ "rax", 0, OperandType::REGISTER });
    simulator.executeInstruction(push_rax_instr);

    uint64_t rsp_after_push = register_map.get64("rsp");
    EXPECT_EQ(rsp_after_push, initial_rsp - 8);

    // POP RBX
    DecodedInstruction pop_rbx_instr;
    pop_rbx_instr.mnemonic = "pop";
    pop_rbx_instr.operands.push_back({ "rbx", 0, OperandType::REGISTER });
    simulator.executeInstruction(pop_rbx_instr);

    uint64_t rsp_after_pop = register_map.get64("rsp");
    EXPECT_EQ(rsp_after_pop, initial_rsp);
    EXPECT_EQ(register_map.get64("rbx"), rax_val);

    // --- Test 32-bit PUSH/POP ---
    const uint32_t ecx_val = 0xAABBCCDD;
    register_map.set64("rsp", initial_rsp);
    register_map.set32("ecx", ecx_val);
    register_map.set32("edx", 0); // Clear edx before pop

    // PUSH ECX
    DecodedInstruction push_ecx_instr;
    push_ecx_instr.mnemonic = "push";
    push_ecx_instr.operands.push_back({ "ecx", 0, OperandType::REGISTER });
    simulator.executeInstruction(push_ecx_instr);

    rsp_after_push = register_map.get64("rsp");
    EXPECT_EQ(rsp_after_push, initial_rsp - 4);

    // To verify the 32-bit push, we pop it into another register.
    // POP EDX
    DecodedInstruction pop_edx_instr;
    pop_edx_instr.mnemonic = "pop";
    pop_edx_instr.operands.push_back({ "edx", 0, OperandType::REGISTER });
    simulator.executeInstruction(pop_edx_instr);

    rsp_after_pop = register_map.get64("rsp");
    EXPECT_EQ(rsp_after_pop, initial_rsp);
    EXPECT_EQ(register_map.get32("edx"), ecx_val);
}
TEST_F(SimulatorCoreTest, PushEcxTest) {
    auto& register_map = simulator.getRegisterMapForTesting();
    auto& memory = simulator.getMemoryForTesting();
    uint64_t initial_rsp = memory.get_stack_bottom();
    const uint32_t ecx_val = 0x55667788;
    register_map.set32("ecx", ecx_val);

    DecodedInstruction push_ecx_instr;
    push_ecx_instr.mnemonic = "push";
    push_ecx_instr.operands.push_back({ "ecx", 0, OperandType::REGISTER });
    simulator.executeInstruction(push_ecx_instr);

    uint64_t rsp_after_push = register_map.get64("rsp");
    EXPECT_EQ(rsp_after_push, initial_rsp - 4);

    uint32_t pushed_value = memory.read_dword(rsp_after_push);
    EXPECT_EQ(pushed_value, ecx_val);
}

TEST_F(SimulatorCoreTest, InInstructionExecution) {
    // Redirect cin
    std::stringstream input;
    input << "A";
    std::streambuf* old_cin = std::cin.rdbuf(input.rdbuf());

    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "in";
    decoded_instr.operands.push_back({ "al", 0, OperandType::REGISTER });
    decoded_instr.operands.push_back({ "0x60", 0x60, OperandType::IMMEDIATE });

    simulator.executeInstruction(decoded_instr);

    EXPECT_EQ(simulator.getRegisterMapForTesting().get8("al"), 'A');

    // Restore cin
    std::cin.rdbuf(old_cin);
}

TEST_F(SimulatorCoreTest, OutInstructionExecution) {
    // Redirect cout
    std::stringstream output;
    std::streambuf* old_cout = std::cout.rdbuf(output.rdbuf());

    simulator.getRegisterMapForTesting().set8("al", 'B');

    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "out";
    decoded_instr.operands.push_back({ "0x61", 0x61, OperandType::IMMEDIATE });
    decoded_instr.operands.push_back({ "al", 0, OperandType::REGISTER });

    simulator.executeInstruction(decoded_instr);

    EXPECT_EQ(output.str(), "B");

    // Restore cout
    std::cout.rdbuf(old_cout);
}

TEST_F(SimulatorCoreTest, VmovupsLoadExecution_1) {
    // Test VMOVUPS ymm0, [address]
    address_t mem_addr = simulator.getMemoryForTesting().get_data_segment_start() + 0x100;
    __m256i test_data = _mm256_set_epi32(8, 7, 6, 5, 4, 3, 2, 1);
    simulator.getMemoryForTesting().write_ymm(mem_addr, test_data);

    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vmovups";
    DecodedOperand dest, src;
    dest.type = OperandType::YMM_REGISTER;
    dest.text = "ymm0";
    src.type = OperandType::MEMORY;
    src.value = mem_addr;
    decoded_instr.operands.push_back(dest);
    decoded_instr.operands.push_back(src);

    simulator.executeInstruction(decoded_instr);

    __m256i result = simulator.getRegisterMapForTesting().getYmm("ymm0");

    int32_t* expected_ints = (int32_t*)&test_data;
    int32_t* actual_ints = (int32_t*)&result;
    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(actual_ints[i], expected_ints[i]);
    }
}

TEST_F(SimulatorCoreTest, VmovupsStoreExecution_1) {
    // Test VMOVUPS [address], ymm0
    address_t mem_addr = simulator.getMemoryForTesting().get_data_segment_start() + 0x200;
    __m256i test_data = _mm256_set_epi32(1, 2, 3, 4, 5, 6, 7, 8);
    simulator.getRegisterMapForTesting().setYmm("ymm0", test_data);

    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vmovups";
    DecodedOperand dest, src;
    dest.type = OperandType::MEMORY;
    dest.value = mem_addr;
    src.type = OperandType::YMM_REGISTER;
    src.text = "ymm0";
    decoded_instr.operands.push_back(dest);
    decoded_instr.operands.push_back(src);

    simulator.executeInstruction(decoded_instr);

    __m256i mem_data = simulator.getMemoryForTesting().read_ymm(mem_addr);

    int32_t* expected_ints = (int32_t*)&test_data;
    int32_t* actual_ints = (int32_t*)&mem_data;
    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(actual_ints[i], expected_ints[i]);
    }
}

TEST_F(SimulatorCoreTest, VmovupsLoadExecution_2) {
    // Test VMOVUPS ymm0, [address]
    address_t mem_addr = simulator.getMemoryForTesting().get_data_segment_start() + 0x300;
    __m256i test_data = _mm256_set_epi32(8, 7, 6, 5, 4, 3, 2, 1);
    simulator.getMemoryForTesting().write_ymm(mem_addr, test_data);

    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vmovups";
    DecodedOperand dest, src;
    dest.type = OperandType::YMM_REGISTER;
    dest.text = "ymm0";
    src.type = OperandType::MEMORY;
    src.value = mem_addr;
    decoded_instr.operands.push_back(dest);
    decoded_instr.operands.push_back(src);

    simulator.executeInstruction(decoded_instr);

    __m256i result = simulator.getRegisterMapForTesting().getYmm("ymm0");

    int32_t* expected_ints = (int32_t*)&test_data;
    int32_t* actual_ints = (int32_t*)&result;
    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(actual_ints[i], expected_ints[i]);
    }
}

TEST_F(SimulatorCoreTest, VmovupsStoreExecution_2) {
    // Test VMOVUPS [address], ymm0
    address_t mem_addr = simulator.getMemoryForTesting().get_data_segment_start() + 0x400;
    __m256i test_data = _mm256_set_epi32(1, 2, 3, 4, 5, 6, 7, 8);
    simulator.getRegisterMapForTesting().setYmm("ymm0", test_data);

    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vmovups";
    DecodedOperand dest, src;
    dest.type = OperandType::MEMORY;
    dest.value = mem_addr;
    src.type = OperandType::YMM_REGISTER;
    src.text = "ymm0";
    decoded_instr.operands.push_back(dest);
    decoded_instr.operands.push_back(src);

    simulator.executeInstruction(decoded_instr);

    __m256i mem_data = simulator.getMemoryForTesting().read_ymm(mem_addr);

    int32_t* expected_ints = (int32_t*)&test_data;
    int32_t* actual_ints = (int32_t*)&mem_data;
    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(actual_ints[i], expected_ints[i]);
    }
}
