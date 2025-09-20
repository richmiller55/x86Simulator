#include "gtest/gtest.h"
#include "../decoder.h"
#include "../memory.h"
#include <vector>

// Test fixture for Decoder tests
class DecoderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up any necessary objects or state
    }

    void TearDown() override {
        // Reset the singleton to ensure no state leaks between test suites.
        decoder.resetInstance();
    }

    Decoder& decoder = Decoder::getInstance();
};


TEST_F(DecoderTest, GetMnemonic) {
    EXPECT_EQ(decoder.getMnemonic(0x90), "NOP");
    EXPECT_EQ(decoder.getMnemonic(0x55), "PUSH");
    EXPECT_EQ(decoder.getMnemonic(0x5D), "POP");
    EXPECT_EQ(decoder.getMnemonic(0x01), "ADD");
    EXPECT_EQ(decoder.getMnemonic(0x29), "SUB");
    EXPECT_EQ(decoder.getMnemonic(0xEB), "JMP");
}

TEST_F(DecoderTest, GetOpcode) {
    EXPECT_EQ(decoder.getOpcode("NOP"), 0x90);
    EXPECT_EQ(decoder.getOpcode("PUSH"), 0x55);
    EXPECT_EQ(decoder.getOpcode("POP"), 0x5D);
    EXPECT_EQ(decoder.getOpcode("ADD"), 0x01);
    EXPECT_EQ(decoder.getOpcode("SUB"), 0x29);
    EXPECT_EQ(decoder.getOpcode("JMP"), 0xEB);
}

TEST_F(DecoderTest, GetInstructionLength) {
    EXPECT_EQ(decoder.getInstructionLength(0x90), 1); // NOP
    EXPECT_EQ(decoder.getInstructionLength(0x55), 1); // PUSH EBP
    EXPECT_EQ(decoder.getInstructionLength(0x5D), 1); // POP EBP
    EXPECT_EQ(decoder.getInstructionLength(0x01), 2); // ADD r/m32, r32
    EXPECT_EQ(decoder.getInstructionLength(0x29), 2); // SUB r/m32, r32
    EXPECT_EQ(decoder.getInstructionLength(0xEB), 2); // JMP rel8
}

TEST_F(DecoderTest, DecodeSimpleInstruction) {
    Memory memory(1024, 1024, 1024);
    address_t start_address = 0x100;

    // Test NOP instruction (0x90)
    memory.write_text(start_address, 0x90);
    auto decoded_instruction = decoder.decodeInstruction(memory, start_address);    
    ASSERT_NE(decoded_instruction, nullptr);
    EXPECT_EQ(decoded_instruction->mnemonic, "nop");
    EXPECT_EQ(decoded_instruction->length_in_bytes, 1);
    EXPECT_EQ(decoded_instruction->address, start_address);
}

TEST_F(DecoderTest, DecodeInstructionWithImmediate) {
    Memory memory(1024, 1024, 1024);
    address_t start_address = 0x200;

    // Test MOV EAX, imm32 instruction (B8 01 00 00 00)
    std::vector<uint8_t> instruction_bytes = {0xB8, 0x01, 0x00, 0x00, 0x00};
    for (size_t i = 0; i < instruction_bytes.size(); ++i) {
        memory.write_text(start_address + i, instruction_bytes[i]);
    }

    auto decoded_instruction = decoder.decodeInstruction(memory, start_address);    
    ASSERT_NE(decoded_instruction, nullptr);
    EXPECT_EQ(decoded_instruction->mnemonic, "mov");
    EXPECT_EQ(decoded_instruction->length_in_bytes, 5);
    EXPECT_EQ(decoded_instruction->address, start_address);
    // Optionally, check operands if your decoder supports it
}

TEST_F(DecoderTest, DecodeVEXInstruction) {
    Memory memory(1024, 1024, 1024);
    address_t start_address = 0x300;

    // VADDPS ymm0, ymm1, ymm2  (c5 f4 58 c2)
    std::vector<uint8_t> instruction_bytes = {0xc5, 0xf4, 0x58, 0xc2};
    for (size_t i = 0; i < instruction_bytes.size(); ++i) {
        memory.write_text(start_address + i, instruction_bytes[i]);
    }

    auto decoded_instruction = decoder.decodeInstruction(memory, start_address);    
    ASSERT_NE(decoded_instruction, nullptr);
    EXPECT_EQ(decoded_instruction->mnemonic, "vaddps");
    EXPECT_EQ(decoded_instruction->length_in_bytes, 4);
    EXPECT_EQ(decoded_instruction->address, start_address);
    ASSERT_EQ(decoded_instruction->operands.size(), 3);
    EXPECT_EQ(decoded_instruction->operands[0].text, "ymm0");
    EXPECT_EQ(decoded_instruction->operands[1].text, "ymm1"); // This is the non-destructive source from VEX.vvvv
    EXPECT_EQ(decoded_instruction->operands[2].text, "ymm2"); // This is from ModRM.rm
}
