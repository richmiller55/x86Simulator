#include "x86_simulator.h"
#include <sstream>
#include <iomanip>
#include "decoder.h"

UIManager::UIManager(const Memory& memory_instance)
: memory_(memory_instance),
  win32(nullptr),
  win64(nullptr),
  win_text_segment(nullptr){
    initscr();
    cbreak();
    noecho();
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    }
    // Create and position all windows.
    // The member windows are now created directly here.
    win32 = newwin(15, 35, 1, 1);
    win64 = newwin(32, 35, 1, 40);
    win_text_segment = newwin(25, 35, 16, 1);
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
  //  const auto& mapSeg = regs.getSegmentRegisterMap();
  for (const std::string& regName : order) {
    std::string formattedString;   
    if (auto it = map64.find(regName); it != map64.end()) {
      const uint64_t r = it->second;
      std::stringstream ss;
      ss << std::left << std::setw(4) << regName << ": 0x" <<
	std::hex << std::setw(8) << std::setfill('0') << r;
      formattedString = ss.str();
    }  else if (auto it = map32.find(regName); it != map32.end()){
      const uint64_t r = it->second;
      std::stringstream ss;
      ss << std::left << std::setw(4) << regName << ": 0x" <<
	std::hex << std::setw(8) << std::setfill('0') << r;
      formattedString = ss.str();
    }
      wattron(win, COLOR_PAIR(1));
      mvwprintw(win, row++, 2, "%s", formattedString.c_str());
      wattroff(win, COLOR_PAIR(1));
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
    // ...
    Decoder& decoder = Decoder::getInstance(); // Or use the dependency-injected instance
    address_t current_display_address = memory_.text_segment_start;
    int y_offset = 1;
    while (current_display_address < memory_.text_segment_size) {
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
        } else {
            // Handle error, e.g., print "UNKNOWN" and move one byte
            mvwprintw(win, y_offset, 2, "UNKNOWN");
            current_display_address++;
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

void UIManager::waitForInput() {
    // Disable blocking behavior
    nodelay(stdscr, FALSE);
    
    // Display a message in a central location, or in a specific window
    mvwprintw(stdscr, LINES - 2, 2, "Press any key to continue...");
    wrefresh(stdscr);

    // Wait for and get a single character
    getch();

    // Clear the message
    mvwprintw(stdscr, LINES - 2, 2, "                              "); // Overwrite with spaces
    wrefresh(stdscr);

    // Re-enable non-blocking if needed for other parts of your program
    // nodelay(stdscr, TRUE);
}
