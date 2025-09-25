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
    symbolTable_.clear();
    
    address_t text_lc = memory_.get_text_segment_start();
    address_t data_lc = memory_.get_data_segment_start();
    address_t bss_lc = memory_.get_bss_segment_start();

    address_t* current_lc = &text_lc; 

    for (const std::string& line_raw : programLines_) {
        std::string line = trim(line_raw);
        if (line.empty() || line[0] == ';') {
            continue;
        }

        std::vector<std::string> tokens = parseLine(line);
        if (tokens.empty()) {
            continue;
        }

        std::string first_token = tokens[0];
        std::transform(first_token.begin(), first_token.end(), first_token.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        
        // Handle labels
        if (first_token.back() == ':') {
            std::string label = first_token.substr(0, first_token.size() - 1);
            symbolTable_[label] = *current_lc;
            // Handle labels on the same line as an instruction/directive
            if (tokens.size() > 1) {
                // Remove the label token and reprocess the rest of the line
                tokens.erase(tokens.begin());
                first_token = tokens[0];
                std::transform(first_token.begin(), first_token.end(), first_token.begin(),
                               [](unsigned char c){ return std::tolower(c); });
            } else {
                continue; // Processed a label on its own line
            }
        }
        
        // Handle section directives
        if (first_token == "section") {
            if (tokens.size() > 1) {
                std::string section_name = tokens[1];
                if (section_name == ".text") {
                    current_lc = &text_lc;
                } else if (section_name == ".data") {
                    current_lc = &data_lc;
                } else if (section_name == ".bss") {
                    current_lc = &bss_lc;
                } else {
                    log(session_id_, "Unknown section directive: " + section_name, "ERROR", 0, __FILE__, __LINE__);
                    return false;
                }
            }
        }
        // Handle data directives
        else if (current_lc == &data_lc) {
            std::vector<std::string> data_tokens = tokens;
            if (tokens[0].back() == ':') {
                data_tokens.erase(data_tokens.begin());
            }

            if (!data_tokens.empty() && data_tokens[0] == "dd") {
                for (size_t i = 1; i < data_tokens.size(); ++i) {
                    std::string val_str = data_tokens[i];
                    if (val_str.back() == ',') {
                        val_str.pop_back();
                    }
                    if (val_str.empty()) continue;

                    uint32_t val_to_write = 0;
                    if (val_str.find('.') != std::string::npos) {
                        float f = std::stof(val_str);
                        val_to_write = *reinterpret_cast<uint32_t*>(&f);
                    } else {
                        val_to_write = static_cast<uint32_t>(std::stoll(val_str));
                    }

                    for (int j = 0; j < 4; ++j) {
                        if (*current_lc + j < memory_.main_memory->size()) {
                            memory_.main_memory->at(*current_lc + j) = (val_to_write >> (j * 8)) & 0xFF;
                        }
                    }
                    *current_lc += 4;
                }
            } else {
                *current_lc += calculate_data_size(tokens);
            }
        }
        // Handle BSS directives
        else if (current_lc == &bss_lc) {
            *current_lc += calculate_bss_size(tokens);
        }
        // Handle instructions in the text segment
        else {
            CodeGenerator temp_gen(symbolTable_); // Will now have data labels
            std::vector<uint8_t> code = temp_gen.generate_code({line_raw});
            *current_lc += code.size();
        }
    }
    
    // Update the memory object with the final text segment size
    memory_.set_text_segment_size(text_lc - memory_.get_text_segment_start());

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

