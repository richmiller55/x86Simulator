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
  win_ymm_(nullptr),
  win_instruction_description_(nullptr),
  win_legend_(nullptr),
  memory_(memory_instance),
  text_scroll_offset_(0),
  ymm_view_mode_(YmmViewMode::HEX_256), // Initialize to default view
  display_base_(DisplayBase::HEX),     // Initialize to hex display
  current_regs_(nullptr),
  RegisterDisplayOrderYMM_({ // Initialize RegisterDisplayOrderYMM_
    "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7",
    "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15"
  })
{
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
    win_ymm_ = newwin(19, 80, 33, 40); // New window for YMM registers
    win_instruction_description_ = newwin(12, 70, 1, 78); // Position for the new window
    win_legend_ = newwin(4, 70, 14, 78);

}

void UIManager::setRegisterMap(const RegisterMap* regs) {
    current_regs_ = regs;
}

UIManager::~UIManager() {
  tearDown();
}

void UIManager::setProgramDecoder(std::unique_ptr<ProgramDecoder> decoder) {
    program_decoder_ = std::move(decoder);
}

void UIManager::tearDown() {
    delwin(win32_);
    delwin(win64_);
    delwin(win_text_segment_);
    delwin(win_ymm_);
    delwin(win_instruction_description_);
    delwin(win_legend_);
    endwin();
}

void UIManager::drawRegisterWindow(WINDOW* win, const std::string& title,
			  const RegisterMap& regs,
			  std::vector<std::string> order,
			  YmmViewMode current_ymm_view_mode, DisplayBase current_display_base) { 		 
  werase(win); // Clear the window before drawing
  box(win, 0, 0);

  std::string actual_title = title;
  if (title == "YMM Registers") {
      std::string view_str;
      switch (current_ymm_view_mode) {
          case YmmViewMode::HEX_256: view_str = "256-bit Hex"; break;
          case YmmViewMode::INTS_8x32: view_str = "8x32-bit Ints"; break;
          case YmmViewMode::INTS_4x64: view_str = "4x64-bit Ints"; break;
          case YmmViewMode::INTS_16x16: view_str = "16x16-bit Ints"; break;
          case YmmViewMode::INTS_32x8: view_str = "32x8-bit Ints"; break;
      }
      std::string base_str;
      switch (current_display_base) {
          case DisplayBase::DEC: base_str = "Dec"; break;
          case DisplayBase::HEX: base_str = "Hex"; break;
          case DisplayBase::OCT: base_str = "Oct"; break;
      }
      actual_title += " (" + view_str + " / " + base_str + ")";
  }
  mvwprintw(win, 1, 2, "--- %s ---", actual_title.c_str());

  int row = 2;
  const auto& map64 = regs.getRegisterNameMap64();
  const auto& map32 = regs.getRegisterNameMap32();

  for (const std::string& regName : order) {
    std::stringstream ss;
    ss << std::left << std::setw(4) << regName << ": ";
    
    bool found = false;
    uint64_t regValue = 0;
    if (auto it = map64.find(regName); it != map64.end()) {
      regValue = regs.get64(regName);
      ss << "0x" << std::hex << std::setfill('0') << std::right << std::setw(16) << regValue;
      found = true;
    } else if (auto it_ymm = regs.getRegisterNameMapYmm().find(regName); it_ymm != regs.getRegisterNameMapYmm().end()) {
        // Special handling for YMM registers using the new formatting utility
        __m256i ymmValue = regs.getYmm(regName);
        ss << format_ymm_register(ymmValue, current_ymm_view_mode, current_display_base);
        found = true;
    }  else if (auto it = map32.find(regName); it != map32.end()){
      regValue = regs.get32(regName);
      ss << "0x" << std::hex << std::setfill('0') << std::right << std::setw(8) << regValue;
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
  drawRegisterWindow(win32_, "32-bit Registers", regs, RegisterDisplayOrder32, YmmViewMode::HEX_256, DisplayBase::HEX);
  drawRegisterWindow(win64_, "64-bit Registers", regs, RegisterDisplayOrder64, YmmViewMode::HEX_256, DisplayBase::HEX);
  drawRegisterWindow(win_ymm_, "YMM Registers", regs, RegisterDisplayOrderYMM_, ymm_view_mode_, display_base_);
  drawLegend();
}
void UIManager::drawTextWindow(address_t current_rip) {
  drawTextSegment(win_text_segment_, "Program", current_rip);
}

void UIManager::drawLegend() {
    werase(win_legend_);
    box(win_legend_, 0, 0);
    mvwprintw(win_legend_, 1, 2, "n: step | b: back | q: quit");
    mvwprintw(win_legend_, 2, 2, "up/down: scroll | v: cycle YMM view");
    mvwprintw(win_legend_, 3, 2, "d: dec | x: hex | o: oct");
}

void UIManager::drawInstructionDescription(address_t current_rip, const RegisterMap& regs) {
    werase(win_instruction_description_);
    box(win_instruction_description_, 0, 0);
    mvwprintw(win_instruction_description_, 1, 2, "--- Instruction Details ---");

    if (!program_decoder_) return;
    const auto& address_to_index_map = program_decoder_->getAddressToIndexMap();
    const auto& decoded_program = program_decoder_->getDecodedProgram();

    auto it = address_to_index_map.find(current_rip);
    if (it != address_to_index_map.end()) {
        const DecodedInstruction& instr = *decoded_program[it->second];
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

    if (!program_decoder_) return;
    const auto& decoded_program = program_decoder_->getDecodedProgram();

    int y_offset = 2;
    const int max_y = getmaxy(win) - 1;

    for (size_t i = text_scroll_offset_; i < decoded_program.size(); ++i) {
        if (y_offset >= max_y) {
            mvwprintw(win, max_y, getmaxx(win) - 10, "[More...]");
            break;
        }

        const auto& decoded_instr = *decoded_program[i];

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

void UIManager::refreshAll() {
    wnoutrefresh(win32_);
    wnoutrefresh(win64_);
    wnoutrefresh(win_ymm_);
    wnoutrefresh(win_text_segment_);
    wnoutrefresh(win_instruction_description_);
    wnoutrefresh(win_legend_);
    doupdate();
}

bool UIManager::waitForInput() {
    while (true) {
        int ch = getch();

        switch (ch) {
            case 'q':
                return false;
            case 'n':
                return true; // continue simulation
            case 'b':
                // Step-back functionality is not implemented in the core simulator.
                // For now, this key does nothing.
                break;
            case 'v': { // Cycle YMM view mode
                ymm_view_mode_ = static_cast<YmmViewMode>((static_cast<int>(ymm_view_mode_) + 1) % 5); // 5 modes
                if (current_regs_) { // Redraw YMM window if regs are available
                    drawRegisters(*current_regs_);
                    refreshAll();
                }
                break;
            }
            case 'd': // Set display base to Decimal
                display_base_ = DisplayBase::DEC;
                if (current_regs_) {
                    drawRegisters(*current_regs_);
                    refreshAll();
                }
                break;
            case 'x': // Set display base to Hexadecimal
                display_base_ = DisplayBase::HEX;
                if (current_regs_) {
                    drawRegisters(*current_regs_);
                    refreshAll();
                }
                break;
            case 'o': // Set display base to Octal
                display_base_ = DisplayBase::OCT;
                if (current_regs_) {
                    drawRegisters(*current_regs_);
                    refreshAll();
                }
                break;
            case KEY_UP:
                if (text_scroll_offset_ > 0) {
                    text_scroll_offset_--;
                }
                drawTextWindow(0); // Dummy RIP for redraw
                refreshAll();
                break; // Continue loop
            case KEY_DOWN:
                if (program_decoder_ && !program_decoder_->getDecodedProgram().empty() && text_scroll_offset_ < program_decoder_->getDecodedProgram().size() - 1) {
                    text_scroll_offset_++;
                }
                drawTextWindow(0); // Dummy RIP for redraw
                refreshAll();
                break; // Continue loop
            default:
                break;
        }
    }
}
