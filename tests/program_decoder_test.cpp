#include "gtest/gtest.h"
#include "../program_decoder.h"
#include "../memory.h"

TEST(ProgramDecoderTest, DecodeSimpleProgram) {
    Memory mem;
    // mov eax, 5
    mem.write_text(mem.get_text_segment_start() + 0, 0xb8);
    mem.write_text(mem.get_text_segment_start() + 1, 0x05);
    mem.write_text(mem.get_text_segment_start() + 2, 0x00);
    mem.write_text(mem.get_text_segment_start() + 3, 0x00);
    mem.write_text(mem.get_text_segment_start() + 4, 0x00);
    // nop
    mem.write_text(mem.get_text_segment_start() + 5, 0x90);
    mem.set_text_segment_size(5 + 1); // mov is 5 bytes, nop is 1 byte

    ProgramDecoder decoder(mem);
    decoder.decode();

    const auto& decoded_program = decoder.getDecodedProgram();
    ASSERT_EQ(decoded_program.size(), 2);

    EXPECT_EQ(decoded_program[0]->mnemonic, "mov");
    EXPECT_EQ(decoded_program[0]->length_in_bytes, 5);

    EXPECT_EQ(decoded_program[1]->mnemonic, "nop");
    EXPECT_EQ(decoded_program[1]->length_in_bytes, 1);

    const auto& address_map = decoder.getAddressToIndexMap();
    ASSERT_EQ(address_map.size(), 2);
    EXPECT_EQ(address_map.at(mem.get_text_segment_start() + 0), 0);
    EXPECT_EQ(address_map.at(mem.get_text_segment_start() + 5), 1);
}

TEST(ProgramDecoderTest, DecodePushPopProgram) {
    Memory mem;
    address_t start_addr = mem.get_text_segment_start();
    size_t current_offset = 0;

    // mov eax, 0x1234
    mem.write_text(start_addr + current_offset++, 0xb8);
    mem.write_text(start_addr + current_offset++, 0x34);
    mem.write_text(start_addr + current_offset++, 0x12);
    mem.write_text(start_addr + current_offset++, 0x00);
    mem.write_text(start_addr + current_offset++, 0x00);

    // push eax
    mem.write_text(start_addr + current_offset++, 0x50);

    // pop ebx
    mem.write_text(start_addr + current_offset++, 0x5b);

    mem.set_text_segment_size(current_offset);

    ProgramDecoder decoder(mem);
    decoder.decode();

    const auto& decoded_program = decoder.getDecodedProgram();
    ASSERT_EQ(decoded_program.size(), 3);

    // Instruction 1: mov eax, 0x1234
    EXPECT_EQ(decoded_program[0]->mnemonic, "mov");
    EXPECT_EQ(decoded_program[0]->length_in_bytes, 5);
    ASSERT_EQ(decoded_program[0]->operands.size(), 2);
    EXPECT_EQ(decoded_program[0]->operands[0].text, "eax");
    EXPECT_EQ(decoded_program[0]->operands[1].value, 0x1234);

    // Instruction 2: push eax
    EXPECT_EQ(decoded_program[1]->mnemonic, "push");
    EXPECT_EQ(decoded_program[1]->length_in_bytes, 1);
    ASSERT_EQ(decoded_program[1]->operands.size(), 1);
    EXPECT_EQ(decoded_program[1]->operands[0].text, "eax");

    // Instruction 3: pop ebx
    EXPECT_EQ(decoded_program[2]->mnemonic, "pop");
    EXPECT_EQ(decoded_program[2]->length_in_bytes, 1);
    ASSERT_EQ(decoded_program[2]->operands.size(), 1);
    EXPECT_EQ(decoded_program[2]->operands[0].text, "ebx");

    const auto& address_map = decoder.getAddressToIndexMap();
    ASSERT_EQ(address_map.size(), 3);
    EXPECT_EQ(address_map.at(start_addr + 0), 0); // mov
    EXPECT_EQ(address_map.at(start_addr + 5), 1); // push
    EXPECT_EQ(address_map.at(start_addr + 6), 2); // pop
}

TEST(ProgramDecoderTest, DecodeVEXThreeOperandInstruction) {
    Memory mem;
    address_t start_addr = mem.get_text_segment_start();
    size_t current_offset = 0;

    // vaddps ymm0, ymm1, ymm2
    // VEX(2-byte).L.vvvv.pp = VEX.256.ymm1.0F
    // Opcode: 58
    // ModR/M: ymm0, ymm2
    // C5 F5 58 C2
    mem.write_text(start_addr + current_offset++, 0xC5);
    mem.write_text(start_addr + current_offset++, 0xF5); // VEX byte 2: R=~0, vvvv=~1, L=1, pp=01
    mem.write_text(start_addr + current_offset++, 0x58); // Opcode
    mem.write_text(start_addr + current_offset++, 0xC2); // ModR/M: mod=11, reg=0 (ymm0), rm=2 (ymm2)

    mem.set_text_segment_size(current_offset);

    ProgramDecoder decoder(mem);
    decoder.decode();

    const auto& decoded_program = decoder.getDecodedProgram();
    ASSERT_EQ(decoded_program.size(), 1);

    const auto& instr = decoded_program[0];
    EXPECT_EQ(instr->mnemonic, "vaddps");
    EXPECT_EQ(instr->length_in_bytes, 4);
    ASSERT_EQ(instr->operands.size(), 3);
    EXPECT_EQ(instr->operands[0].text, "ymm0"); // dest
    EXPECT_EQ(instr->operands[1].text, "ymm1"); // src1 (from vvvv)
    EXPECT_EQ(instr->operands[2].text, "ymm2"); // src2 (from rm)
}

TEST(ProgramDecoderTest, DecodeVEXTwoOperandInstruction) {
    Memory mem;
    address_t start_addr = mem.get_text_segment_start();
    size_t current_offset = 0;

    // vrcpps ymm0, ymm1
    // VEX.L.vvvv.pp = VEX.256.1111B.0F
    // Opcode: 53
    // ModR/M: ymm0, ymm1
    // C5 85 53 C1
    mem.write_text(start_addr + current_offset++, 0xC5);
    mem.write_text(start_addr + current_offset++, 0x85); // VEX byte 2: R=~0, vvvv=~15, L=1, pp=01
    mem.write_text(start_addr + current_offset++, 0x53); // Opcode
    mem.write_text(start_addr + current_offset++, 0xC1); // ModR/M: mod=11, reg=0 (ymm0), rm=1 (ymm1)

    mem.set_text_segment_size(current_offset);

    ProgramDecoder decoder(mem);
    decoder.decode();

    const auto& decoded_program = decoder.getDecodedProgram();
    ASSERT_EQ(decoded_program.size(), 1);

    const auto& instr = decoded_program[0];
    EXPECT_EQ(instr->mnemonic, "vrcpps");
    EXPECT_EQ(instr->length_in_bytes, 4);
    ASSERT_EQ(instr->operands.size(), 2);
    EXPECT_EQ(instr->operands[0].text, "ymm0"); // dest
    EXPECT_EQ(instr->operands[1].text, "ymm1"); // src
}

TEST(ProgramDecoderTest, DecodeVMOVUPSLoad) {
    Memory mem;
    address_t start_addr = mem.get_text_segment_start();
    size_t current_offset = 0;

    // vmovups ymm0, [rip+0x100]
    // C5 F8 10 05 00 01 00 00
    mem.write_text(start_addr + current_offset++, 0xC5);
    mem.write_text(start_addr + current_offset++, 0xFD); // VEX byte 2: R=~0, vvvv=~0, L=1, pp=01
    mem.write_text(start_addr + current_offset++, 0x10); // Opcode
    mem.write_text(start_addr + current_offset++, 0x05); // ModR/M: mod=00, reg=0 (ymm0), rm=101 (rip+disp)
    mem.write_text_dword(start_addr + current_offset, 0x100);
    current_offset += 4;

    mem.set_text_segment_size(current_offset);

    ProgramDecoder decoder(mem);
    decoder.decode();

    const auto& decoded_program = decoder.getDecodedProgram();
    ASSERT_EQ(decoded_program.size(), 1);

    const auto& instr = decoded_program[0];
    EXPECT_EQ(instr->mnemonic, "vmovups");
    EXPECT_EQ(instr->length_in_bytes, 2 + 1 + 1 + 4);
    ASSERT_EQ(instr->operands.size(), 2);
    EXPECT_EQ(instr->operands[0].text, "ymm0"); // dest
    EXPECT_EQ(instr->operands[0].type, OperandType::YMM_REGISTER);
    EXPECT_EQ(instr->operands[1].type, OperandType::MEMORY);
    address_t expected_addr = start_addr + 8 + 0x100;
    EXPECT_EQ(instr->operands[1].value, expected_addr);
}
