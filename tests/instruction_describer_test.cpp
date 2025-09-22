#include "gtest/gtest.h"
#include "../instruction_describer.h"
#include "../decoder.h"
#include "../register_map.h"
#include "../memory.h"

class InstructionDescriberTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    RegisterMap regs;
    Decoder& decoder = Decoder::getInstance();
    Memory memory{1024, 1024, 1024};
};

TEST_F(InstructionDescriberTest, DescribeNOP) {
    address_t addr = memory.get_text_segment_start();
    memory.write_text(addr, 0x90); // NOP
    auto decoded_instr = decoder.decodeInstruction(memory, addr);
    ASSERT_NE(decoded_instr, nullptr);

    std::string description = InstructionDescriber::describe(*decoded_instr, regs);    
    EXPECT_EQ(description, "No operation. This instruction does nothing.");
}

TEST_F(InstructionDescriberTest, DescribeMOV) {
    address_t addr = memory.get_text_segment_start();
    // MOV EAX, 0x1234
    memory.write_text(addr, 0xB8);
    memory.write_text_dword(addr + 1, 0x1234);
    auto decoded_instr = decoder.decodeInstruction(memory, addr);
    ASSERT_NE(decoded_instr, nullptr);

    std::string expected_description = "Moves the value from 0x1234 to eax.";
    std::string description = InstructionDescriber::describe(*decoded_instr, regs);
    EXPECT_EQ(description, expected_description);
}
