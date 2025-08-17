#include "x86_simulator.h"

  void X86Simulator::displayRegistersWithDiff() {
    displayRegistersControlled();
  }
      
  void X86Simulator::displayRegistersControlled() {
    // Clear screen
    std::cout << "\033[2J\033[H"; // Clears screen and moves cursor to top-left

    // Display 32-bit registers
    int row = 2;
    int col = 1;
    std::cout << "\033[" << row++ << ";" << col << "H" << "--- 32-bit Registers ---";
    for (const std::string& regName : RegisterDisplayOrder32) {
      if (auto it = regs32_.find(regName); it != regs32_.end()) {
	const Register& r = it->second;
	std::cout << "\033[" << row++ << ";" << col << "H"; // Move cursor for this line

	// Build the string first, then apply color if needed
	std::stringstream ss;
	ss << std::left << std::setw(4) << regName << ": 0x" << std::hex << std::setw(8) << std::setfill('0') << r.getValue();
	std::string formattedString = ss.str();

	if (r.getValue() != r.getPreviousValue()) {
	  std::cout << "\033[93m" << formattedString << "\033[0m";
	} else {
	  std::cout << formattedString;
	}
      }
    }

    // Display 64-bit registers
    row = 2;
    col = 40;
    std::cout << "\033[" << row++ << ";" << col << "H" << "--- 64-bit Registers ---";

    for (const std::string& regName : RegisterDisplayOrder64) {
      if (auto it = regs64_.find(regName); it != regs64_.end()) {
	const Register& r = it->second;
	std::cout << "\033[" << row++ << ";" << col << "H"; // Move cursor for this line

	std::stringstream ss;
	ss << std::left << std::setw(4) << regName << ": 0x" << std::hex << std::setw(16) << std::setfill('0') << r.getValue();
	std::string formattedString = ss.str();

	if (r.getValue() != r.getPreviousValue()) {
	  std::cout << "\033[93m" << formattedString << "\033[0m";
	} else {
	  std::cout << formattedString;
	}
      }
    }
    std::cout << "\033[25;1H"; // Move cursor to a safe spot
    std::cout << std::dec; // Reset to decimal for other output
    std::flush(std::cout); // Ensure output is written immediately
  }

