#include <gtest/gtest.h>
#include "../arm_ui_manager.h"
#include "../memory.h"
#include "../i_register_map.h"
#include <vector>
#include <string>

// Simple mock for IRegisterMap
class MockRegisterMap : public IRegisterMap {
public:
    void getValue(const std::string& reg_name, void* dest_buffer, size_t size_bytes) const override {
        if (reg_name == "pc") {
            uint32_t val = 0x1234;
            *(uint32_t*)dest_buffer = val;
        } else {
            *(uint32_t*)dest_buffer = 0;
        }
    }
    void setValue(const std::string& reg_name, const void* src_buffer, size_t size_bytes) override {
        // Do nothing
    }
};


TEST(ArmUIManagerTest, Constructor) {
    Memory memory;
    // ArmUIManager ui(memory);
}

TEST(ArmUIManagerTest, DrawCycle) {
    Memory memory;
    ArmUIManager ui(memory);
    MockRegisterMap regs;
    
    ui.setRegisterMap(&regs);
    
    ui.draw(0x1234);
}

TEST(ArmUIManagerTest, WindowPointersAfterReinitialization) {
    Memory memory;
    
    // First instance
    {
        ArmUIManager ui1(memory);
        // Use friend access to check internal state
        EXPECT_NE(ui1.win_gpr_, nullptr);
        EXPECT_NE(ui1.win_text_segment_, nullptr);
        EXPECT_NE(ui1.win_instruction_description_, nullptr);
        EXPECT_NE(ui1.win_pipeline_, nullptr);
        EXPECT_NE(ui1.win_legend_, nullptr);

        MockRegisterMap regs1;
        ui1.setRegisterMap(&regs1);
        ui1.draw(0);
    }
    // ui1 is destroyed here, calling endwin()

    // Second instance
    {
        ArmUIManager ui2(memory);
        // After endwin() and initscr() again, are the windows valid?
        EXPECT_NE(ui2.win_gpr_, nullptr);
        EXPECT_NE(ui2.win_text_segment_, nullptr);
        EXPECT_NE(ui2.win_instruction_description_, nullptr);
        EXPECT_NE(ui2.win_pipeline_, nullptr);
        EXPECT_NE(ui2.win_legend_, nullptr);

        MockRegisterMap regs2;
        ui2.setRegisterMap(&regs2);
        ui2.draw(0);
    }
}