#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <iomanip>
#include <cstdint>
#include "simulator_utils.h"
#include "x86_registers.h" 
#include "register.h"     
#include "register_map.h" 
#include <filesystem> //(C++17+)
#include <iomanip>    // For std::hex, std::setw, std::setfill
#include <fstream>

namespace fs = std::filesystem;

// Function to read lines from a file into a vector of strings
std::vector<std::string> readLinesFromFile(const std::string& filePath) {
    std::vector<std::string> lines;
    std::ifstream file(filePath);

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        file.close();
    } else {
        std::cerr << "Error: Unable to open file: " << filePath << std::endl;
    }
    return lines;
}


class X86Simulator {
public:
  X86Simulator()
    : regs32_(Registers32), regs64_(Registers64) {}
  void display32() {
    for (auto& [n,r] : regs32_)
      std::cout << n << ": " << r.getValue() << "\n";
  }

  void display64() {
    for (auto& [n,r] : regs64_)
      std::cout << n << ": " << r.getValue() << "\n";
  }

  bool update(const std::string& name, uint64_t val) {
    bool updated = false;
    if (auto it = regs32_.find(name); it != regs32_.end()) {
      if (it->second.getValue() != val) { // Only update if value changed
          it->second.update(val);
          updated = true;
          // Trigger a display update for this specific register (more advanced)
      }
    }
    if (auto it = regs64_.find(name); it != regs64_.end()) {
      if (it->second.getValue() != val) { // Only update if value changed
          it->second.update(val);
          updated = true;
          // Trigger a display update for this specific register (more advanced)
      }
    }
    return updated;
  }

  // A more controlled display method that could be called after each instruction
  void displayRegistersControlled() {
      // Clear screen (might be too aggressive, depending on desired layout)
      // std::cout << "\033[2J\033[H"; // Clears screen and moves cursor to top-left

      // Display 32-bit registers
      int row = 2; // Start from row 2 for registers
      int col = 1;
      std::cout << "\033[" << row++ << ";" << col << "H" << "--- 32-bit Registers ---";
      for (auto& [n,r] : regs32_) {
          std::cout << "\033[" << row++ << ";" << col << "H"; // Move cursor
          // Format the output
          std::cout << n << ": 0x" << std::hex << std::setw(8) << std::setfill('0') << r.getValue();
      }

      // Display 64-bit registers
      row = 2; // Reset row, or adjust for adjacent display
      col = 40; // Start at column 40 for 64-bit registers
      std::cout << "\033[" << row++ << ";" << col << "H" << "--- 64-bit Registers ---";
      for (auto& [n,r] : regs64_) {
          std::cout << "\033[" << row++ << ";" << col << "H"; // Move cursor
          // Format the output
          std::cout << n << ": 0x" << std::hex << std::setw(16) << std::setfill('0') << r.getValue();
      }
      std::cout << std::dec; // Reset to decimal for other output

      // You might also want to move the cursor to a safe "bottom" area
      // after displaying registers so that subsequent console output doesn't overwrite them.
      std::cout << "\033[20;1H"; // Example: Move to row 20, column 1
  }


 bool loadProgram(const std::string& filename) {
      if (!fs::exists(filename)) { // Check if the file exists using std::filesystem
          std::cerr << "Error: File '" << filename << "' does not exist." << std::endl;
          return false;
      }
      
      std::vector<std::string> programLines = readLinesFromFile(filename);

      if (programLines.empty()) {
          std::cout << "Warning: Loaded program file is empty or could not be read." << std::endl;
          return false;
      }

      std::cout << "Loading program from " << filename << ":" << std::endl;
      for (const std::string& line : programLines) {
          std::vector<std::string> parsedLine = parseLine(line); // Use your existing parseLine function
          if (parsedLine.size() == 2) {
              std::cout << "  Instruction: " << parsedLine[0] << ", Arguments: " << parsedLine[1] << std::endl;
              // Here, you would typically store these instructions
              // and arguments in a data segment within your simulator.
              // For demonstration, we're just printing them.
          } else if (!line.empty()) { // Handle non-empty lines that don't parse as expected
              std::cout << "  Skipping malformed line: " << line << std::endl;
          }
      }
      std::cout << "Program loaded." << std::endl;
      return true;
  }

private:
  RegisterMap regs32_;
  RegisterMap regs64_;
};

void test_sim_utils()
{
      std::string testLine = "  MOV  AX, BX  ";
    std::vector<std::string> parsed = parseLine(testLine);

    if (!parsed.empty()) {
        std::cout << "Instruction: " << parsed[0] << std::endl;
        std::cout << "Arguments: " << parsed[1] << std::endl;
    }

    std::string anotherString = "   Hello World!   ";
    trim(anotherString);
    std::cout << "Trimmed string: " << anotherString << std::endl;
}

int main() {

    X86Simulator sim;

    sim.update("eax", 0x12345678);
    sim.update("rax", 0xABCDEF0123456789);
    sim.displayRegistersControlled(); // Call the new display method

    sim.update("ebx", 0xAAAA); // Update another register
    // To see the update, you'd call displayRegistersControlled again,
    // or ideally, have a more granular update mechanism.
    sim.displayRegistersControlled();
 



   std::cout << "\n--- Loading Program Example ---" << std::endl;

    // Example of loading a program from a file
    // Assuming you have a file named 'programs/program1.txt'
    // and another file named 'programs/subdirectory/program2.txt'
    sim.loadProgram("programs/program1.txt");

    std::cout << "\n--- Loading another Program Example ---" << std::endl;
    sim.loadProgram("programs/subdirectory/program2.txt");

    std::cout << "\n--- Trying to load a non-existent file ---" << std::endl;
    sim.loadProgram("non_existent_program.txt");


  return 0;
}
