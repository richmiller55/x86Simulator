#include "gtest/gtest.h"
#include "../arm_to_ir.h"
#include "../architecture.h"



TEST(ArmToIrConverterTest, TranslateSWI) {
    Architecture arch = create_arm_cortex_r8_architecture();
    ArmToIrConverter converter(arch, nullptr);
    std::string assembly = "swi 0";
    IRProgram program = converter.convert(assembly);
    ASSERT_EQ(program.size(), 1);
    EXPECT_EQ(program[0]->opcode, IROpcode::Syscall);
    ASSERT_EQ(program[0]->operands.size(), 1);
    EXPECT_TRUE(std::holds_alternative<uint64_t>(program[0]->operands[0]));
    EXPECT_EQ(std::get<uint64_t>(program[0]->operands[0]), 0);
}

TEST(ArmToIrConverterTest, TranslateSWIHex) {
    Architecture arch = create_arm_cortex_r8_architecture();
    ArmToIrConverter converter(arch, nullptr);
    std::string assembly = "swi 0x1A";
    IRProgram program = converter.convert(assembly);
    ASSERT_EQ(program.size(), 1);
    EXPECT_EQ(program[0]->opcode, IROpcode::Syscall);
    ASSERT_EQ(program[0]->operands.size(), 1);
    EXPECT_TRUE(std::holds_alternative<uint64_t>(program[0]->operands[0]));
    EXPECT_EQ(std::get<uint64_t>(program[0]->operands[0]), 0x1A);
}

TEST(ArmToIrConverterTest, TranslateDataProcessingImmediate) {
    Architecture arch = create_arm_cortex_r8_architecture();
    ArmToIrConverter converter(arch, nullptr);
    std::string assembly = "add r0, r1, #123";
    IRProgram program = converter.convert(assembly);
    ASSERT_EQ(program.size(), 1);
    EXPECT_EQ(program[0]->opcode, IROpcode::Add);
    ASSERT_EQ(program[0]->operands.size(), 3);
    EXPECT_TRUE(std::holds_alternative<std::string>(program[0]->operands[0]));
    EXPECT_EQ(std::get<std::string>(program[0]->operands[0]), "r0");
    EXPECT_TRUE(std::holds_alternative<std::string>(program[0]->operands[1]));
    EXPECT_EQ(std::get<std::string>(program[0]->operands[1]), "r1");
    EXPECT_TRUE(std::holds_alternative<uint64_t>(program[0]->operands[2]));
    EXPECT_EQ(std::get<uint64_t>(program[0]->operands[2]), 123);
}
