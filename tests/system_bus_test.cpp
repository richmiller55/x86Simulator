#include "gtest/gtest.h"
#include "../system_bus.h"
#include "mock_database_manager.h"
#include <fstream>

class SystemBusTest : public ::testing::Test {
protected:
    MockDatabaseManager dbManager;
    SystemBus systemBus;

    SystemBusTest() : systemBus(dbManager) {}

    void TearDown() override {
        std::remove("test_config_ui.json");
        std::remove("test_config_headless.json");
        std::remove("test_config_malformed.json");
    }
};

TEST_F(SystemBusTest, LoadsConfigurationAndSetsUI) {
    std::ofstream config_file("test_config_ui.json");
    config_file << R"({"ui_enabled": true, "processes": [{"path": "test.asm"}]})";
    config_file.close();

    systemBus.load_configuration("test_config_ui.json");
    ASSERT_EQ(systemBus.get_process_count(), 1);
    const X86Simulator* sim = systemBus.get_process(0);
    ASSERT_NE(sim, nullptr);
    EXPECT_FALSE(sim->is_headless());
}

TEST_F(SystemBusTest, LoadsConfigurationAndSetsHeadless) {
    std::ofstream config_file("test_config_headless.json");
    config_file << R"({"ui_enabled": false, "processes": [{"path": "test.asm"}]})";
    config_file.close();

    systemBus.load_configuration("test_config_headless.json");
    ASSERT_EQ(systemBus.get_process_count(), 1);
    const X86Simulator* sim = systemBus.get_process(0);
    ASSERT_NE(sim, nullptr);
    EXPECT_TRUE(sim->is_headless());
}

TEST_F(SystemBusTest, HandlesMissingFile) {
    systemBus.load_configuration("non_existent_file.json");
    EXPECT_EQ(systemBus.get_process_count(), 0);
}

TEST_F(SystemBusTest, HandlesMalformedFile) {
    std::ofstream config_file("test_config_malformed.json");
    config_file << R"({"ui_enabled": true, "processes": })"; // Malformed JSON
    config_file.close();

    EXPECT_NO_THROW(systemBus.load_configuration("test_config_malformed.json"));
    EXPECT_EQ(systemBus.get_process_count(), 0);
}
