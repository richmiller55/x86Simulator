
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
