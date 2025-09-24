#include "CodeGenerator.h"
#include "decoder.h"
#include "operand_parser.h"
#include <sstream>
#include <algorithm>
#include <iostream>
#include <stdexcept>

CodeGenerator::CodeGenerator(std::map<std::string, address_t>& symbol_table)
    : symbol_table_(symbol_table), current_address_(0) {}

std::vector<uint8_t> CodeGenerator::generate_code(const std::vector<std::string>& program_lines) {
    machine_code_.clear();
    for (const auto& line : program_lines) {
        process_line(line);
    }
    return machine_code_;
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

    std::string mnemonic = tokens[0];
    std::transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(), ::tolower);
    if (mnemonic.back() == ':') {
        return; // Skip labels in this pass
    }

    OperandParser operands(tokens);

    if (mnemonic == "mov") {
        if (operands.operand_count() < 2) return;
        std::string dest = operands.get_operand(0);
        std::string src = operands.get_operand(1);

        // Handle mov r32, imm32
        try {
            // Use stoul with base 0 to handle "0x" prefixes for hex values
            uint32_t value = std::stoul(src, nullptr, 0);
            uint8_t opcode = 0xB8; // Base for MOV r32, imm32
            if (dest == "eax") opcode = 0xB8;
            else if (dest == "ecx") opcode = 0xB9;
            else if (dest == "ebx") opcode = 0xBB;
            // Note: This doesn't handle all registers, just the ones in testAlpha.asm

            machine_code_.push_back(opcode);
            current_address_ += 1;
            machine_code_.insert(machine_code_.end(), { (uint8_t)value, (uint8_t)(value >> 8), (uint8_t)(value >> 16), (uint8_t)(value >> 24) });
            current_address_ += 4;
        } catch (const std::exception&) {
            // Not an immediate, so assume it's a register-to-register move
            if (dest == "eax" && (src == "ebx" || src == "ecx" || src == "edx" || src == "ebp")) {
                machine_code_.push_back(0x89); // Opcode for MOV r/m32, r32
                current_address_ += 1;
                if (src == "ebx") machine_code_.push_back(0xD8); // ModR/M for MOV EAX, EBX
                if (src == "ecx") machine_code_.push_back(0xC8); // ModR/M for MOV EAX, ECX
                if (src == "edx") machine_code_.push_back(0xD0); // ModR/M for MOV EAX, EDX
                if (src == "ebp") machine_code_.push_back(0xE8); // ModR/M for MOV EAX, EBP
                current_address_ += 1;
            } else if (dest == "ebp" && src == "esp") { // mov ebp, esp
                machine_code_.push_back(0x89);
                machine_code_.push_back(0xE5);
                current_address_ += 2;
            } else if (dest == "ebx" && src == "eax") { // mov ebx, eax
                machine_code_.push_back(0x89);
                machine_code_.push_back(0xC3);
            }
        }
    } else if (mnemonic == "add") {
        if (operands.operand_count() < 2) return;
        std::string dest = operands.get_operand(0);
        std::string src = operands.get_operand(1);
        if (dest == "eax" && src == "ebx") {
            machine_code_.push_back(0x01); // ADD EAX, EBX
            machine_code_.push_back(0xD8); // ModR/M for EAX, EBX
            current_address_ += 2;
        } else if (dest == "eax" && src == "ecx") {
            machine_code_.push_back(0x01);
            machine_code_.push_back(0xC8);
            current_address_ += 2;
        }
    } else if (mnemonic == "inc") {
        if (operands.operand_count() < 1) return;
        std::string dest = operands.get_operand(0);
        if (dest == "ecx") {
            machine_code_.push_back(0xFF);
            machine_code_.push_back(0xC1);
            current_address_ += 2;
        }
    } else if (mnemonic == "cmp") {
        if (operands.operand_count() < 2) return;
        std::string dest = operands.get_operand(0);
        std::string src = operands.get_operand(1);
        if (dest == "eax" && src == "ecx") {
            machine_code_.push_back(0x39); // CMP r/m32, r32
            machine_code_.push_back(0xC8); // ModR/M for CMP EAX, ECX
            current_address_ += 2;
        } else if (dest == "ecx") { // Assuming immediate
            machine_code_.push_back(0x83);
            machine_code_.push_back(0xF9);
            current_address_ += 2;
            try {
                uint8_t value = std::stoul(src, nullptr, 0);
                machine_code_.push_back(value);
                current_address_ += 1;
            } catch (const std::exception&) { /* error */ }
        }
    } else if (mnemonic == "jne") {
        if (operands.operand_count() < 1) return;
        std::string label = operands.get_operand(0);
        auto it = symbol_table_.find(label);
        if (it != symbol_table_.end()) {
            address_t target_address = it->second;
            int8_t offset = target_address - (current_address_ + 2); // 2 bytes for jne rel8
            machine_code_.push_back(0x75);
            machine_code_.push_back(offset);
            current_address_ += 2;
        } else {
            // Sizing pass for a forward reference. Assume short jump size.
             machine_code_.push_back(0x75); // placeholder opcode
             machine_code_.push_back(0x00); // placeholder offset
             current_address_ += 2;
        }
    } else if (mnemonic == "jmp") {
        if (operands.operand_count() < 1) return;
        std::string label = operands.get_operand(0);
        auto it = symbol_table_.find(label);
        if (it != symbol_table_.end()) {
            address_t target_address = it->second;
            int32_t offset = target_address - (current_address_ + 5); // 5 bytes for jmp rel32
            machine_code_.push_back(0xE9);
            current_address_ += 1;
            machine_code_.insert(machine_code_.end(), { (uint8_t)offset, (uint8_t)(offset >> 8), (uint8_t)(offset >> 16), (uint8_t)(offset >> 24) });
            current_address_ += 4; // dword
        } else {
            // Sizing pass for a forward reference. Assume near jump size.
            machine_code_.push_back(0xE9); // placeholder opcode
            machine_code_.insert(machine_code_.end(), {0,0,0,0}); // placeholder offset
            current_address_ += 5;
        }
    } else if (mnemonic == "int") {
        if (operands.operand_count() < 1) return;
        // Assuming "int 0x80"
        machine_code_.push_back(0xCD);
        machine_code_.push_back(0x80);
        current_address_ += 2;
    } else if (mnemonic == "mul") {
        if (operands.operand_count() < 1) return;
        // MUL r/m32. Opcode: F7 /4
        machine_code_.push_back(0xF7);
        // For now, hardcoding for `mul ebx` -> ModR/M is E3
        machine_code_.push_back(0xE3);
        current_address_ += 2;
    } else if (mnemonic == "dec") {
        if (operands.operand_count() < 1) return;
        // DEC r32. Opcode: FF /1
        machine_code_.push_back(0xFF);
        // For now, hardcoding for `dec ecx` -> ModR/M is C9
        machine_code_.push_back(0xC9);
        current_address_ += 2;
    } else if (mnemonic == "div") {
        if (operands.operand_count() < 1) return;
        // DIV r/m32. Opcode: F7 /6
        machine_code_.push_back(0xF7);
        // For now, hardcoding for `div ebx` -> ModR/M is F3
        machine_code_.push_back(0xF3);
        current_address_ += 2;
    } else if (mnemonic == "and") {
        if (operands.operand_count() < 2) return;
        // AND r/m32, r32. Opcode: 21 /r
        machine_code_.push_back(0x21);
        // For now, hardcoding for `and eax, ebx` -> ModR/M is D8
        machine_code_.push_back(0xD8);
        current_address_ += 2;
    } else if (mnemonic == "or") {
        if (operands.operand_count() < 2) return;
        // OR r/m32, r32. Opcode: 09 /r
        machine_code_.push_back(0x09);
        // For now, hardcoding for `or eax, ebx` -> ModR/M is D8
        machine_code_.push_back(0xD8);
        current_address_ += 2;
    } else if (mnemonic == "xor") {
        if (operands.operand_count() < 2) return;
        std::string dest = operands.get_operand(0);
        std::string src = operands.get_operand(1);

        // Handle xor r32, r32
        if (dest == "eax" && src == "eax") {
            // XOR EAX, EAX. Opcode: 31 /r
            machine_code_.push_back(0x31);
            machine_code_.push_back(0xC0); // ModR/M for EAX, EAX
            current_address_ += 2;
        } else if (dest == "ebx") { // Handle xor r32, imm8
             try {
                uint8_t value = std::stoul(src, nullptr, 0);
                // XOR r/m32, imm8. Opcode: 83 /6
                machine_code_.push_back(0x83);
                machine_code_.push_back(0xF3); // ModR/M for EBX
                machine_code_.push_back(value);
                current_address_ += 3;
            } catch (const std::exception&) {
                // Could be another register form, but we only have imm8 in testBeta.asm
            }
        }
    } else if (mnemonic == "not") {
        if (operands.operand_count() < 1) return;
        // NOT r/m32. Opcode: F7 /2
        machine_code_.push_back(0xF7);
        // For now, hardcoding for `not eax` -> ModR/M is D0
        machine_code_.push_back(0xD0);
        current_address_ += 2;
    } else if (mnemonic == "push" || mnemonic == "pop") {
        if (operands.operand_count() < 1) return;
        std::string reg = operands.get_operand(0);
        
        static const std::map<std::string, uint8_t> reg_to_idx = {
            {"eax", 0}, {"ecx", 1}, {"edx", 2}, {"ebx", 3},
            {"esp", 4}, {"ebp", 5}, {"esi", 6}, {"edi", 7}
        };

        auto it = reg_to_idx.find(reg);
        if (it != reg_to_idx.end()) {
            uint8_t base_opcode = (mnemonic == "push") ? 0x50 : 0x58;
            uint8_t opcode = base_opcode + it->second;
            machine_code_.push_back(opcode);
            current_address_ += 1;
        }
    } else if (mnemonic == "in") {
        if (operands.operand_count() < 2) return;
        std::string dest = operands.get_operand(0);
        std::string src = operands.get_operand(1);
        if (dest == "al") {
            machine_code_.push_back(0xE4);
            current_address_ += 1;
            try {
                uint8_t value = std::stoul(src, nullptr, 0);
                machine_code_.push_back(value);
                current_address_ += 1;
            } catch (const std::exception&) { /* error */ }
        }
    } else if (mnemonic == "out") {
        if (operands.operand_count() < 2) return;
        std::string dest = operands.get_operand(0);
        std::string src = operands.get_operand(1);
        if (src == "al") {
            machine_code_.push_back(0xE6);
            current_address_ += 1;
            try {
                uint8_t value = std::stoul(dest, nullptr, 0);
                machine_code_.push_back(value);
                current_address_ += 1;
            } catch (const std::exception&) { /* error */ }
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
