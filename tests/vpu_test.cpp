#include "gtest/gtest.h"
#include "../vpu.h"
#include "../x86_simulator.h"
#include "../memory.h"
#include "mock_database_manager.h"
#include "../avx_core.h"
#include <cmath>

class VPUTest : public ::testing::Test {
protected:
    std::unique_ptr<VPU> vpu;
    MockDatabaseManager dbManager;
    Memory memory;
    X86Simulator simulator;

    VPUTest() : vpu(std::make_unique<VPU>()), simulator(dbManager, memory, 1, true) {}

    void SetUp() override {
        // Set up any necessary objects or state
    }

    void TearDown() override {
        // Clean up
    }
};

TEST_F(VPUTest, VaddpsExecution) {
    // VADDPS ymm0, ymm1, ymm2
    // Initialize ymm1 and ymm2 registers
    m256i_t val1 = _mm256_set_ps_sim(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    m256i_t val2 = _mm256_set_ps_sim(8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).setYmm("ymm1", val1);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).setYmm("ymm2", val2);

    // Create an IR instruction
    IRInstruction ir_instr(IROpcode::PackedAddPS, { "ymm0", "ymm1", "ymm2" }, FunctionalUnitType::VPU);
    PipelinedInstruction p_instr(ir_instr, InstructionState::Executing);

    // Execute the instruction
    vpu->execute(p_instr, simulator);

    // Verify the result in ymm0
    m256i_t result = static_cast<RegisterMap&>(simulator.getRegisterMap()).getYmm("ymm0");
    m256i_t expected = _mm256_add_ps_sim(val1, val2);

    for (int i = 0; i < 8; ++i) {
        EXPECT_FLOAT_EQ(result.m256_f32[i], expected.m256_f32[i]);
    }
}

TEST_F(VPUTest, VmaxpsExecution) {
    // VMAXPS ymm0, ymm1, ymm2
    m256i_t val1 = _mm256_set_ps_sim(1.0f, 8.0f, 3.0f, 6.0f, 5.0f, 4.0f, 7.0f, 2.0f);
    m256i_t val2 = _mm256_set_ps_sim(8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).setYmm("ymm1", val1);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).setYmm("ymm2", val2);

    IRInstruction ir_instr(IROpcode::PackedMaxPS, { "ymm0", "ymm1", "ymm2" }, FunctionalUnitType::VPU);
    PipelinedInstruction p_instr(ir_instr, InstructionState::Executing);

    vpu->execute(p_instr, simulator);

    m256i_t result = static_cast<RegisterMap&>(simulator.getRegisterMap()).getYmm("ymm0");
    m256i_t expected = _mm256_max_ps_sim(val1, val2);

    for (int i = 0; i < 8; ++i) {
        EXPECT_FLOAT_EQ(result.m256_f32[i], expected.m256_f32[i]);
    }
}

TEST_F(VPUTest, VpandnExecution) {
    // VPANDN ymm0, ymm1, ymm2
    m256i_t val1 = _mm256_set_epi32_sim(0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xF0F0F0F0, 0x0F0F0F0F, 0xAAAAAAAA, 0x55555555);
    m256i_t val2 = _mm256_set_epi32_sim(0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFF00FF00, 0xFF00FF00, 0xCCCCCCCC, 0xCCCCCCCC);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).setYmm("ymm1", val1);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).setYmm("ymm2", val2);

    IRInstruction ir_instr(IROpcode::PackedAndNot, { "ymm0", "ymm1", "ymm2" }, FunctionalUnitType::VPU);
    PipelinedInstruction p_instr(ir_instr, InstructionState::Executing);

    vpu->execute(p_instr, simulator);

    m256i_t result = static_cast<RegisterMap&>(simulator.getRegisterMap()).getYmm("ymm0");

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

TEST_F(VPUTest, VpandExecution) {
    m256i_t val1 = _mm256_set_epi32_sim(0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xF0F0F0F0, 0x0F0F0F0F, 0xAAAAAAAA, 0x55555555);
    m256i_t val2 = _mm256_set_epi32_sim(0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFF00FF00, 0xFF00FF00, 0xCCCCCCCC, 0xCCCCCCCC);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).setYmm("ymm1", val1);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).setYmm("ymm2", val2);

    IRInstruction ir_instr(IROpcode::PackedAnd, { "ymm0", "ymm1", "ymm2" }, FunctionalUnitType::VPU);
    PipelinedInstruction p_instr(ir_instr, InstructionState::Executing);

    vpu->execute(p_instr, simulator);

    m256i_t result = static_cast<RegisterMap&>(simulator.getRegisterMap()).getYmm("ymm0");

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

TEST_F(VPUTest, VpmullwExecution) {
    m256i_t val1 = _mm256_set_epi16_sim(1, 2, 3, 4, 5, 6, 7, 8, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000);
    m256i_t val2 = _mm256_set_epi16_sim(10, 20, 30, 40, 50, 60, 70, 80, 1, 2, 3, 4, 5, 6, 7, 8);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).setYmm("ymm1", val1);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).setYmm("ymm2", val2);

    IRInstruction ir_instr(IROpcode::PackedMulLowI16, { "ymm0", "ymm1", "ymm2" }, FunctionalUnitType::VPU);
    PipelinedInstruction p_instr(ir_instr, InstructionState::Executing);

    vpu->execute(p_instr, simulator);

    m256i_t result = static_cast<RegisterMap&>(simulator.getRegisterMap()).getYmm("ymm0");

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

TEST_F(VPUTest, VminpsExecution) {
    m256i_t val1 = _mm256_set_ps_sim(1.0f, 8.0f, 3.0f, 6.0f, 5.0f, 4.0f, 7.0f, 2.0f);
    m256i_t val2 = _mm256_set_ps_sim(8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).setYmm("ymm1", val1);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).setYmm("ymm2", val2);

    IRInstruction ir_instr(IROpcode::PackedMinPS, { "ymm0", "ymm1", "ymm2" }, FunctionalUnitType::VPU);
    PipelinedInstruction p_instr(ir_instr, InstructionState::Executing);

    vpu->execute(p_instr, simulator);

    m256i_t result = static_cast<RegisterMap&>(simulator.getRegisterMap()).getYmm("ymm0");
    m256i_t expected = _mm256_min_ps_sim(val1, val2);

    for (int i = 0; i < 8; ++i) {
        EXPECT_FLOAT_EQ(result.m256_f32[i], expected.m256_f32[i]);
    }
}

TEST_F(VPUTest, VpxorExecution) {
    m256i_t val1 = _mm256_set_epi32_sim(0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xF0F0F0F0, 0x0F0F0F0F, 0xAAAAAAAA, 0x55555555);
    m256i_t val2 = _mm256_set_epi32_sim(0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFF00FF00, 0xFF00FF00, 0xCCCCCCCC, 0xCCCCCCCC);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).setYmm("ymm1", val1);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).setYmm("ymm2", val2);

    IRInstruction ir_instr(IROpcode::PackedXor, { "ymm0", "ymm1", "ymm2" }, FunctionalUnitType::VPU);
    PipelinedInstruction p_instr(ir_instr, InstructionState::Executing);

    vpu->execute(p_instr, simulator);

    m256i_t result = static_cast<RegisterMap&>(simulator.getRegisterMap()).getYmm("ymm0");

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

TEST_F(VPUTest, VrcppsExecution) {
    m256i_t val1 = _mm256_set_ps_sim(1.0f, 2.0f, 4.0f, 8.0f, 0.5f, 0.25f, -2.0f, -4.0f);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).setYmm("ymm1", val1);

    IRInstruction ir_instr(IROpcode::PackedReciprocalPS, { "ymm0", "ymm1" }, FunctionalUnitType::VPU);
    PipelinedInstruction p_instr(ir_instr, InstructionState::Executing);

    vpu->execute(p_instr, simulator);

    m256i_t result = static_cast<RegisterMap&>(simulator.getRegisterMap()).getYmm("ymm0");
    m256i_t expected = _mm256_rcp_ps_sim(val1);

    for (int i = 0; i < 8; ++i) {
        EXPECT_NEAR(result.m256_f32[i], expected.m256_f32[i], 0.001);
    }
}

TEST_F(VPUTest, VrcppsExecutionSpecialValues) {
    float inf = std::numeric_limits<float>::infinity();
    float nan = std::numeric_limits<float>::quiet_NaN();

    m256i_t val = _mm256_set_ps_sim(1.0f, nan, inf, -0.0f, 0.0f, -2.0f, 4.0f, 1.0f);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).setYmm("ymm1", val);

    IRInstruction ir_instr(IROpcode::PackedReciprocalPS, { "ymm0", "ymm1" }, FunctionalUnitType::VPU);
    PipelinedInstruction p_instr(ir_instr, InstructionState::Executing);

    vpu->execute(p_instr, simulator);

    m256i_t result = static_cast<RegisterMap&>(simulator.getRegisterMap()).getYmm("ymm0");

    EXPECT_NEAR(result.m256_f32[0], 1.0f, 0.001);
    EXPECT_NEAR(result.m256_f32[1], 0.25f, 0.001);
    EXPECT_NEAR(result.m256_f32[2], -0.5f, 0.001);
    EXPECT_EQ(result.m256_f32[3], std::numeric_limits<float>::infinity());
    EXPECT_EQ(result.m256_f32[4], -std::numeric_limits<float>::infinity());
    EXPECT_EQ(result.m256_f32[5], 0.0f);
    EXPECT_TRUE(std::isnan(result.m256_f32[6]));
    EXPECT_NEAR(result.m256_f32[7], 1.0f, 0.001);
}

TEST_F(VPUTest, VrcppsExecutionMemorySource) {
    address_t mem_addr = simulator.getMemory().get_data_segment_start() + 0x600;
    m256i_t val = _mm256_set_ps_sim(1.0f, 2.0f, 4.0f, 8.0f, 0.5f, 0.25f, -2.0f, -4.0f);
    simulator.getMemory().write_ymm(mem_addr, val);

    IRInstruction ir_instr(IROpcode::PackedReciprocalPS, { "ymm0", IRMemoryOperand{std::nullopt, std::nullopt, 1, static_cast<int64_t>(mem_addr), 256} }, FunctionalUnitType::VPU);
    PipelinedInstruction p_instr(ir_instr, InstructionState::Executing);

    vpu->execute(p_instr, simulator);

    m256i_t result = static_cast<RegisterMap&>(simulator.getRegisterMap()).getYmm("ymm0");
    m256i_t expected = _mm256_rcp_ps_sim(val);

    for (int i = 0; i < 8; ++i) {
        EXPECT_NEAR(result.m256_f32[i], expected.m256_f32[i], 0.001);
    }
}

TEST_F(VPUTest, VsqrtpsExecution) {
    m256i_t val1 = _mm256_set_ps_sim(1.0f, 4.0f, 9.0f, 16.0f, 25.0f, 100.0f, 0.25f, 0.04f);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).setYmm("ymm1", val1);

    IRInstruction ir_instr(IROpcode::PackedSqrtPS, { "ymm0", "ymm1" }, FunctionalUnitType::VPU);
    PipelinedInstruction p_instr(ir_instr, InstructionState::Executing);

    vpu->execute(p_instr, simulator);

    m256i_t result = static_cast<RegisterMap&>(simulator.getRegisterMap()).getYmm("ymm0");
    m256i_t expected = _mm256_sqrt_ps_sim(val1);

    for (int i = 0; i < 8; ++i) {
        EXPECT_NEAR(result.m256_f32[i], expected.m256_f32[i], 1e-6);
    }
}

TEST_F(VPUTest, VsubpsExecution) {
    m256i_t val1 = _mm256_set_ps_sim(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    m256i_t val2 = _mm256_set_ps_sim(8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).setYmm("ymm1", val1);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).setYmm("ymm2", val2);

    IRInstruction ir_instr(IROpcode::PackedSubPS, { "ymm0", "ymm1", "ymm2" }, FunctionalUnitType::VPU);
    PipelinedInstruction p_instr(ir_instr, InstructionState::Executing);

    vpu->execute(p_instr, simulator);

    m256i_t result = static_cast<RegisterMap&>(simulator.getRegisterMap()).getYmm("ymm0");
    m256i_t expected = _mm256_sub_ps_sim(val1, val2);

    for (int i = 0; i < 8; ++i) {
        EXPECT_FLOAT_EQ(result.m256_f32[i], expected.m256_f32[i]);
    }
}

TEST_F(VPUTest, VporExecution) {
    m256i_t val1 = _mm256_set_epi32_sim(0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xF0F0F0F0, 0x0F0F0F0F, 0xAAAAAAAA, 0x55555555);
    m256i_t val2 = _mm256_set_epi32_sim(0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFF00FF00, 0xFF00FF00, 0xCCCCCCCC, 0xCCCCCCCC);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).setYmm("ymm1", val1);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).setYmm("ymm2", val2);

    IRInstruction ir_instr(IROpcode::PackedOr, { "ymm0", "ymm1", "ymm2" }, FunctionalUnitType::VPU);
    PipelinedInstruction p_instr(ir_instr, InstructionState::Executing);

    vpu->execute(p_instr, simulator);

    m256i_t result = static_cast<RegisterMap&>(simulator.getRegisterMap()).getYmm("ymm0");

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

TEST_F(VPUTest, VmovupsLoadExecution_1) {
    // Test VMOVUPS ymm0, [address]
    address_t mem_addr = simulator.getMemory().get_data_segment_start() + 0x100;
    m256i_t test_data = _mm256_set_epi32_sim(8, 7, 6, 5, 4, 3, 2, 1);
    simulator.getMemory().write_ymm(mem_addr, test_data);

    IRInstruction ir_instr(IROpcode::VectorMove, { "ymm0", IRMemoryOperand{std::nullopt, std::nullopt, 1, static_cast<int64_t>(mem_addr), 256} }, FunctionalUnitType::VPU);
    PipelinedInstruction p_instr(ir_instr, InstructionState::Executing);

    vpu->execute(p_instr, simulator);

    m256i_t result = static_cast<RegisterMap&>(simulator.getRegisterMap()).getYmm("ymm0");

    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(result.m256i_i32[i], test_data.m256i_i32[i]);
    }
}

TEST_F(VPUTest, VmovupsStoreExecution_1) {
    // Test VMOVUPS [address], ymm0
    address_t mem_addr = simulator.getMemory().get_data_segment_start() + 0x200;
    m256i_t test_data = _mm256_set_epi32_sim(1, 2, 3, 4, 5, 6, 7, 8);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).setYmm("ymm0", test_data);

    IRInstruction ir_instr(IROpcode::VectorMove, { IRMemoryOperand{std::nullopt, std::nullopt, 1, static_cast<int64_t>(mem_addr), 256}, "ymm0" }, FunctionalUnitType::VPU);
    PipelinedInstruction p_instr(ir_instr, InstructionState::Executing);

    vpu->execute(p_instr, simulator);

    m256i_t mem_data = simulator.getMemory().read_ymm(mem_addr);

    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(mem_data.m256i_i32[i], test_data.m256i_i32[i]);
    }
}

TEST_F(VPUTest, VmovupsLoadExecution_2) {
    address_t mem_addr = simulator.getMemory().get_data_segment_start() + 0x300;
    m256i_t test_data = _mm256_set_epi32_sim(8, 7, 6, 5, 4, 3, 2, 1);
    simulator.getMemory().write_ymm(mem_addr, test_data);

    IRInstruction ir_instr(IROpcode::VectorMove, { "ymm0", IRMemoryOperand{std::nullopt, std::nullopt, 1, static_cast<int64_t>(mem_addr), 256} }, FunctionalUnitType::VPU);
    PipelinedInstruction p_instr(ir_instr, InstructionState::Executing);

    vpu->execute(p_instr, simulator);

    m256i_t result = static_cast<RegisterMap&>(simulator.getRegisterMap()).getYmm("ymm0");

    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(result.m256i_i32[i], test_data.m256i_i32[i]);
    }
}

TEST_F(VPUTest, VmovupsStoreExecution_2) {
    address_t mem_addr = simulator.getMemory().get_data_segment_start() + 0x400;
    m256i_t test_data = _mm256_set_epi32_sim(1, 2, 3, 4, 5, 6, 7, 8);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).setYmm("ymm0", test_data);

    IRInstruction ir_instr(IROpcode::VectorMove, { IRMemoryOperand{std::nullopt, std::nullopt, 1, static_cast<int64_t>(mem_addr), 256}, "ymm0" }, FunctionalUnitType::VPU);
    PipelinedInstruction p_instr(ir_instr, InstructionState::Executing);

    vpu->execute(p_instr, simulator);

    m256i_t mem_data = simulator.getMemory().read_ymm(mem_addr);

    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(mem_data.m256i_i32[i], test_data.m256i_i32[i]);
    }
}

TEST_F(VPUTest, VsqrtpsExecutionSpecialValues) {
    float inf = std::numeric_limits<float>::infinity();
    float nan = std::numeric_limits<float>::quiet_NaN();

    m256i_t val = _mm256_set_ps_sim(1.0f, 1.0f, nan, -1.0f, inf, -0.0f, 0.0f, -1.0f);
    static_cast<RegisterMap&>(simulator.getRegisterMap()).setYmm("ymm1", val);

    IRInstruction ir_instr(IROpcode::PackedSqrtPS, { "ymm0", "ymm1" }, FunctionalUnitType::VPU);
    PipelinedInstruction p_instr(ir_instr, InstructionState::Executing);

    vpu->execute(p_instr, simulator);

    m256i_t result = static_cast<RegisterMap&>(simulator.getRegisterMap()).getYmm("ymm0");

    EXPECT_TRUE(std::isnan(result.m256_f32[0])); // sqrt(-1.0)
    EXPECT_EQ(result.m256_f32[1], 0.0f); // sqrt(0.0)
    EXPECT_EQ(result.m256_f32[2], -0.0f); // sqrt(-0.0)
    EXPECT_EQ(result.m256_f32[3], inf); // sqrt(+inf)
    EXPECT_TRUE(std::isnan(result.m256_f32[4])); // sqrt(-1.0)
    EXPECT_TRUE(std::isnan(result.m256_f32[5])); // sqrt(NaN)
    EXPECT_FLOAT_EQ(result.m256_f32[6], 1.0f);
    EXPECT_FLOAT_EQ(result.m256_f32[7], 1.0f);
}

TEST_F(VPUTest, VsqrtpsExecutionMemorySource) {
    address_t mem_addr = simulator.getMemory().get_data_segment_start() + 0x500;
    m256i_t val = _mm256_set_ps_sim(1.0f, 4.0f, 9.0f, 16.0f, 25.0f, 36.0f, 49.0f, 64.0f);
    simulator.getMemory().write_ymm(mem_addr, val);

    IRInstruction ir_instr(IROpcode::PackedSqrtPS, { "ymm0", IRMemoryOperand{std::nullopt, std::nullopt, 1, static_cast<int64_t>(mem_addr), 256} }, FunctionalUnitType::VPU);
    PipelinedInstruction p_instr(ir_instr, InstructionState::Executing);

    vpu->execute(p_instr, simulator);

    m256i_t result = static_cast<RegisterMap&>(simulator.getRegisterMap()).getYmm("ymm0");
    m256i_t expected = _mm256_sqrt_ps_sim(val);

    for (int i = 0; i < 8; ++i) {
        EXPECT_NEAR(result.m256_f32[i], expected.m256_f32[i], 1e-6);
    }
}
