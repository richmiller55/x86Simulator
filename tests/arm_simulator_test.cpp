#include "gtest/gtest.h"
#include "../arm_simulator.h"
#include "../architecture.h"
#include "../arm_to_ir.h"
#include "../ir.h"
#include "../memory.h"
#include "../file_system_device.h"
#include "mock_database_manager.h"
#include "../architecture.h"

class ArmSimulatorCoreTest : public ::testing::Test {
protected:
    MockDatabaseManager dbManager;
    FileSystemDevice fs_device;
    Memory memory;
    Architecture arm_arch;
    std::unique_ptr<ArmToIrConverter> converter;
    std::unique_ptr<ArmSimulator> simulator;

    ArmSimulatorCoreTest()
        : memory(), 
          arm_arch(create_arm_cortex_r8_architecture())
    {
        // Future setup can go here
    }

    void SetUp() override {
        converter = std::make_unique<ArmToIrConverter>(arm_arch, nullptr);
        simulator = std::make_unique<ArmSimulator>(dbManager, memory, 0, true);
    }
};

TEST_F(ArmSimulatorCoreTest, ReadProgram) {

    EXPECT_TRUE(true);
}
