#include "gtest/gtest.h"
#include "../parser_utils.h"
#include <vector>
#include <string>

TEST(ParserUtilsTest, ParseLineBasic) {
    std::string line = "mov eax, 1";
    std::vector<std::string> expected = {"mov", "eax", ",", "1"};
    std::vector<std::string> actual = parse_line(line);
    EXPECT_EQ(actual, expected);
}

TEST(ParserUtilsTest, ParseLineWithTabs) {
    std::string line = "mov\teax, 1";
    std::vector<std::string> expected = {"mov", "eax", ",", "1"};
    std::vector<std::string> actual = parse_line(line);
    EXPECT_EQ(actual, expected);
}

TEST(ParserUtilsTest, ParseLineWithMultipleSpaces) {
    std::string line = "mov  eax,   1";
    std::vector<std::string> expected = {"mov", "eax", ",", "1"};
    std::vector<std::string> actual = parse_line(line);
    EXPECT_EQ(actual, expected);
}

TEST(ParserUtilsTest, ParseLineWithLeadingTrailingSpaces) {
    std::string line = "  mov eax, 1  ";
    std::vector<std::string> expected = {"mov", "eax", ",", "1"};
    std::vector<std::string> actual = parse_line(line);
    EXPECT_EQ(actual, expected);
}

TEST(ParserUtilsTest, ParseLineDataDirective) {
    std::string line = "my_var db 'a', 10, 0xFF";
    std::vector<std::string> expected = {"my_var", "db", "'a'", ",", "10", ",", "0xFF"};
    std::vector<std::string> actual = parse_line(line);
    EXPECT_EQ(actual, expected);
}

TEST(ParserUtilsTest, ParseLineLabel) {
    std::string line = "my_label: mov eax, 1";
    std::vector<std::string> expected = {"my_label:", "mov", "eax", ",", "1"};
    std::vector<std::string> actual = parse_line(line);
    EXPECT_EQ(actual, expected);
}

TEST(ParserUtilsTest, ParseLineEmpty) {
    std::string line = "";
    std::vector<std::string> expected = {};
    std::vector<std::string> actual = parse_line(line);
    EXPECT_EQ(actual, expected);
}

TEST(ParserUtilsTest, ParseLineWhitespaceOnly) {
    std::string line = "   \t  ";
    std::vector<std::string> expected = {};
    std::vector<std::string> actual = parse_line(line);
    EXPECT_EQ(actual, expected);
}
