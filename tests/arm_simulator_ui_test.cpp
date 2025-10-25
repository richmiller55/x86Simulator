#include "gtest/gtest.h"
#include "../arm_simulator.h"
#include "../i_database_manager.h"
#include "../memory.h"
#include <fstream>

class MockDatabaseManager : public IDatabaseManager {
public:
    void logEvent(int session_id, const std::string& event_type, const std::string& payload) override {}
    int createSession(const std::string& programName) override { return 1; }
    void saveSnapshot(int session_id, const std::string& snapshotData) override {}
    void log(int session_id, const std::string& message, const std::string& level,
             uint64_t instruction_pointer, const std::string& source_file,
             int source_line) override {}
};

TEST(ArmSimulatorUITest, SymbolTableIsPassedToUI) {
    MockDatabaseManager db_manager;
    Memory memory;
    
    // Create a dummy assembly file
    std::ofstream asm_file("test_program.asm");
    asm_file << "label1:\n";
    asm_file << "mov r0, #1";
    asm_file.close();

    // Create a non-headless simulator to have a UI manager
    ArmSimulator simulator(db_manager, memory, 1, false);
    simulator.loadProgram("test_program.asm");

    // The ArmToIrConverter creates the symbol table, but the ArmSimulator does not pass it to the ArmUIManager.
    // This test will fail because the symbol table is never set in the UI manager.

    // To test this, we need to run the simulator for one step to trigger the draw call.
    // The runProgram method has a loop that waits for input, so we can't call it directly in the test.
    // We will call the draw method directly.

    ArmUIManager* ui = simulator.getUIManager();
    ASSERT_NE(ui, nullptr);

    // The first instruction is at address 0, which corresponds to the label "label1".
    // The address is the line number, and the first line is empty in the assembly string.
    // So the label is at line 1.
    ui->draw(1);

    EXPECT_EQ(ui->last_drawn_label, "label1");
}