#ifndef ARM_UI_MANAGER_H
#define ARM_UI_MANAGER_H

#include "memory.h"
#include "i_register_map.h"
#include "pipeline.h"
#include <ncursesw/curses.h>
#include <string>
#include <vector>
#include <map>

class ProgramDecoder;
class ArmUIManagerTest_TwoUIManagerInstances_Test;

extern bool ncurses_initialized;

class ArmUIManager {
    friend class ArmUIManagerTest_WindowPointersAfterReinitialization_Test;
public:
    ArmUIManager(const Memory& memory_instance);
    ~ArmUIManager();

    void setRegisterMap(const IRegisterMap* regs);
    void setPipeline(const Pipeline* pipeline);
    void setSymbolTable(const std::map<std::string, address_t>* symbol_table);
    void setProgramDecoder(ProgramDecoder* decoder);

    void refreshAll();
    bool waitForInput();
    void draw(address_t current_pc);

    std::string last_drawn_label; // For testing

private:
    void arrangeWindows();
    void drawGprWindow();
    void drawPipelineWindow();
    void drawTextWindow(address_t current_pc);
    void drawInstructionDescription(address_t current_pc);
    void drawLegend();

    WINDOW* win_gpr_;
    WINDOW* win_text_segment_;
    WINDOW* win_instruction_description_;
    WINDOW* win_pipeline_;
    WINDOW* win_legend_;

    const Memory& memory_;
    const IRegisterMap* current_regs_;
    const Pipeline* pipeline_;
    const std::map<std::string, address_t>* symbol_table_;
    ProgramDecoder* program_decoder_;

    size_t text_scroll_offset_;
    std::map<address_t, std::string> address_to_label_;
    bool show_labels_in_text_segment_;
};

#endif // ARM_UI_MANAGER_H
