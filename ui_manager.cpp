#include "ui_manager.h"
#include "x86_simulator.h"
#include <sstream>
#include <iomanip>
#include "decoder.h"
#include "instruction_describer.h" // Will be created later

UIManager::UIManager(const Memory& memory_instance)
: win32_(nullptr),
  win64_(nullptr),
  win_text_segment_(nullptr),
  win_instruction_description_(nullptr),
  memory_(memory_instance),
  text_scroll_offset_(0),
  decoded_program_(),
  address_to_index_map_() {
    initscr();
    clear();
    refresh();
    cbreak();
    noecho();
    keypad(stdscr, TRUE); // Enable keypad for arrow keys

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_YELLOW, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
    }
    // Create and position all windows.
    win32_ = newwin(13, 35, 1, 1);
    win64_ = newwin(32, 35, 1, 40);
    win_text_segment_ = newwin(25, 35, 14, 1);
    win_instruction_description_ = newwin(12, 70, 33, 40); // Position for the new window

}

UIManager::~UIManager() {
  tearDown();
}

void UIManager::tearDown() {
    delwin(win32_);
    delwin(win64_);
    delwin(win_text_segment_);
    delwin(win_instruction_description_);
    endwin();
}

void UIManager::drawRegisterWindow(WINDOW* win, const std::string& title,
				   const RegisterMap& regs,
				   std::vector<std::string> order) {
				 
  werase(win); // Clear the window before drawing
  box(win, 0, 0);
  mvwprintw(win, 1, 2, "--- %s ---", title.c_str());

  int row = 2;
  const auto& map64 = regs.getRegisterNameMap64();
  const auto& map32 = regs.getRegisterNameMap32();

  for (const std::string& regName : order) {
    std::stringstream ss;
    ss << std::left << std::setw(4) << regName << ": 0x" << std::hex << std::setfill('0');
    
    bool found = false;
    uint64_t regValue = 0;
    if (auto it = map64.find(regName); it != map64.end()) {
      regValue = regs.get64(regName);
      ss << std::right << std::setw(16) << regValue;
      found = true;
    }  else if (auto it = map32.find(regName); it != map32.end()){
      regValue = regs.get32(regName);
      ss << std::right << std::setw(8) << regValue;
      found = true;
    }

    if (found) {
      if (regName == "rip" || regName == "rsp" || regName == "esp") {
          wattron(win, COLOR_PAIR(2)); // Use green for rip, rsp, esp
      } else {
          wattron(win, COLOR_PAIR(1)); // Use yellow for others
      }
      mvwprintw(win, row++, 2, "%s", ss.str().c_str());
      wattroff(win, A_COLOR); // Turn off any color attribute
    }
  }
}

void UIManager::drawRegisters(const RegisterMap& regs) {
  drawRegisterWindow(win32_, "32-bit Registers", regs, RegisterDisplayOrder32);
  drawRegisterWindow(win64_, "64-bit Registers", regs, RegisterDisplayOrder64);

}
void UIManager::drawTextWindow(address_t current_rip) {
  drawTextSegment(win_text_segment_, "Program", current_rip);
}

void UIManager::drawInstructionDescription(address_t current_rip, const RegisterMap& regs) {
    werase(win_instruction_description_);
    box(win_instruction_description_, 0, 0);
    mvwprintw(win_instruction_description_, 1, 2, "--- Instruction Details ---");

    auto it = address_to_index_map_.find(current_rip);
    if (it != address_to_index_map_.end()) {
        const DecodedInstruction& instr = *decoded_program_[it->second];
        std::string description = InstructionDescriber::describe(instr, regs);

        // Truncate description to fit in the window to prevent overflow
        int max_desc_width = getmaxx(win_instruction_description_) - 3;
        if (max_desc_width < 0) max_desc_width = 0;
        if (description.length() > static_cast<size_t>(max_desc_width)) {
            description = description.substr(0, max_desc_width);
        }
        mvwaddstr(win_instruction_description_, 2, 2, description.c_str());
    } else {
        mvwprintw(win_instruction_description_, 2, 2, "No instruction data for this address.");
    }
}

void UIManager::drawTextSegment(WINDOW* win, const std::string& title, address_t current_rip) {
    werase(win);
    box(win, 0, 0);
    mvwprintw(win, 1, 2, "--- %s ---", title.c_str());

    int y_offset = 2;
    const int max_y = getmaxy(win) - 1;

    for (size_t i = text_scroll_offset_; i < decoded_program_.size(); ++i) {
        if (y_offset >= max_y) {
            mvwprintw(win, max_y, getmaxx(win) - 10, "[More...]");
            break;
        }

        const auto& decoded_instr = *decoded_program_[i];

        if (decoded_instr.address == current_rip) {
            wattron(win, COLOR_PAIR(2));
        }

        std::string line = decoded_instr.mnemonic;
        for (const auto& operand : decoded_instr.operands) {
            line += " " + operand.text;
        }

        // Truncate line to fit in the window to prevent overflow
        int max_width = getmaxx(win) - 3; // start at col 2, leave 1 for border
        if (max_width < 0) max_width = 0;
        if (line.length() > static_cast<size_t>(max_width)) {
            line = line.substr(0, max_width);
        }
        mvwaddstr(win, y_offset, 2, line.c_str());
        
        if (decoded_instr.address == current_rip) {
            wattroff(win, COLOR_PAIR(2));
        }

        y_offset++;
    }

    if (text_scroll_offset_ > 0) {
        mvwprintw(win, 2, getmaxx(win) - 10, "[More...]");
    }
}

void UIManager::preDecodeProgram() {
    Decoder& decoder = Decoder::getInstance();
    address_t addr = memory_.get_text_segment_start();
    size_t index = 0;
    while (addr < memory_.get_text_segment_start() + memory_.get_text_segment_size()) {
        if (auto decoded_instr_opt = decoder.decodeInstruction(memory_, addr)) {
            decoded_program_.push_back(std::make_unique<DecodedInstruction>(*decoded_instr_opt));
            address_to_index_map_[addr] = index++;
            addr += decoded_instr_opt->length_in_bytes;
        } else {
            addr++;
        }
    }
}

void UIManager::refreshAll() {
    wnoutrefresh(win32_);
    wnoutrefresh(win64_);
    wnoutrefresh(win_text_segment_);
    wnoutrefresh(win_instruction_description_);
    doupdate();
}

bool UIManager::waitForInput() {
    while (true) {
        mvwprintw(stdscr, LINES - 2, 2, "Press 'q' to quit, UP/DOWN to scroll, any other key to step...");
        
        int ch = getch();

        wmove(stdscr, LINES - 2, 2);
        wclrtoeol(stdscr);

        switch (ch) {
            case 'q':
                return false;
            case KEY_UP:
                if (text_scroll_offset_ > 0) {
                    text_scroll_offset_--;
                }
                drawTextWindow(0); // Dummy RIP for redraw
                refreshAll();
                break; // Continue loop
            case KEY_DOWN:
                if (!decoded_program_.empty() && text_scroll_offset_ < decoded_program_.size() - 1) {
                    text_scroll_offset_++;
                }
                drawTextWindow(0); // Dummy RIP for redraw
                refreshAll();
                break; // Continue loop
            default:
                return true; // Exit loop and continue simulation
        }
    }
}
