#include "risc_ui_manager.h"
#include "register_map.h"
#include "pipeline.h"
#include "program_decoder.h"
#include "instruction_describer.h"
#include <sstream>
#include <iomanip>
#include <ncursesw/curses.h>

static bool ncurses_initialized_risc = false;

// RISC-V Register Display Order
const std::vector<std::string> RegisterDisplayOrderRISCV = {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6", "pc"
};

RiscUIManager::RiscUIManager(const Memory& memory_instance)
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
    if (!ncurses_initialized_risc) {
        initscr();
        clear();
        refresh();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);

        if (has_colors()) {
            start_color();
            init_pair(1, COLOR_YELLOW, COLOR_BLACK);
            init_pair(2, COLOR_GREEN, COLOR_BLACK);
            init_pair(3, COLOR_BLUE, COLOR_BLACK);
            init_pair(4, COLOR_CYAN, COLOR_BLACK);
            init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
        }
        ncurses_initialized_risc = true;
    }

    win_gpr_ = newwin(1, 1, 1, 1);
    win_text_segment_ = newwin(1, 1, 1, 1);
    win_instruction_description_ = newwin(1, 1, 1, 1);
    win_pipeline_ = newwin(1, 1, 1, 1);
    win_legend_ = newwin(1, 1, 1, 1);
    arrangeWindows();
}

RiscUIManager::~RiscUIManager() {
    delwin(win_gpr_);
    delwin(win_text_segment_);
    delwin(win_instruction_description_);
    delwin(win_pipeline_);
    delwin(win_legend_);
    if (ncurses_initialized_risc) {
        endwin();
        ncurses_initialized_risc = false;
    }
}

void RiscUIManager::arrangeWindows() {
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

void RiscUIManager::setRegisterMap(const RegisterMap* regs) {
    current_regs_ = regs;
}

void RiscUIManager::setPipeline(const Pipeline* pipeline) {
    pipeline_ = pipeline;
}

void RiscUIManager::setSymbolTable(const std::map<std::string, address_t>* symbol_table) {
    symbol_table_ = symbol_table;
    if (symbol_table_) {
        address_to_label_.clear();
        for (const auto& pair : *symbol_table_) {
            address_to_label_[pair.second] = pair.first;
        }
    }
}

void RiscUIManager::setProgramDecoder(ProgramDecoder* decoder) {
    program_decoder_ = decoder;
}

void RiscUIManager::draw(address_t current_pc) {
    if (!current_regs_) return;
    drawGprWindow();
    drawTextWindow(current_pc);
    drawPipelineWindow();
    drawInstructionDescription(current_pc);
    drawLegend();
    refreshAll();
}

void RiscUIManager::drawGprWindow() {
    werase(win_gpr_);
    box(win_gpr_, 0, 0);
    mvwprintw(win_gpr_, 1, 2, "--- RISC-V Registers ---");

    int row = 2;
    for (const auto& regName : RegisterDisplayOrderRISCV) {
        std::stringstream ss;
        ss << std::left << std::setw(5) << regName << ": ";
        uint64_t regValue = current_regs_->get64(regName);
        ss << "0x" << std::hex << std::setfill('0') << std::right << std::setw(16) << regValue;

        if (regName == "pc" || regName == "sp" || regName == "ra") {
            wattron(win_gpr_, COLOR_PAIR(2));
        }
        else {
            wattron(win_gpr_, COLOR_PAIR(1));
        }
        mvwprintw(win_gpr_, row++, 2, "%s", ss.str().c_str());
        wattroff(win_gpr_, A_COLOR);
    }
}

void RiscUIManager::drawPipelineWindow() {
    werase(win_pipeline_);
    box(win_pipeline_, 0, 0);
    mvwprintw(win_pipeline_, 1, 2, "--- Multi-Channel Pipeline ---");

    // Headers for the grid
    const std::vector<std::string> stages = {"Fetch", "Decode", "Int ALU", "FPU", "Branch", "L/S Unit", "Memory", "Writeback"};
    int row = 2;
    int col = 10;
    for (const auto& stage : stages) {
        mvwprintw(win_pipeline_, row++, 2, "%s", stage.c_str());
    }

    // Draw time axis
    for (int i = 0; i < 10; ++i) { // Draw 10 cycles for now
        mvwprintw(win_pipeline_, 1, col + i * 6, "C%d", i);
    }

    if (pipeline_) {
        // This is a placeholder for the actual pipeline visualization logic.
        // It assumes pipeline_->get_pipeline_grid() returns a 2D vector of strings.
        // std::vector<std::vector<std::string>> grid = pipeline_->get_pipeline_grid();
        // for (size_t r = 0; r < grid.size(); ++r) {
        //     for (size_t c = 0; c < grid[r].size(); ++c) {
        //         mvwprintw(win_pipeline_, r + 2, col + c * 6, "%s", grid[r][c].c_str());
        //     }
        // }
        mvwprintw(win_pipeline_, row + 1, 2, "Pipeline visualization pending data structure.");
    }
}

void RiscUIManager::drawTextWindow(address_t current_pc) {
    // This function would be very similar to the one in UIManager,
    // but adapted for RISC-V instructions if needed.
    werase(win_text_segment_);
    box(win_text_segment_, 0, 0);
    mvwprintw(win_text_segment_, 1, 2, "--- Program ---");
    // Placeholder
    mvwprintw(win_text_segment_, 2, 2, "Text segment view for RISC-V.");
}

void RiscUIManager::drawInstructionDescription(address_t current_pc) {
    // This function would be very similar to the one in UIManager.
    werase(win_instruction_description_);
    box(win_instruction_description_, 0, 0);
    mvwprintw(win_instruction_description_, 1, 2, "--- Instruction Details ---");
    // Placeholder
    mvwprintw(win_instruction_description_, 2, 2, "Instruction description for RISC-V.");
}

void RiscUIManager::drawLegend() {
    werase(win_legend_);
    box(win_legend_, 0, 0);
    mvwprintw(win_legend_, 1, 2, "n: step | q: quit");
}

void RiscUIManager::refreshAll() {
    wnoutrefresh(win_gpr_);
    wnoutrefresh(win_text_segment_);
    wnoutrefresh(win_pipeline_);
    wnoutrefresh(win_instruction_description_);
    wnoutrefresh(win_legend_);
    doupdate();
}

bool RiscUIManager::waitForInput() {
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
