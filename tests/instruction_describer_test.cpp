#include "gtest/gtest.h"
#include "../instruction_describer.h"
#include "../decoder.h"
#include "../register_map.h"
#include "../memory.h"

class InstructionDescriberTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize registers if needed
    }

    RegisterMap regs;
    Decoder& decoder = Decoder::getInstance();
    Memory memory{1024, 1024, 1024};
};

TEST_F(InstructionDescriberTest, DescribeNOP) {
    address_t addr = memory.get_text_segment_start();
    memory.write_text(addr, 0x90); // NOP
    auto decoded_instr = decoder.decodeInstruction(memory, addr);
    ASSERT_TRUE(decoded_instr.has_value());

    std::string description = InstructionDescriber::describe(*decoded_instr, regs);
    EXPECT_EQ(description, "Mnemonic: NOP. No detailed description available yet.");
}

TEST_F(InstructionDescriberTest, DescribeMOV) {
    address_t addr = memory.get_text_segment_start();
    // MOV EAX, 0x1234
    memory.write_text(addr, 0xB8);
    memory.write_text_dword(addr + 1, 0x1234);
    auto decoded_instr = decoder.decodeInstruction(memory, addr);
    ASSERT_TRUE(decoded_instr.has_value());

    std::string expected_description = "Mnemonic: MOV. No detailed description available yet.";
    std::string description = InstructionDescriber::describe(*decoded_instr, regs);
    EXPECT_EQ(description, expected_description);
}
