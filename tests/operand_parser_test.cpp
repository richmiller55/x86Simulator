#include "gtest/gtest.h"
#include "../operand_parser.h"
#include <vector>
#include <string>

TEST(OperandParserTest, NoOperands) {
    std::vector<std::string> tokens = {"NOP"};
    OperandParser parser(tokens);
    EXPECT_EQ(parser.operand_count(), 0);
}

TEST(OperandParserTest, OneOperand) {
    std::vector<std::string> tokens = {"JMP", "0x1234"};
    OperandParser parser(tokens);
    EXPECT_EQ(parser.operand_count(), 1);
    EXPECT_EQ(parser.get_operand(0), "0x1234");
}

TEST(OperandParserTest, TwoOperands) {
    std::vector<std::string> tokens = {"MOV", "EAX,", "EBX"};
    OperandParser parser(tokens);
    EXPECT_EQ(parser.operand_count(), 2);
    EXPECT_EQ(parser.get_operand(0), "eax");
    EXPECT_EQ(parser.get_operand(1), "ebx");
}

TEST(OperandParserTest, TwoOperandsWithWhitespace) {
    std::vector<std::string> tokens = {"MOV", "  EAX  ,", "  EBX  "};
    OperandParser parser(tokens);
    EXPECT_EQ(parser.operand_count(), 2);
    EXPECT_EQ(parser.get_operand(0), "eax");
    EXPECT_EQ(parser.get_operand(1), "ebx");
}

TEST(OperandParserTest, MixedCaseOperands) {
    std::vector<std::string> tokens = {"mov", "eAx,", "eBx"};
    OperandParser parser(tokens);
    EXPECT_EQ(parser.operand_count(), 2);
    EXPECT_EQ(parser.get_operand(0), "eax");
    EXPECT_EQ(parser.get_operand(1), "ebx");
}

TEST(OperandParserTest, CombinedOperands) {
    std::vector<std::string> tokens = {"ADD", "EAX,EBX"};
    OperandParser parser(tokens);
    EXPECT_EQ(parser.operand_count(), 2);
    EXPECT_EQ(parser.get_operand(0), "eax");
    EXPECT_EQ(parser.get_operand(1), "ebx");
}
