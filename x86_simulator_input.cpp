#include "x86_simulator.h"
#include "ui_manager.h"
#include "ui_manager.h"
#include "decoder.h" // For DecodedOperand and helper types if any
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm> // For std::remove_if, std::isspace
#include "CodeGenerator.h"

#include <stdexcept>
#include <string>
#include <vector>
#include <sstream>

namespace fs = std::filesystem;

// Implement the helper functions here if they are not part of a class
// and their declarations were put in X86Simulator.h
// Example:
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
    //    log(0, "Unable to open file: ", "ERROR", 0, __FILE__, __LINE__); 
  }
  return lines;
}

bool X86Simulator::loadProgram(const std::string& filename) {
  memory_.reset();
  programLines_ = readLinesFromFile(filename);
  return !programLines_.empty(); // Or a more robust check for successful read.
}

std::vector<std::string> X86Simulator::parseLine(const std::string& line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

bool X86Simulator::firstPass() {
    // First pass is now only for building the symbol table.
    address_t location_counter = 0;
    for (const std::string& line_raw : programLines_) {
        std::string line = trim(line_raw);
        if (line.empty() || line[0] == ';') {
            continue;
        }

        std::vector<std::string> tokens = parseLine(line);
        if (tokens.empty()) {
            continue;
        }

        std::string& mnemonic = tokens[0];
        std::transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        if (mnemonic.back() == ':') {
            std::string label = mnemonic.substr(0, mnemonic.size() - 1);
            symbolTable_[label] = location_counter;
        }
        // Handle assembler directives
        else if (mnemonic == "section" || mnemonic == "global") {
            // These directives do not generate code and do not advance the location counter.
            // We can just ignore them for the purpose of address calculation.
            // A more advanced assembler might use 'section' to switch between .text, .data, etc.
            // and 'global' to adjust symbol visibility, but for now, we can skip.
            continue;
        }
        // The CodeGenerator will determine the actual location counter during the second pass.
        // To keep the symbol table correct, we must simulate the address calculation here.
        else {
            // To get the correct address for labels, we must calculate the size of each instruction.
            // We use a CodeGenerator for this, as it has the sizing logic.
            // With the refactoring, we no longer need a temporary Memory object.
            CodeGenerator temp_gen(symbolTable_);
            std::vector<std::string> single_line_vec = { line_raw };
            std::vector<uint8_t> code = temp_gen.generate_code(single_line_vec);
            size_t instruction_size = code.size();
            location_counter += instruction_size;
        }
    }
    return true;
}

bool X86Simulator::secondPass() {
    CodeGenerator code_generator(symbolTable_);
    std::vector<uint8_t> machine_code = code_generator.generate_code(programLines_);
    program_size_in_bytes_ = machine_code.size();

    // Write the generated code to the simulator's memory
    for (size_t i = 0; i < program_size_in_bytes_; ++i) {
        memory_.write_text(memory_.get_text_segment_start() + i, machine_code[i]);
    }

    memory_.set_text_segment_size(program_size_in_bytes_);

    // Set the initial instruction pointer (RIP) to the address of the entry point label.
    auto it = symbolTable_.find(entryPointLabel_);
    if (it != symbolTable_.end()) {
        register_map_.set64("rip", it->second);
    } else {
        log(session_id_, "Entry point label '" + entryPointLabel_ + "' not found. Defaulting to start of text segment.", "ERROR", 0, __FILE__, __LINE__);
        // Fallback to the start of the text segment if the label is not found
        register_map_.set64("rip", memory_.text_segment_start);
    }
    auto program_decoder = std::make_unique<ProgramDecoder>(memory_);
    program_decoder->decode();
    if (ui_) {
        ui_->setProgramDecoder(std::move(program_decoder));
    }
    return true;
}

// Helper to remove leading/trailing whitespace
std::string X86Simulator::trim(const std::string& str) {
  size_t first = str.find_first_not_of(" \t\n\r");
  if (std::string::npos == first) {
    return "";
  }
  size_t last = str.find_last_not_of(" \t\n\r");
  return str.substr(first, (last - first + 1));
}
void X86Simulator::waitForInput() {
  if (ui_) {
    ui_->waitForInput();
  }
}
