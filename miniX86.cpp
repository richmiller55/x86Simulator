#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <iomanip>
#include <cstdint>
#include "x86_registers.h" 
#include "register.h"     
#include "register_map.h" 
#include "string_utils.h" 
#include "parser_utils.h" 
#include <filesystem> //(C++17+)
#include <iomanip>    // For std::hex, std::setw, std::setfill
#include <fstream>
#include <chrono> 
#include <thread> 

#include "x86_simulator.h"
#include "memory.h"

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
    : regs32_(Registers32), regs64_(Registers64),
      memory_()
  {

  }
  void display32() {
    for (auto& [n,r] : regs32_)
      std::cout << n << ": " << r.getValue() << "\n";
  }
  void display64() {
    for (auto& [n,r] : regs64_)
      std::cout << n << ": " << r.getValue() << "\n";
  }

  // Method to execute a single instruction
  bool executeInstruction(const std::string& instruction, const std::string& args) {
    // Convert instruction string to a normalized format (e.g., uppercase)
    std::string normalizedInstr = instruction;
    std::transform(normalizedInstr.begin(), normalizedInstr.end(),
		   normalizedInstr.begin(), ::toupper);

    // This is a simplified example. A real simulator might use
    // more sophisticated decoding or a jump table/map of function pointers.

    if (normalizedInstr == "MOV") {
      // Parse arguments: "AX, BX" -> dest="AX", src="BX"
      std::vector<std::string> argParts = parseArguments(args); // A new helper function needed
      if (argParts.size() == 2) {
	handleMov(argParts[0], argParts[1]);
	return true;
      }
    } else if (normalizedInstr == "ADD") {
      std::vector<std::string> argParts = parseArguments(args);
      if (argParts.size() == 2) {
	handleAdd(argParts[0], argParts[1]);
	return true;
      }
    } else if (normalizedInstr == "JMP") {
      handleJmp(args); // JMP usually takes one argument: a label or address
      return true;
    }
    // ... handle other instructions

    std::cerr << "Error: unsupported instruction: " << instruction << std::endl;
    return false;
  }

  void runNextInstruction() {
    if (instructionPointer_ < programInstructions_.size()) {
      const auto& instrData = programInstructions_[instructionPointer_];
      if (executeInstruction(instrData[0], instrData[1])) {
	instructionPointer_++; // Increment IP if instruction executed successfully (unless it was a jump)
	// Note: JMP would set the IP itself, so careful here to not double-increment
	// For now, assume a simple sequential flow unless a JMP explicitly changes it
      } else {
	// Handle error, maybe halt simulation
	std::cerr << "Simulation halted due to instruction execution error." << std::endl;
	instructionPointer_ = programInstructions_.size(); // Stop execution
      }
    } else {
      std::cout << "End of program reached." << std::endl;
    }
  }

  void runProgram() {
    // Loop until instructionPointer_ goes out of bounds or a HALT instruction is encountered
    while (instructionPointer_ < programInstructions_.size()) {
      runNextInstruction();
      // Potentially add a delay here for step-by-step viewing
      // or call your displayRegistersWithDiff()
      displayRegistersWithDiff(); // Update display after each instruction
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  // You might want a method to fetch and execute the next instruction in sequence
  //  void runNextInstruction();


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
  void displayRegistersWithDiff() {
    displayRegistersControlled();
  }
      
  void displayRegistersControlled() {
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

  bool loadProgram(const std::string& filename) {
    if (!fs::exists(filename)) { // Check if the file exists using std::filesystem
      std::cerr << "Error: File '" << filename << "' does not exist." << std::endl;
      return false;
    }
      
    std::vector<std::string> programLines = readLinesFromFile(filename);

    if (programLines.empty()) {
      std::cout << "Warning: Loaded program file is empty unreadable" << std::endl;
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
  Memory memory_;
  // Helper functions for individual instructions
  void handleMov(const std::string& dest, const std::string& src) {
    uint64_t sourceValue = 0;

    // 1. Determine source value (from register or immediate value)
    // This is where it gets complex: differentiating between register names and immediate numbers
    if (auto it = regs64_.find(src); it != regs64_.end()) {
      sourceValue = it->second.getValue();
    } else if (auto it = regs32_.find(src); it != regs32_.end()) {
      sourceValue = it->second.getValue();
    } else {
      // Try to parse as a number (immediate value)
      try {
	// Support hex (0x...) and decimal
	if (src.rfind("0x", 0) == 0 || src.rfind("0X", 0) == 0) { // C++20 starts_with
	  sourceValue = std::stoull(src.substr(2), nullptr, 16);
	} else {
	  sourceValue = std::stoull(src);
	}
      } catch (const std::invalid_argument& e) {
	std::cerr << "Error: Invalid source operand in MOV: " << src << std::endl;
	return;
      } catch (const std::out_of_range& e) {
	std::cerr << "Error: Source operand out of range in MOV: " << src << std::endl;
	return;
      }
    }

    // 2. Update destination register
    if (!update(dest, sourceValue)) { // Use the existing update method
      std::cerr << "Error: Invalid destination register in MOV: " << dest << std::endl;
    }
    // Increment instruction pointer (this happens after each instruction, usually)
  }
  void handleAdd(const std::string& dest, const std::string& src) {
    uint64_t destValue = 0;
    uint64_t sourceValue = 0;

    // Get destination register value
    if (auto it = regs64_.find(dest); it != regs64_.end()) {
      destValue = it->second.getValue();
    } else if (auto it = regs32_.find(dest); it != regs32_.end()) {
      destValue = it->second.getValue();
    } else {
      std::cerr << "Error: Invalid destination operand in ADD: " << dest << std::endl;
      return;
    }

    // Determine source value (from register or immediate value)
    if (auto it = regs64_.find(src); it != regs64_.end()) {
      sourceValue = it->second.getValue();
    } else if (auto it = regs32_.find(src); it != regs32_.end()) {
      sourceValue = it->second.getValue();
    } else {
      // Try to parse as a number (immediate value)
      try {
	if (src.rfind("0x", 0) == 0 || src.rfind("0X", 0) == 0) {
	  sourceValue = std::stoull(src.substr(2), nullptr, 16);
	} else {
	  sourceValue = std::stoull(src);
	}
      } catch (const std::invalid_argument& e) {
	std::cerr << "Error: Invalid source operand in ADD: " << src << std::endl;
	return;
      } catch (const std::out_of_range& e) {
	std::cerr << "Error: Source operand out of range in ADD: " << src << std::endl;
	return;
      }
    }

    // Perform addition and update destination
    update(dest, destValue + sourceValue);
    // Increment instruction pointer
  }


  void handleJmp(const std::string& targetLabel) {
    // This is significantly more complex. You need to:
    // 1. Store labels and their corresponding line numbers during program loading.
    // 2. Look up the 'targetLabel' in your stored label map.
    // 3. Set the 'instructionPointer_' to the line number of the target label.
    std::cerr << "JMP instruction not yet fully implemented for labels." << std::endl;
    // For now, let's just assume it's an absolute line number for simplicity (not realistic for asm)
    // try {
    //     size_t lineNumber = std::stoul(targetLabel);
    //     if (lineNumber < programInstructions_.size()) {
    //         instructionPointer_ = lineNumber;
    //     } else {
    //         std::cerr << "Error: JMP target out of bounds: " << targetLabel << std::endl;
    //     }
    // } catch(...) {
    //      std::cerr << "Error: JMP target must be a label or address." << std::endl;
    // }

    // No instructionPointer_ increment here, as JMP sets the new IP.
  }

  // You'll need an Instruction Pointer (e.g., store the current instruction line number)
  size_t instructionPointer_ = 0;
  std::vector<std::vector<std::string>> programInstructions_; // Store parsed program
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
 



    //   std::cout << "\n--- Loading Program Example ---" << std::endl;

   /*
    sim.loadProgram("./programs/program1.asm");

    std::cout << "\n--- Loading another Program Example ---" << std::endl;
    sim.loadProgram("./programs/subdirectory/program2.asm");
   */

  return 0;
}
