#include "arm_ui_manager.h"
#include "i_register_map.h"
#include "pipeline.h"
#include "program_decoder.h"
#include "instruction_describer.h"
#include <sstream>
#include <iomanip>
#include <ncursesw/curses.h>



// ARM Register Display Order
const std::vector<std::string> RegisterDisplayOrderARM = {
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
    "r8", "r9", "r10", "r11", "r12", "sp", "lr", "pc", "cpsr"
};

bool ncurses_initialized = false;

ArmUIManager::ArmUIManager(const Memory& memory_instance)
: win_gpr_(nullptr),
  win_text_segment_(nullptr),
  win_instruction_description_(nullptr),
  win_pipeline_(nullptr),
  win_legend_(nullptr),
  memory_(memory_instance),
  current_regs_(nullptr),
  pipeline_(nullptr),
  symbol_table_(nullptr),
  program_decoder_(nullptr),
  text_scroll_offset_(0),
  show_labels_in_text_segment_(false)
{
    if (ncurses_initialized) {
        throw std::runtime_error("Cannot create a second UI manager.");
    }

    initscr();
    ncurses_initialized = true;

    win_gpr_ = newwin(1, 1, 1, 1);
    win_text_segment_ = newwin(1, 1, 1, 1);
    win_instruction_description_ = newwin(1, 1, 1, 1);
    win_pipeline_ = newwin(1, 1, 1, 1);
    win_legend_ = newwin(1, 1, 1, 1);
    arrangeWindows();
}

ArmUIManager::~ArmUIManager() {
    delwin(win_gpr_);
    delwin(win_text_segment_);
    delwin(win_instruction_description_);
    delwin(win_pipeline_);
    delwin(win_legend_);

    if (ncurses_initialized) {
        endwin();
        ncurses_initialized = false;
    }
}

void ArmUIManager::arrangeWindows() {
    int height, width;
    getmaxyx(stdscr, height, width);

    int reg_width = 30;
    int text_width = 40;
    int pipeline_width = width - reg_width - text_width - 4;

    mvwin(win_gpr_, 1, 1);
    wresize(win_gpr_, height - 4, reg_width);

    mvwin(win_text_segment_, 1, reg_width + 2);
    wresize(win_text_segment_, height - 4, text_width);

    mvwin(win_pipeline_, 1, reg_width + text_width + 3);
    wresize(win_pipeline_, height / 2, pipeline_width);

    mvwin(win_instruction_description_, (height / 2) + 2, reg_width + text_width + 3);
    wresize(win_instruction_description_, height / 2 - 5, pipeline_width);

    mvwin(win_legend_, height - 2, 1);
    wresize(win_legend_, 3, width - 2);

    clear();
    refresh();
}

void ArmUIManager::setRegisterMap(const IRegisterMap* regs) {
    current_regs_ = regs;
}

void ArmUIManager::setPipeline(const Pipeline* pipeline) {
    pipeline_ = pipeline;
}

void ArmUIManager::setSymbolTable(const std::map<std::string, address_t>* symbol_table) {
    symbol_table_ = symbol_table;
    if (symbol_table_) {
        address_to_label_.clear();
        for (const auto& pair : *symbol_table_) {
            address_to_label_[pair.second] = pair.first;
        }
    }
}

void ArmUIManager::setProgramDecoder(ProgramDecoder* decoder) {
    program_decoder_ = decoder;
}

void ArmUIManager::draw(address_t current_pc) {
    if (!current_regs_) return;
    drawGprWindow();
    drawTextWindow(current_pc);
    drawPipelineWindow();
    drawInstructionDescription(current_pc);
    drawLegend();
    refreshAll();
}

void ArmUIManager::drawGprWindow() {
    werase(win_gpr_);
    box(win_gpr_, 0, 0);
    mvwprintw(win_gpr_, 1, 2, "--- ARM Registers ---");

    int row = 2;
    for (const auto& regName : RegisterDisplayOrderARM) {
        std::stringstream ss;
        ss << std::left << std::setw(5) << regName << ": ";
        uint32_t regValue = current_regs_->get32(regName);
        ss << "0x" << std::hex << std::setfill('0') << std::right << std::setw(8) << regValue;

        if (regName == "pc" || regName == "sp" || regName == "lr") {
            wattron(win_gpr_, COLOR_PAIR(2));
        }
        else {
            wattron(win_gpr_, COLOR_PAIR(1));
        }
        mvwprintw(win_gpr_, row++, 2, "%s", ss.str().c_str());
        wattroff(win_gpr_, A_COLOR);
    }
}

void ArmUIManager::drawPipelineWindow() {
    werase(win_pipeline_);
    box(win_pipeline_, 0, 0);
    mvwprintw(win_pipeline_, 1, 2, "--- Multi-Channel Pipeline ---");

    const std::vector<std::string> stages = {"Fetch", "Decode", "Int ALU", "FPU", "Branch", "L/S Unit", "Memory", "Writeback"};
    int row = 2;
    int col = 10;
    for (const auto& stage : stages) {
        mvwprintw(win_pipeline_, row++, 2, "%s", stage.c_str());
    }

    for (int i = 0; i < 10; ++i) { 
        mvwprintw(win_pipeline_, 1, col + i * 6, "C%d", i);
    }

    if (pipeline_) {
        mvwprintw(win_pipeline_, row + 1, 2, "Pipeline visualization pending data structure.");
    }
}

void ArmUIManager::drawTextWindow(address_t current_pc) {
    werase(win_text_segment_);
    box(win_text_segment_, 0, 0);
    mvwprintw(win_text_segment_, 1, 2, "--- Program ---");

    if (address_to_label_.count(current_pc)) {
        last_drawn_label = address_to_label_.at(current_pc);
    }

    mvwprintw(win_text_segment_, 2, 2, "Text segment view for ARM.");
}

void ArmUIManager::drawInstructionDescription(address_t current_pc) {
    werase(win_instruction_description_);
    box(win_instruction_description_, 0, 0);
    mvwprintw(win_instruction_description_, 1, 2, "--- Instruction Details ---");
    mvwprintw(win_instruction_description_, 2, 2, "Instruction description for ARM.");
}

void ArmUIManager::drawLegend() {
    werase(win_legend_);
    box(win_legend_, 0, 0);
    mvwprintw(win_legend_, 1, 2, "n: step | q: quit");
}

void ArmUIManager::refreshAll() {
    wnoutrefresh(win_gpr_);
    wnoutrefresh(win_text_segment_);
    wnoutrefresh(win_pipeline_);
    wnoutrefresh(win_instruction_description_);
    wnoutrefresh(win_legend_);
    doupdate();
}

bool ArmUIManager::waitForInput() {
    int ch = getch();
    switch (ch) {
        case 'q':
            return false;
        case 'n':
            return true;
        default:
            return true;
    }
}
