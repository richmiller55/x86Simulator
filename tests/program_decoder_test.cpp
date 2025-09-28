#include "gtest/gtest.h"
#include "../program_decoder.h"
#include "../memory.h"
#include "../CodeGenerator.h"
#include <map>

// Helper function to trim strings, needed for the test
std::string trim_for_test(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (std::string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}

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

TEST(ProgramDecoderTest, AssembleAndDecodeJumps) {
    std::vector<std::string> program_lines = {
        "start:",
        "  mov ecx, 10",
        "loop:",
        "  cmp ecx, 0",
        "  je end",
        "  dec ecx",
        "  jmp loop",
        "end:",
        "  nop"
    };

    std::map<std::string, address_t> symbol_table;
    address_t current_address = 0;

    // Manual first pass to build symbol table
    for (const auto& line : program_lines) {
        std::string trimmed_line = trim_for_test(line);
        if (trimmed_line.empty() || trimmed_line[0] == ';') continue;

        if (trimmed_line.back() == ':') {
            std::string label = trimmed_line.substr(0, trimmed_line.length() - 1);
            symbol_table[label] = current_address;
        } else {
            std::map<std::string, address_t> empty_symbol_table;
            CodeGenerator temp_gen(empty_symbol_table, current_address);
            std::vector<uint8_t> temp_code = temp_gen.generate_code({trimmed_line});
            current_address += temp_code.size();
        }
    }

    // --- Second Pass: Generate final code ---
    CodeGenerator code_gen(symbol_table, 0);
    std::vector<uint8_t> machine_code = code_gen.generate_code(program_lines);

    // --- Load code into memory and decode ---
    Memory mem;
    mem.set_text_segment_size(machine_code.size());
    for(size_t i = 0; i < machine_code.size(); ++i) {
        mem.write_text(mem.get_text_segment_start() + i, machine_code[i]);
    }

    ProgramDecoder decoder(mem);
    decoder.decode();

    const auto& decoded_program = decoder.getDecodedProgram();
    ASSERT_EQ(decoded_program.size(), 6);

    // --- Verify Decoded Instructions ---
    // 1. mov ecx, 10 (address 0, size 5)
    EXPECT_EQ(decoded_program[0]->mnemonic, "mov");
    EXPECT_EQ(decoded_program[0]->address, 0);

    // 2. cmp ecx, 0 (address 5, size 3)
    EXPECT_EQ(decoded_program[1]->mnemonic, "cmp");
    EXPECT_EQ(decoded_program[1]->address, 5);

    // 3. je end (address 8, size 2)
    const auto& je_instr = decoded_program[2];
    EXPECT_EQ(je_instr->mnemonic, "je");
    EXPECT_EQ(je_instr->address, 8);
    ASSERT_EQ(je_instr->operands.size(), 1);
    EXPECT_EQ(je_instr->operands[0].value, symbol_table["end"]);

    // 4. dec ecx (address 10, size 2)
    EXPECT_EQ(decoded_program[3]->mnemonic, "dec");
    EXPECT_EQ(decoded_program[3]->address, 10);

    // 5. jmp loop (address 12, size 5)
    const auto& jmp_instr = decoded_program[4];
    EXPECT_EQ(jmp_instr->mnemonic, "jmp");
    EXPECT_EQ(jmp_instr->address, 12);
    ASSERT_EQ(jmp_instr->operands.size(), 1);
    EXPECT_EQ(jmp_instr->operands[0].value, symbol_table["loop"]);

    // 6. nop (address 17)
    EXPECT_EQ(decoded_program[5]->mnemonic, "nop");
    EXPECT_EQ(decoded_program[5]->address, 17);
}
