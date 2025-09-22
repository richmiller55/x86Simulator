#include "ui_manager.h"
#include "register_enums.h"
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
  ymm_scroll_offset_(0),
  current_view_(UIView::kNormal),
  show_flags_as_text_(true),
  ymm_view_mode_(YmmViewMode::HEX_256),
  display_base_(DisplayBase::HEX),
  current_regs_(nullptr)
{
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
    }

    win32_ = newwin(1, 1, 1, 1);
    win64_ = newwin(1, 1, 1, 1);
    win_text_segment_ = newwin(1, 1, 1, 1);
    win_ymm_ = newwin(1, 1, 1, 1);
    win_instruction_description_ = newwin(1, 1, 1, 1);
    win_legend_ = newwin(1, 1, 1, 1);

    arrangeWindows();
}

namespace { // Anonymous namespace for layout constants
    const WindowLayout kNormalWin32Layout = { .y = 1, .x = 1, .height = 13, .width = 30 };
    const WindowLayout kNormalWin64Layout = { .y = 1, .x = 32, .height = 22, .width = 30 };
    const WindowLayout kNormalTextSegmentLayout = { .y = 14, .x = 1, .height = 25, .width = 30 };
    const WindowLayout kNormalYmmLayout = { .y = 23, .x = 32, .height = 12, .width = 80 };
    const WindowLayout kNormalInstructionDescLayout = { .y = 1, .x = 63, .height = 12, .width = 50 };
    const WindowLayout kNormalLegendLayout = { .y = 14, .x = 63, .height = 6, .width = 50 };

    const WindowLayout kExpandedYmmLayout = { .y = 23, .x = 32, .height = 19, .width = 80 };
} // namespace

std::string UIManager::formatFlags(uint64_t flags_value) {
    std::stringstream ss;
    ss << "[ ";
    ss << ((flags_value >> RFLAGS_OF_BIT) & 1 ? 'O' : '-');
    ss << ((flags_value >> RFLAGS_DF_BIT) & 1 ? 'D' : '-');
    ss << ((flags_value >> RFLAGS_IF_BIT) & 1 ? 'I' : '-');
    ss << ((flags_value >> RFLAGS_TF_BIT) & 1 ? 'T' : '-');
    ss << ((flags_value >> RFLAGS_SF_BIT) & 1 ? 'S' : '-');
    ss << ((flags_value >> RFLAGS_ZF_BIT) & 1 ? 'Z' : '-');
    ss << ((flags_value >> RFLAGS_AF_BIT) & 1 ? 'A' : '-');
    ss << ((flags_value >> RFLAGS_PF_BIT) & 1 ? 'P' : '-');
    ss << ((flags_value >> RFLAGS_CF_BIT) & 1 ? 'C' : '-');
    ss << " ]";
    return ss.str();
}

void UIManager::arrangeWindows() {
    const auto& win32_layout = kNormalWin32Layout;
    const auto& win64_layout = kNormalWin64Layout;
    const auto& text_segment_layout = kNormalTextSegmentLayout;
    const auto& instruction_desc_layout = kNormalInstructionDescLayout;
    const auto& legend_layout = kNormalLegendLayout;
    const auto& ymm_layout = (current_view_ == UIView::kNormal) ? kNormalYmmLayout : kExpandedYmmLayout;

    mvwin(win32_, win32_layout.y, win32_layout.x);
    wresize(win32_, win32_layout.height, win32_layout.width);

    mvwin(win64_, win64_layout.y, win64_layout.x);
    wresize(win64_, win64_layout.height, win64_layout.width);

    mvwin(win_text_segment_, text_segment_layout.y, text_segment_layout.x);
    wresize(win_text_segment_, text_segment_layout.height, text_segment_layout.width);

    mvwin(win_ymm_, ymm_layout.y, ymm_layout.x);
    wresize(win_ymm_, ymm_layout.height, ymm_layout.width);

    mvwin(win_instruction_description_, instruction_desc_layout.y, instruction_desc_layout.x);
    wresize(win_instruction_description_, instruction_desc_layout.height, instruction_desc_layout.width);

    mvwin(win_legend_, legend_layout.y, legend_layout.x);
    wresize(win_legend_, legend_layout.height, legend_layout.width);

    clear();
    refresh();
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
			  const std::vector<std::string>& order,
			  YmmViewMode current_ymm_view_mode, DisplayBase current_display_base,
			  size_t scroll_offset, int max_regs) {
  werase(win);
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

  size_t end = (max_regs == -1) ? order.size() : std::min(order.size(), scroll_offset + static_cast<size_t>(max_regs));
  for (size_t i = scroll_offset; i < end; ++i) {
    const std::string& regName = order[i];
    std::stringstream ss;
    ss << std::left << std::setw(4) << regName << ": ";
    
    bool found = false;
    uint64_t regValue = 0;
    if (auto it = map64.find(regName); it != map64.end()) {
      regValue = regs.get64(regName);
      if (regName == "rflags" && show_flags_as_text_) {
          ss << formatFlags(regValue);
      } else {
          ss << "0x" << std::hex << std::setfill('0') << std::right << std::setw(16) << regValue;
      }
      found = true;
    } else if (auto it_ymm = regs.getRegisterNameMapYmm().find(regName); it_ymm != regs.getRegisterNameMapYmm().end()) {
        __m256i ymmValue = regs.getYmm(regName);
        ss << format_ymm_register(ymmValue, current_ymm_view_mode, current_display_base);
        found = true;
    }  else if (auto it = map32.find(regName); it != map32.end()){
      regValue = regs.get32(regName);
      if (regName == "eflags" && show_flags_as_text_) {
          ss << formatFlags(regValue);
      } else {
          ss << "0x" << std::hex << std::setfill('0') << std::right << std::setw(8) << regValue;
      }
      found = true;
    }

    if (found) {
      if (regName == "rip" || regName == "rsp" || regName == "esp") {
          wattron(win, COLOR_PAIR(2));
      } else {
          wattron(win, COLOR_PAIR(1));
      }
      mvwprintw(win, row++, 2, "%s", ss.str().c_str());
      wattroff(win, A_COLOR);
    }
  }
}

void UIManager::drawMainRegisters(const RegisterMap& regs) {
  drawRegisterWindow(win32_, "32-bit Registers", regs, RegisterDisplayOrder32, YmmViewMode::HEX_256, DisplayBase::HEX);
  drawRegisterWindow(win64_, "64-bit Registers", regs, RegisterDisplayOrder64, YmmViewMode::HEX_256, DisplayBase::HEX);
}

void UIManager::drawYmmRegisters(const RegisterMap& regs) {
    if (current_view_ == UIView::kNormal) {
        drawRegisterWindow(win_ymm_, "YMM Registers (Peek)", regs, RegisterDisplayOrderYMM_, ymm_view_mode_, display_base_, ymm_scroll_offset_, 8);
    } else {
        drawRegisterWindow(win_ymm_, "YMM Registers (Expanded)", regs, RegisterDisplayOrderYMM_, ymm_view_mode_, display_base_);
    }
}
void UIManager::drawTextWindow(address_t current_rip) {
  drawTextSegment(win_text_segment_, "Program", current_rip);
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

void UIManager::drawLegend() {
    werase(win_legend_);
    box(win_legend_, 0, 0);
    mvwprintw(win_legend_, 1, 2, "n: step | q: quit | m: toggle view");
    mvwprintw(win_legend_, 2, 2, "up/down: scroll text | +/-: scroll YMM");
    mvwprintw(win_legend_, 3, 2, "v: YMM view | d/x/o: base | f: flags");
}

bool UIManager::waitForInput() {
    while (true) {
        int ch = getch();

        switch (ch) {
            case 'q':
                return false;
            case 'n':
                return true; // continue simulation
            case 'f':
                show_flags_as_text_ = !show_flags_as_text_;
                if (current_regs_) {
                    drawMainRegisters(*current_regs_);
                    refreshAll();
                }
                break;
            case 'm':
                current_view_ = (current_view_ == UIView::kNormal) ? UIView::kYmmExpanded : UIView::kNormal;
                arrangeWindows();
                if (current_regs_) {
                    drawMainRegisters(*current_regs_);
                    drawYmmRegisters(*current_regs_);
                    drawTextWindow(current_regs_->get64("rip"));
                    drawInstructionDescription(current_regs_->get64("rip"), *current_regs_);
                    drawLegend();
                }
                refreshAll();
                break;
            case '+':
                if (ymm_scroll_offset_ < RegisterDisplayOrderYMM_.size() - 3) {
                    ymm_scroll_offset_++;
                }
                if (current_regs_) {
                    drawYmmRegisters(*current_regs_);
                }
                refreshAll();
                break;
            case '-':
                if (ymm_scroll_offset_ > 0) {
                    ymm_scroll_offset_--;
                }
                if (current_regs_) {
                    drawYmmRegisters(*current_regs_);
                }
                refreshAll();
                break;
            case 'v': { // Cycle YMM view mode
                ymm_view_mode_ = static_cast<YmmViewMode>((static_cast<int>(ymm_view_mode_) + 1) % 5); // 5 modes
                if (current_regs_) { // Redraw YMM window if regs are available
                    drawYmmRegisters(*current_regs_);
                    refreshAll();
                }
                break;
            }
            case 'd': // Set display base to Decimal
                display_base_ = DisplayBase::DEC;
                if (current_regs_) {
                    drawYmmRegisters(*current_regs_);
                    refreshAll();
                }
                break;
            case 'x': // Set display base to Hexadecimal
                display_base_ = DisplayBase::HEX;
                if (current_regs_) {
                    drawYmmRegisters(*current_regs_);
                    refreshAll();
                }
                break;
            case 'o': // Set display base to Octal
                display_base_ = DisplayBase::OCT;
                if (current_regs_) {
                    drawYmmRegisters(*current_regs_);
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