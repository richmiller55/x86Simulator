#include "x86_simulator.h"
#include <ncurses.h>

  void X86Simulator::displayRegistersWithDiff() {
    displayRegistersControlled();
  }
      
void X86Simulator::displayRegistersControlled() {
  initscr(); // Initialize the screen
  cbreak();  // Line buffering disabled, pass keypresses immediately
  noecho();  // Don't echo input characters

  // Create a window for 32-bit registers
  WINDOW *win32 = newwin(15, 35, 1, 1); // 15 rows, 35 cols, start at (1,1)
  // box(win32, 0, 0); // Draw a border around the window
  mvwprintw(win32, 1, 2, "--- 32-bit Registers ---"); // Print in window

  int row = 2;
  for (const std::string& regName : RegisterDisplayOrder32) {
    // ... get register value and previous value ...
    if (auto it = regs32_.find(regName); it != regs32_.end()) {
      const Register& r = it->second;

      std::stringstream ss;
      ss << std::left << std::setw(4) << regName << ": 0x" <<
	std::hex << std::setw(8) << std::setfill('0') << r.getValue();
      std::string formattedString = ss.str();

      if (r.getValue() != r.getPreviousValue()) {
	wattron(win32, COLOR_PAIR(1)); // Turn on color (e.g., yellow)
	mvwprintw(win32, row++, 2, "%s", formattedString.c_str());
	wattroff(win32, COLOR_PAIR(1)); // Turn off color
      } else {
	mvwprintw(win32, row++, 2, "%s", formattedString.c_str());
      }
    }
    wrefresh(win32); // Refresh the window
  }
    // Create a window for 64-bit registers
    WINDOW *win64 = newwin(32, 35, 1, 40); // 32 rows, 35 cols, start at (1,40)
    row = 2;
    // box(win64, 0, 0);
    mvwprintw(win64, 1, 2, "--- 64-bit Registers ---");

    for (const std::string& regName : RegisterDisplayOrder64) {
 
      if (auto it = regs64_.find(regName); it != regs64_.end()) {
	const Register& r = it->second;
	std::stringstream ss;
        ss << std::left << std::setw(4) << regName << ": 0x" << std::hex
	   << std::setw(8) << std::setfill('0') << r.getValue();
        std::string formattedString = ss.str();

        if (r.getValue() != r.getPreviousValue()) {
	  wattron(win64, COLOR_PAIR(1)); // Turn on color (e.g., yellow)
	  mvwprintw(win64, row++, 2, "%s", formattedString.c_str());
	  wattroff(win64, COLOR_PAIR(1)); // Turn off color
        } else {
	  mvwprintw(win64, row++, 2, "%s", formattedString.c_str());
        }
      }
    }
      wrefresh(win64);

}
