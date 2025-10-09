#include "gtest/gtest.h"
#include "../x86_simulator.h"
#include "../decoder.h"
#include "../memory.h"
#include "mock_database_manager.h"
#include <vector>
#include "../avx_core.h"
#include <cmath>
#include <limits>

class SimulatorCoreTest : public ::testing::Test {
protected:
    MockDatabaseManager dbManager;
    Memory memory;
    X86Simulator simulator;

    SimulatorCoreTest() : memory(), simulator(dbManager, memory, 1) {}

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
    m256i_t val1 = _mm256_set_ps_sim(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    m256i_t val2 = _mm256_set_ps_sim(8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
    simulator.getRegisterMapForTesting().setYmm("ymm1", val1);
    simulator.getRegisterMapForTesting().setYmm("ymm2", val2);

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
    m256i_t result = simulator.getRegisterMapForTesting().getYmm("ymm0");
    m256i_t expected = _mm256_add_ps_sim(val1, val2);

    for (int i = 0; i < 8; ++i) {
        EXPECT_FLOAT_EQ(result.m256_f32[i], expected.m256_f32[i]);
    }
}

TEST_F(SimulatorCoreTest, VmaxpsExecution) {
    // VMAXPS ymm0, ymm1, ymm2
    m256i_t val1 = _mm256_set_ps_sim(1.0f, 8.0f, 3.0f, 6.0f, 5.0f, 4.0f, 7.0f, 2.0f);
    m256i_t val2 = _mm256_set_ps_sim(8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
    simulator.getRegisterMapForTesting().setYmm("ymm1", val1);
    simulator.getRegisterMapForTesting().setYmm("ymm2", val2);

    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vmaxps";
    decoded_instr.operands.push_back({ "ymm0", 0, OperandType::YMM_REGISTER });
    decoded_instr.operands.push_back({ "ymm1", 0, OperandType::YMM_REGISTER });
    decoded_instr.operands.push_back({ "ymm2", 0, OperandType::YMM_REGISTER });

    simulator.executeInstruction(decoded_instr);

    m256i_t result = simulator.getRegisterMapForTesting().getYmm("ymm0");
    m256i_t expected = _mm256_max_ps_sim(val1, val2);

    for (int i = 0; i < 8; ++i) {
        EXPECT_FLOAT_EQ(result.m256_f32[i], expected.m256_f32[i]);
    }
}

TEST_F(SimulatorCoreTest, VpandnExecution) {
    // VPANDN ymm0, ymm1, ymm2
    m256i_t val1 = _mm256_set_epi32_sim(0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xF0F0F0F0, 0x0F0F0F0F, 0xAAAAAAAA, 0x55555555);
    m256i_t val2 = _mm256_set_epi32_sim(0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFF00FF00, 0xFF00FF00, 0xCCCCCCCC, 0xCCCCCCCC);
    simulator.getRegisterMapForTesting().setYmm("ymm1", val1);
    simulator.getRegisterMapForTesting().setYmm("ymm2", val2);

    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vpandn";
    decoded_instr.operands.push_back({ "ymm0", 0, OperandType::YMM_REGISTER });
    decoded_instr.operands.push_back({ "ymm1", 0, OperandType::YMM_REGISTER });
    decoded_instr.operands.push_back({ "ymm2", 0, OperandType::YMM_REGISTER });

    simulator.executeInstruction(decoded_instr);

    m256i_t result = simulator.getRegisterMapForTesting().getYmm("ymm0");

    m128i_t val1_low  = _mm256_extractf128_si256_sim(val1, 0);
    m128i_t val1_high = _mm256_extractf128_si256_sim(val1, 1);
    m128i_t val2_low  = _mm256_extractf128_si256_sim(val2, 0);
    m128i_t val2_high = _mm256_extractf128_si256_sim(val2, 1);
    m128i_t expected_low = _mm_andnot_si128_sim(val1_low, val2_low);
    m128i_t expected_high = _mm_andnot_si128_sim(val1_high, val2_high);
    m256i_t expected = _mm256_set_m128i_sim(expected_high, expected_low);

    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(result.m256i_i32[i], expected.m256i_i32[i]);
    }
}

TEST_F(SimulatorCoreTest, VpandExecution) {
    m256i_t val1 = _mm256_set_epi32_sim(0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xF0F0F0F0, 0x0F0F0F0F, 0xAAAAAAAA, 0x55555555);
    m256i_t val2 = _mm256_set_epi32_sim(0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFF00FF00, 0xFF00FF00, 0xCCCCCCCC, 0xCCCCCCCC);
    simulator.getRegisterMapForTesting().setYmm("ymm1", val1);
    simulator.getRegisterMapForTesting().setYmm("ymm2", val2);

    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vpand";
    decoded_instr.operands.push_back({ "ymm0", 0, OperandType::YMM_REGISTER });
    decoded_instr.operands.push_back({ "ymm1", 0, OperandType::YMM_REGISTER });
    decoded_instr.operands.push_back({ "ymm2", 0, OperandType::YMM_REGISTER });

    simulator.executeInstruction(decoded_instr);

    m256i_t result = simulator.getRegisterMapForTesting().getYmm("ymm0");

    m128i_t val1_low  = _mm256_extractf128_si256_sim(val1, 0);
    m128i_t val1_high = _mm256_extractf128_si256_sim(val1, 1);
    m128i_t val2_low  = _mm256_extractf128_si256_sim(val2, 0);
    m128i_t val2_high = _mm256_extractf128_si256_sim(val2, 1);
    m128i_t expected_low = _mm_and_si128_sim(val1_low, val2_low);
    m128i_t expected_high = _mm_and_si128_sim(val1_high, val2_high);
    m256i_t expected = _mm256_set_m128i_sim(expected_high, expected_low);

    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(result.m256i_i32[i], expected.m256i_i32[i]);
    }
}

TEST_F(SimulatorCoreTest, VpmullwExecution) {
    m256i_t val1 = _mm256_set_epi16_sim(1, 2, 3, 4, 5, 6, 7, 8, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000);
    m256i_t val2 = _mm256_set_epi16_sim(10, 20, 30, 40, 50, 60, 70, 80, 1, 2, 3, 4, 5, 6, 7, 8);
    simulator.getRegisterMapForTesting().setYmm("ymm1", val1);
    simulator.getRegisterMapForTesting().setYmm("ymm2", val2);

    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vpmullw";
    decoded_instr.operands.push_back({ "ymm0", 0, OperandType::YMM_REGISTER });
    decoded_instr.operands.push_back({ "ymm1", 0, OperandType::YMM_REGISTER });
    decoded_instr.operands.push_back({ "ymm2", 0, OperandType::YMM_REGISTER });

    simulator.executeInstruction(decoded_instr);

    m256i_t result = simulator.getRegisterMapForTesting().getYmm("ymm0");

    m128i_t val1_low  = _mm256_extractf128_si256_sim(val1, 0);
    m128i_t val1_high = _mm256_extractf128_si256_sim(val1, 1);
    m128i_t val2_low  = _mm256_extractf128_si256_sim(val2, 0);
    m128i_t val2_high = _mm256_extractf128_si256_sim(val2, 1);
    m128i_t expected_low = _mm_mullo_epi16_sim(val1_low, val2_low);
    m128i_t expected_high = _mm_mullo_epi16_sim(val1_high, val2_high);
    m256i_t expected = _mm256_set_m128i_sim(expected_high, expected_low);

    for (int i = 0; i < 16; ++i) {
        EXPECT_EQ(result.m256i_i16[i], expected.m256i_i16[i]);
    }
}

TEST_F(SimulatorCoreTest, VminpsExecution) {
    m256i_t val1 = _mm256_set_ps_sim(1.0f, 8.0f, 3.0f, 6.0f, 5.0f, 4.0f, 7.0f, 2.0f);
    m256i_t val2 = _mm256_set_ps_sim(8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
    simulator.getRegisterMapForTesting().setYmm("ymm1", val1);
    simulator.getRegisterMapForTesting().setYmm("ymm2", val2);

    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vminps";
    decoded_instr.operands.push_back({ "ymm0", 0, OperandType::YMM_REGISTER });
    decoded_instr.operands.push_back({ "ymm1", 0, OperandType::YMM_REGISTER });
    decoded_instr.operands.push_back({ "ymm2", 0, OperandType::YMM_REGISTER });

    simulator.executeInstruction(decoded_instr);

    m256i_t result = simulator.getRegisterMapForTesting().getYmm("ymm0");
    m256i_t expected = _mm256_min_ps_sim(val1, val2);

    for (int i = 0; i < 8; ++i) {
        EXPECT_FLOAT_EQ(result.m256_f32[i], expected.m256_f32[i]);
    }
}

TEST_F(SimulatorCoreTest, VpxorExecution) {
    m256i_t val1 = _mm256_set_epi32_sim(0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xF0F0F0F0, 0x0F0F0F0F, 0xAAAAAAAA, 0x55555555);
    m256i_t val2 = _mm256_set_epi32_sim(0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFF00FF00, 0xFF00FF00, 0xCCCCCCCC, 0xCCCCCCCC);
    simulator.getRegisterMapForTesting().setYmm("ymm1", val1);
    simulator.getRegisterMapForTesting().setYmm("ymm2", val2);

    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vpxor";
    decoded_instr.operands.push_back({ "ymm0", 0, OperandType::YMM_REGISTER });
    decoded_instr.operands.push_back({ "ymm1", 0, OperandType::YMM_REGISTER });
    decoded_instr.operands.push_back({ "ymm2", 0, OperandType::YMM_REGISTER });

    simulator.executeInstruction(decoded_instr);

    m256i_t result = simulator.getRegisterMapForTesting().getYmm("ymm0");

    m128i_t val1_low  = _mm256_extractf128_si256_sim(val1, 0);
    m128i_t val1_high = _mm256_extractf128_si256_sim(val1, 1);
    m128i_t val2_low  = _mm256_extractf128_si256_sim(val2, 0);
    m128i_t val2_high = _mm256_extractf128_si256_sim(val2, 1);
    m128i_t expected_low = _mm_xor_si128_sim(val1_low, val2_low);
    m128i_t expected_high = _mm_xor_si128_sim(val1_high, val2_high);
    m256i_t expected = _mm256_set_m128i_sim(expected_high, expected_low);

    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(result.m256i_i32[i], expected.m256i_i32[i]);
    }
}

TEST_F(SimulatorCoreTest, VrcppsExecution) {
    m256i_t val1 = _mm256_set_ps_sim(1.0f, 2.0f, 4.0f, 8.0f, 0.5f, 0.25f, -2.0f, -4.0f);
    simulator.getRegisterMapForTesting().setYmm("ymm1", val1);

    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vrcpps";
    decoded_instr.operands.push_back({ "ymm0", 0, OperandType::YMM_REGISTER });
    decoded_instr.operands.push_back({ "ymm1", 0, OperandType::YMM_REGISTER });

    simulator.executeInstruction(decoded_instr);

    m256i_t result = simulator.getRegisterMapForTesting().getYmm("ymm0");
    m256i_t expected = _mm256_rcp_ps_sim(val1);

    for (int i = 0; i < 8; ++i) {
        EXPECT_NEAR(result.m256_f32[i], expected.m256_f32[i], 0.001);
    }
}

TEST_F(SimulatorCoreTest, VsqrtpsExecution) {
    m256i_t val1 = _mm256_set_ps_sim(1.0f, 4.0f, 9.0f, 16.0f, 25.0f, 100.0f, 0.25f, 0.04f);
    simulator.getRegisterMapForTesting().setYmm("ymm1", val1);

    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vsqrtps";
    decoded_instr.operands.push_back({ "ymm0", 0, OperandType::YMM_REGISTER });
    decoded_instr.operands.push_back({ "ymm1", 0, OperandType::YMM_REGISTER });

    simulator.executeInstruction(decoded_instr);

    m256i_t result = simulator.getRegisterMapForTesting().getYmm("ymm0");
    m256i_t expected = _mm256_sqrt_ps_sim(val1);

    for (int i = 0; i < 8; ++i) {
        EXPECT_NEAR(result.m256_f32[i], expected.m256_f32[i], 1e-6);
    }
}

TEST_F(SimulatorCoreTest, VsubpsExecution) {
    m256i_t val1 = _mm256_set_ps_sim(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    m256i_t val2 = _mm256_set_ps_sim(8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
    simulator.getRegisterMapForTesting().setYmm("ymm1", val1);
    simulator.getRegisterMapForTesting().setYmm("ymm2", val2);

    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vsubps";
    decoded_instr.operands.push_back({ "ymm0", 0, OperandType::YMM_REGISTER });
    decoded_instr.operands.push_back({ "ymm1", 0, OperandType::YMM_REGISTER });
    decoded_instr.operands.push_back({ "ymm2", 0, OperandType::YMM_REGISTER });

    simulator.executeInstruction(decoded_instr);

    m256i_t result = simulator.getRegisterMapForTesting().getYmm("ymm0");
    m256i_t expected = _mm256_sub_ps_sim(val1, val2);

    for (int i = 0; i < 8; ++i) {
        EXPECT_FLOAT_EQ(result.m256_f32[i], expected.m256_f32[i]);
    }
}

TEST_F(SimulatorCoreTest, VporExecution) {
    m256i_t val1 = _mm256_set_epi32_sim(0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xF0F0F0F0, 0x0F0F0F0F, 0xAAAAAAAA, 0x55555555);
    m256i_t val2 = _mm256_set_epi32_sim(0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFF00FF00, 0xFF00FF00, 0xCCCCCCCC, 0xCCCCCCCC);
    simulator.getRegisterMapForTesting().setYmm("ymm1", val1);
    simulator.getRegisterMapForTesting().setYmm("ymm2", val2);

    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vpor";
    decoded_instr.operands.push_back({ "ymm0", 0, OperandType::YMM_REGISTER });
    decoded_instr.operands.push_back({ "ymm1", 0, OperandType::YMM_REGISTER });
    decoded_instr.operands.push_back({ "ymm2", 0, OperandType::YMM_REGISTER });

    simulator.executeInstruction(decoded_instr);

    m256i_t result = simulator.getRegisterMapForTesting().getYmm("ymm0");

    m128i_t val1_low  = _mm256_extractf128_si256_sim(val1, 0);
    m128i_t val1_high = _mm256_extractf128_si256_sim(val1, 1);
    m128i_t val2_low  = _mm256_extractf128_si256_sim(val2, 0);
    m128i_t val2_high = _mm256_extractf128_si256_sim(val2, 1);
    m128i_t expected_low = _mm_or_si128_sim(val1_low, val2_low);
    m128i_t expected_high = _mm_or_si128_sim(val1_high, val2_high);
    m256i_t expected = _mm256_set_m128i_sim(expected_high, expected_low);

    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(result.m256i_i32[i], expected.m256i_i32[i]);
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
    m256i_t test_data = _mm256_set_epi32_sim(8, 7, 6, 5, 4, 3, 2, 1);
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

    m256i_t result = simulator.getRegisterMapForTesting().getYmm("ymm0");

    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(result.m256i_i32[i], test_data.m256i_i32[i]);
    }
}

TEST_F(SimulatorCoreTest, VmovupsStoreExecution_1) {
    // Test VMOVUPS [address], ymm0
    address_t mem_addr = simulator.getMemoryForTesting().get_data_segment_start() + 0x200;
    m256i_t test_data = _mm256_set_epi32_sim(1, 2, 3, 4, 5, 6, 7, 8);
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

    m256i_t mem_data = simulator.getMemoryForTesting().read_ymm(mem_addr);

    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(mem_data.m256i_i32[i], test_data.m256i_i32[i]);
    }
}

TEST_F(SimulatorCoreTest, VmovupsLoadExecution_2) {
    address_t mem_addr = simulator.getMemoryForTesting().get_data_segment_start() + 0x300;
    m256i_t test_data = _mm256_set_epi32_sim(8, 7, 6, 5, 4, 3, 2, 1);
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

    m256i_t result = simulator.getRegisterMapForTesting().getYmm("ymm0");

    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(result.m256i_i32[i], test_data.m256i_i32[i]);
    }
}

TEST_F(SimulatorCoreTest, VmovupsStoreExecution_2) {
    address_t mem_addr = simulator.getMemoryForTesting().get_data_segment_start() + 0x400;
    m256i_t test_data = _mm256_set_epi32_sim(1, 2, 3, 4, 5, 6, 7, 8);
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

    m256i_t mem_data = simulator.getMemoryForTesting().read_ymm(mem_addr);

    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(mem_data.m256i_i32[i], test_data.m256i_i32[i]);
    }
}

TEST_F(SimulatorCoreTest, VsqrtpsExecutionSpecialValues) {
    float inf = std::numeric_limits<float>::infinity();
    float nan = std::numeric_limits<float>::quiet_NaN();

    m256i_t val = _mm256_set_ps_sim(1.0f, 1.0f, nan, -1.0f, inf, -0.0f, 0.0f, -1.0f);
    simulator.getRegisterMapForTesting().setYmm("ymm1", val);

    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vsqrtps";
    decoded_instr.operands.push_back({ "ymm0", 0, OperandType::YMM_REGISTER });
    decoded_instr.operands.push_back({ "ymm1", 0, OperandType::YMM_REGISTER });

    simulator.executeInstruction(decoded_instr);

    m256i_t result = simulator.getRegisterMapForTesting().getYmm("ymm0");

    EXPECT_TRUE(std::isnan(result.m256_f32[0])); // sqrt(-1.0)
    EXPECT_EQ(result.m256_f32[1], 0.0f); // sqrt(0.0)
    EXPECT_EQ(result.m256_f32[2], -0.0f); // sqrt(-0.0)
    EXPECT_EQ(result.m256_f32[3], inf); // sqrt(+inf)
    EXPECT_TRUE(std::isnan(result.m256_f32[4])); // sqrt(-1.0)
    EXPECT_TRUE(std::isnan(result.m256_f32[5])); // sqrt(NaN)
    EXPECT_FLOAT_EQ(result.m256_f32[6], 1.0f);
    EXPECT_FLOAT_EQ(result.m256_f32[7], 1.0f);
}

TEST_F(SimulatorCoreTest, VsqrtpsExecutionMemorySource) {
    address_t mem_addr = simulator.getMemoryForTesting().get_data_segment_start() + 0x500;
    m256i_t val = _mm256_set_ps_sim(1.0f, 4.0f, 9.0f, 16.0f, 25.0f, 36.0f, 49.0f, 64.0f);
    simulator.getMemoryForTesting().write_ymm(mem_addr, val);

    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vsqrtps";
    DecodedOperand dest, src;
    dest.type = OperandType::YMM_REGISTER;
    dest.text = "ymm0";
    src.type = OperandType::MEMORY;
    src.value = mem_addr;
    decoded_instr.operands.push_back(dest);
    decoded_instr.operands.push_back(src);

    simulator.executeInstruction(decoded_instr);

    m256i_t result = simulator.getRegisterMapForTesting().getYmm("ymm0");
    m256i_t expected = _mm256_sqrt_ps_sim(val);

    for (int i = 0; i < 8; ++i) {
        EXPECT_NEAR(result.m256_f32[i], expected.m256_f32[i], 1e-6);
    }
}

TEST_F(SimulatorCoreTest, VrcppsExecutionSpecialValues) {
    float inf = std::numeric_limits<float>::infinity();
    float nan = std::numeric_limits<float>::quiet_NaN();

    m256i_t val = _mm256_set_ps_sim(1.0f, nan, inf, -0.0f, 0.0f, -2.0f, 4.0f, 1.0f);
    simulator.getRegisterMapForTesting().setYmm("ymm1", val);

    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vrcpps";
    decoded_instr.operands.push_back({ "ymm0", 0, OperandType::YMM_REGISTER });
    decoded_instr.operands.push_back({ "ymm1", 0, OperandType::YMM_REGISTER });

    simulator.executeInstruction(decoded_instr);

    m256i_t result = simulator.getRegisterMapForTesting().getYmm("ymm0");

    EXPECT_NEAR(result.m256_f32[0], 1.0f, 0.001);
    EXPECT_NEAR(result.m256_f32[1], 0.25f, 0.001);
    EXPECT_NEAR(result.m256_f32[2], -0.5f, 0.001);
    EXPECT_EQ(result.m256_f32[3], std::numeric_limits<float>::infinity());
    EXPECT_EQ(result.m256_f32[4], -std::numeric_limits<float>::infinity());
    EXPECT_EQ(result.m256_f32[5], 0.0f);
    EXPECT_TRUE(std::isnan(result.m256_f32[6]));
    EXPECT_NEAR(result.m256_f32[7], 1.0f, 0.001);
}

TEST_F(SimulatorCoreTest, VrcppsExecutionMemorySource) {
    address_t mem_addr = simulator.getMemoryForTesting().get_data_segment_start() + 0x600;
    m256i_t val = _mm256_set_ps_sim(1.0f, 2.0f, 4.0f, 8.0f, 0.5f, 0.25f, -2.0f, -4.0f);
    simulator.getMemoryForTesting().write_ymm(mem_addr, val);

    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "vrcpps";
    DecodedOperand dest, src;
    dest.type = OperandType::YMM_REGISTER;
    dest.text = "ymm0";
    src.type = OperandType::MEMORY;
    src.value = mem_addr;
    decoded_instr.operands.push_back(dest);
    decoded_instr.operands.push_back(src);

    simulator.executeInstruction(decoded_instr);

    m256i_t result = simulator.getRegisterMapForTesting().getYmm("ymm0");
    m256i_t expected = _mm256_rcp_ps_sim(val);

    for (int i = 0; i < 8; ++i) {
        EXPECT_NEAR(result.m256_f32[i], expected.m256_f32[i], 0.001);
    }
}

TEST_F(SimulatorCoreTest, CallExecution) {
    auto& register_map = simulator.getRegisterMapForTesting();
    auto& memory = simulator.getMemoryForTesting();
    
    address_t initial_rip = 0x1000;
    uint64_t initial_rsp = memory.get_stack_bottom();
    address_t call_target = 0x2000;
    // The return address is the address of the instruction immediately following the CALL.
    address_t return_address = initial_rip + 5; // Length of CALL rel32 is 5 bytes

    register_map.set64("rip", initial_rip);
    register_map.set64("rsp", initial_rsp);

    // Create a decoded instruction object for CALL
    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "call";
    decoded_instr.length_in_bytes = 5;
    decoded_instr.address = initial_rip;
    
    DecodedOperand target;
    target.type = OperandType::IMMEDIATE;
    target.value = call_target;
    decoded_instr.operands.push_back(target);

    // Execute the instruction
    simulator.executeInstruction(decoded_instr);

    // Verify RIP is updated to the call target
    EXPECT_EQ(register_map.get64("rip"), call_target);

    // Verify RSP is decremented (assuming 64-bit mode, return address is 8 bytes)
    uint64_t rsp_after_call = register_map.get64("rsp");
    EXPECT_EQ(rsp_after_call, initial_rsp - 8);

    // Verify the return address was pushed to the stack
    uint64_t return_address_from_stack = memory.read64(rsp_after_call);
    EXPECT_EQ(return_address_from_stack, return_address);
}

TEST_F(SimulatorCoreTest, JmpExecution) {
    auto& register_map = simulator.getRegisterMapForTesting();
    address_t initial_rip = 0x1000;
    address_t target_address = 0x2000;

    register_map.set64("rip", initial_rip);

    DecodedInstruction jmp_instr;
    jmp_instr.mnemonic = "jmp";
    jmp_instr.operands.push_back({ "0x2000", target_address, OperandType::IMMEDIATE });

    simulator.executeInstruction(jmp_instr);

    EXPECT_EQ(register_map.get64("rip"), target_address);
}

TEST_F(SimulatorCoreTest, JeExecution) {
    auto& register_map = simulator.getRegisterMapForTesting();
    address_t initial_rip = 0x1000;
    address_t target_address = 0x2000;

    // --- Case 1: Jump taken (ZF=1) ---
    register_map.set64("rip", initial_rip);
    // Execute `sub eax, eax` to set ZF=1
    DecodedInstruction sub_instr;
    sub_instr.mnemonic = "sub";
    sub_instr.operands.push_back({ "eax", 0, OperandType::REGISTER });
    sub_instr.operands.push_back({ "eax", 0, OperandType::REGISTER });
    simulator.executeInstruction(sub_instr);

    DecodedInstruction je_instr_taken;
    je_instr_taken.mnemonic = "je";
    je_instr_taken.operands.push_back({ "0x2000", target_address, OperandType::IMMEDIATE });
    
    // The RIP is set to the location of the JE instruction before executing it.
    register_map.set64("rip", initial_rip);
    simulator.executeInstruction(je_instr_taken);
    EXPECT_EQ(register_map.get64("rip"), target_address);

    // --- Case 2: Jump not taken (ZF=0) ---
    register_map.set64("rip", initial_rip);
    // Execute `mov eax, 1` then `cmp eax, 0` to set ZF=0
    register_map.set32("eax", 1);
    DecodedInstruction cmp_instr;
    cmp_instr.mnemonic = "cmp";
    cmp_instr.operands.push_back({ "eax", 0, OperandType::REGISTER });
    cmp_instr.operands.push_back({ "0", 0, OperandType::IMMEDIATE });
    simulator.executeInstruction(cmp_instr);

    DecodedInstruction je_instr_not_taken;
    je_instr_not_taken.mnemonic = "je";
    je_instr_not_taken.operands.push_back({ "0x2000", target_address, OperandType::IMMEDIATE });

    // The RIP is set to the location of the JE instruction before executing it.
    register_map.set64("rip", initial_rip);
    simulator.executeInstruction(je_instr_not_taken);
    // RIP should not be modified by the handler if jump is not taken
    EXPECT_EQ(register_map.get64("rip"), initial_rip);
}

TEST_F(SimulatorCoreTest, JgeExecution) {
    auto& register_map = simulator.getRegisterMapForTesting();
    address_t initial_rip = 0x1000;
    address_t target_address = 0x2000;

    // --- Case 1: Jump taken (SF=OF) ---
    register_map.set64("rip", initial_rip);
    // Execute `mov eax, 10` then `cmp eax, 5`. Result is 5. SF=0, OF=0. SF==OF.
    register_map.set32("eax", 10);
    DecodedInstruction cmp_instr_taken;
    cmp_instr_taken.mnemonic = "cmp";
    cmp_instr_taken.operands.push_back({ "eax", 0, OperandType::REGISTER });
    cmp_instr_taken.operands.push_back({ "5", 5, OperandType::IMMEDIATE });
    simulator.executeInstruction(cmp_instr_taken);

    DecodedInstruction jge_instr_taken;
    jge_instr_taken.mnemonic = "jge";
    jge_instr_taken.operands.push_back({ "0x2000", target_address, OperandType::IMMEDIATE });
    
    // The RIP is set to the location of the JGE instruction before executing it.
    register_map.set64("rip", initial_rip);
    simulator.executeInstruction(jge_instr_taken);
    EXPECT_EQ(register_map.get64("rip"), target_address);

    // --- Case 2: Jump not taken (SF!=OF) ---
    register_map.set64("rip", initial_rip);
    // Execute `mov eax, 5` then `cmp eax, 10`. Result is -5. SF=1, OF=0. SF!=OF.
    register_map.set32("eax", 5);
    DecodedInstruction cmp_instr_not_taken;
    cmp_instr_not_taken.mnemonic = "cmp";
    cmp_instr_not_taken.operands.push_back({ "eax", 0, OperandType::REGISTER });
    cmp_instr_not_taken.operands.push_back({ "10", 10, OperandType::IMMEDIATE });
    simulator.executeInstruction(cmp_instr_not_taken);

    DecodedInstruction jge_instr_not_taken;
    jge_instr_not_taken.mnemonic = "jge";
    jge_instr_not_taken.operands.push_back({ "0x2000", target_address, OperandType::IMMEDIATE });

    // The RIP is set to the location of the JGE instruction before executing it.
    register_map.set64("rip", initial_rip);
    simulator.executeInstruction(jge_instr_not_taken);
    // RIP should not be modified by the handler if jump is not taken
    EXPECT_EQ(register_map.get64("rip"), initial_rip);
}