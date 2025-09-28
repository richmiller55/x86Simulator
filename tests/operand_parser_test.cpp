#include "gtest/gtest.h"
#include "../operand_parser.h"
#include <vector>
#include <string>

TEST(OperandParserTest, NoOperands) {
    std::vector<std::string> tokens = {"nop"};
    OperandParser parser(tokens);
    EXPECT_EQ(parser.operand_count(), 0);
}

TEST(OperandParserTest, OneOperand) {
    std::vector<std::string> tokens = {"jmp", "my_label"};
    OperandParser parser(tokens);
    EXPECT_EQ(parser.operand_count(), 1);
    EXPECT_EQ(parser.get_operand(0), "my_label");
}

TEST(OperandParserTest, TwoOperands) {
    std::vector<std::string> tokens = {"mov", "eax", ",", "ebx"};
    OperandParser parser(tokens);
    EXPECT_EQ(parser.operand_count(), 2);
    EXPECT_EQ(parser.get_operand(0), "eax");
    EXPECT_EQ(parser.get_operand(1), "ebx");
}

TEST(OperandParserTest, MixedCaseOperands) {
    std::vector<std::string> tokens = {"MOV", "EAX", ",", "EBX"};
    OperandParser parser(tokens);
    EXPECT_EQ(parser.operand_count(), 2);
    EXPECT_EQ(parser.get_operand(0), "eax");
    EXPECT_EQ(parser.get_operand(1), "ebx");
}

TEST(OperandParserTest, PushInstruction) {
    std::vector<std::string> tokens = {"push", "eax"};
    OperandParser parser(tokens);
    EXPECT_EQ(parser.operand_count(), 1);
    EXPECT_EQ(parser.get_operand(0), "eax");
}

TEST(OperandParserTest, PopInstruction) {
    std::vector<std::string> tokens = {"pop", "ebp"};
    OperandParser parser(tokens);
    EXPECT_EQ(parser.operand_count(), 1);
    EXPECT_EQ(parser.get_operand(0), "ebp");
}

TEST(OperandParserTest, ThreeOperandsVEX) {
    std::vector<std::string> tokens = {"vaddps", "ymm0", ",", "ymm1", ",", "ymm2"};
    OperandParser parser(tokens);
    EXPECT_EQ(parser.operand_count(), 3);
    EXPECT_EQ(parser.get_operand(0), "ymm0");
    EXPECT_EQ(parser.get_operand(1), "ymm1");
    EXPECT_EQ(parser.get_operand(2), "ymm2");
}

TEST(OperandParserTest, InInstruction) {
    std::vector<std::string> tokens = {"in", "al", ",", "0x60"};
    OperandParser parser(tokens);
    EXPECT_EQ(parser.operand_count(), 2);
    EXPECT_EQ(parser.get_operand(0), "al");
    EXPECT_EQ(parser.get_operand(1), "0x60");
}

TEST(OperandParserTest, OutInstruction) {
    std::vector<std::string> tokens = {"out", "0x61", ",", "al"};
    OperandParser parser(tokens);
    EXPECT_EQ(parser.operand_count(), 2);
    EXPECT_EQ(parser.get_operand(0), "0x61");
    EXPECT_EQ(parser.get_operand(1), "al");
}
