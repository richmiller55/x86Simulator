#include "gtest/gtest.h"
#include "../file_system_device.h"

class FileSystemDeviceTest : public ::testing::Test {
protected:
    FileSystemDevice fs_device;

    FileSystemDeviceTest() : fs_device("") {} // Disable persistence for tests
};

TEST_F(FileSystemDeviceTest, AppendToNewFile) {
    fs_device.appendToFile("/root/test.txt", 'a');
    const std::vector<std::string>* content = fs_device.getFileContent("/root/test.txt");
    ASSERT_NE(content, nullptr);
    ASSERT_EQ(content->size(), 1);
    EXPECT_EQ((*content)[0], "a");
}

TEST_F(FileSystemDeviceTest, AppendToExistingFile) {
    fs_device.appendToFile("/root/test.txt", 'a');
    fs_device.appendToFile("/root/test.txt", 'b');
    const std::vector<std::string>* content = fs_device.getFileContent("/root/test.txt");
    ASSERT_NE(content, nullptr);
    ASSERT_EQ(content->size(), 1);
    EXPECT_EQ((*content)[0], "ab");
}

TEST_F(FileSystemDeviceTest, AppendWithNewline) {
    fs_device.appendToFile("/root/test.txt", 'a');
    fs_device.appendToFile("/root/test.txt", '\n');
    fs_device.appendToFile("/root/test.txt", 'b');
    const std::vector<std::string>* content = fs_device.getFileContent("/root/test.txt");
    ASSERT_NE(content, nullptr);
    ASSERT_EQ(content->size(), 2);
    EXPECT_EQ((*content)[0], "a");
    EXPECT_EQ((*content)[1], "b");
}

TEST_F(FileSystemDeviceTest, GetContentOfNonExistentFile) {
    const std::vector<std::string>* content = fs_device.getFileContent("/root/nonexistent.txt");
    EXPECT_EQ(content, nullptr);
}
