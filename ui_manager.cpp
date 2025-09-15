#include "x86_simulator.h"
#include <sstream>
#include <iomanip>
#include "decoder.h"

UIManager::UIManager(const Memory& memory_instance)
: win32(nullptr),
  win64(nullptr),
  win_text_segment(nullptr),
  memory_(memory_instance) {
    initscr();
    clear();
    refresh();
    cbreak();
    noecho();
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_YELLOW, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK); // New color pair for highlighted registers
    }
    // Create and position all windows.
    // The member windows are now created directly here.
    win32 = newwin(13, 35, 1, 1);
    win64 = newwin(32, 35, 1, 40);
    win_text_segment = newwin(25, 35, 14, 1);
}

UIManager::~UIManager() {
  tearDown();
}

void UIManager::tearDown() {
    delwin(win32);
    delwin(win64);
    delwin(win_text_segment);
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
      ss << std::setw(16) << regValue;
      found = true;
    }  else if (auto it = map32.find(regName); it != map32.end()){
      regValue = regs.get32(regName);
      ss << std::setw(8) << regValue;
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
  drawRegisterWindow(win32, "32-bit Registers", regs, RegisterDisplayOrder32);
  drawRegisterWindow(win64, "64-bit Registers", regs, RegisterDisplayOrder64);

}
void UIManager::drawTextWindow() {
  drawTextSegment( win_text_segment, "Program" );
}

void UIManager::drawTextSegment(WINDOW* win, const std::string& title) {
    werase(win);
    box(win, 0, 0);
    mvwprintw(win, 1, 2, "--- %s ---", title.c_str());
    Decoder& decoder = Decoder::getInstance();
    int y_offset = 2;
    const int max_y = getmaxy(win) - 1; // getmaxy returns the number of rows, so the last valid row is max_y - 1

    // Iterate up to the text segment's size.
    for (size_t i = 0; i < memory_.text_segment_size; /* nothing here, advancement is manual */) {
        if (y_offset >= max_y) {
            break;
        }
        address_t current_display_address = memory_.text_segment_start + i;
        if (auto decoded_instr_opt = decoder.decodeInstruction(memory_, current_display_address)) {
            DecodedInstruction decoded_instr = *decoded_instr_opt;
            // Display mnemonic
            mvwprintw(win, y_offset, 2, "%s", decoded_instr.mnemonic.c_str());
            
            // Display operands
            int x_offset = 2 + decoded_instr.mnemonic.length() + 1;
            for (const auto& operand : decoded_instr.operands) {
                mvwprintw(win, y_offset, x_offset, "%s", operand.text.c_str());
                x_offset += operand.text.length() + 1;
            }
            
            // Crucial change: Advance by the full instruction length
            current_display_address += decoded_instr.length_in_bytes;
            // ... (rest of the print logic remains the same) ...

            // Advance the index by the instruction length
            i += decoded_instr.length_in_bytes;
        } else {
            // Handle error...
            mvwprintw(win, y_offset, 2, "UNKNOWN");
            // Advance by one byte on failure
            i++;
        }
        y_offset++;
    }
}

void UIManager::refreshAll() {
    // Optimize redraws to avoid flicker by using wnoutrefresh and doupdate
    wnoutrefresh(win32);
    wnoutrefresh(win64);
    wnoutrefresh(win_text_segment);
    doupdate();
}

bool UIManager::waitForInput() {
    // Enable blocking behavior to wait for a key press
    nodelay(stdscr, FALSE);
    
    // Display a message in a central location, or in a specific window
    mvwprintw(stdscr, LINES - 2, 2, "Press 'q' to quit, any other key to continue...");
    wrefresh(stdscr);

    // Wait for and get a single character
    int ch = getch();

    // Clear the message
    wmove(stdscr, LINES - 2, 2);
    wclrtoeol(stdscr);
    wrefresh(stdscr);

    // Re-enable non-blocking if needed for other parts of your program
    // nodelay(stdscr, TRUE);

    return ch != 'q';
}
