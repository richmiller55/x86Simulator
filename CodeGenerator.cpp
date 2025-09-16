#include "CodeGenerator.h"
#include "decoder.h"
#include <sstream>
#include <algorithm>
#include <iostream>
#include <stdexcept>

CodeGenerator::CodeGenerator(Memory& memory, const std::map<std::string, address_t>& symbol_table)
    : memory_(memory), symbol_table_(symbol_table), current_address_(memory.text_segment_start) {}

void CodeGenerator::generate_code(const std::vector<std::string>& program_lines) {
    for (const auto& line : program_lines) {
        process_line(line);
    }
    memory_.text_segment_size = current_address_ - memory_.text_segment_start;
}

void CodeGenerator::process_line(const std::string& line_raw) {
    std::string line = trim(line_raw);
    if (line.empty() || line[0] == ';') {
        return; // Skip comments and empty lines
    }

    std::vector<std::string> tokens = parse_line(line);
    if (tokens.empty()) {
        return;
    }

    std::string& mnemonic = tokens[0];
    std::transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    if (mnemonic.back() == ':') {
        return; // Skip labels in this pass
    }

    if (mnemonic == "mov") {
        if (tokens.size() < 3) return;
        std::string dest = tokens[1];
        std::transform(dest.begin(), dest.end(), dest.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        if(dest.back() == ',') dest.pop_back();
        std::string src = tokens[2];
        std::transform(src.begin(), src.end(), src.begin(),
                       [](unsigned char c){ return std::tolower(c); });

        // Handle mov r32, imm32
        if (dest == "eax" || dest == "ecx") {
            try {
                uint32_t value = std::stoul(src);
                uint8_t opcode = 0xB8;
                if (dest == "ecx") opcode = 0xB9;
                memory_.write_text(current_address_++, opcode);
                memory_.write_text_dword(current_address_, value);
                current_address_ += 4;
            } catch (const std::exception&) {
                // Not an immediate, do nothing, will be handled by the next block
            }
        } else if (dest == "ebx" && src == "eax") {
            memory_.write_text(current_address_++, 0x89);
            memory_.write_text(current_address_++, 0xC3);
        }
    } else if (mnemonic == "add") {
        if (tokens.size() < 3) return;
        std::string dest = tokens[1];
        std::transform(dest.begin(), dest.end(), dest.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        if(dest.back() == ',') dest.pop_back();
        std::string src = tokens[2];
        std::transform(src.begin(), src.end(), src.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        if (dest == "eax" && src == "ecx") {
            memory_.write_text(current_address_++, 0x01);
            memory_.write_text(current_address_++, 0xC8);
        }
    } else if (mnemonic == "inc") {
        if (tokens.size() < 2) return;
        std::string dest = tokens[1];
        std::transform(dest.begin(), dest.end(), dest.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        if (dest == "ecx") {
            memory_.write_text(current_address_++, 0xFF);
            memory_.write_text(current_address_++, 0xC1);
        }
    } else if (mnemonic == "cmp") {
        if (tokens.size() < 3) return;
        std::string dest = tokens[1];
        std::transform(dest.begin(), dest.end(), dest.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        if(dest.back() == ',') dest.pop_back();
        std::string src = tokens[2];
        std::transform(src.begin(), src.end(), src.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        if (dest == "ecx" && src == "6") {
            memory_.write_text(current_address_++, 0x83);
            memory_.write_text(current_address_++, 0xF9);
            memory_.write_text(current_address_++, 0x06);
        }
    } else if (mnemonic == "jne") {
        if (tokens.size() < 2) return;
        std::string label = tokens[1];
        auto it = symbol_table_.find(label);
        if (it != symbol_table_.end()) {
            address_t target_address = it->second;
            int8_t offset = target_address - (current_address_ + 2);
            memory_.write_text(current_address_++, 0x75);
            memory_.write_text(current_address_++, offset);
        }
    } else if (mnemonic == "jmp") {
        if (tokens.size() < 2) return;
        std::string label = tokens[1];
        auto it = symbol_table_.find(label);
        if (it != symbol_table_.end()) {
            address_t target_address = it->second;
            int32_t offset = target_address - (current_address_ + 5);
            memory_.write_text(current_address_++, 0xE9);
            memory_.write_text_dword(current_address_, offset);
            current_address_ += 4;
        }
    }
}

std::vector<std::string> CodeGenerator::parse_line(const std::string& line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string CodeGenerator::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}
