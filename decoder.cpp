#include "decoder.h"
#include <map>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

// Helper to get 8-bit register name from index
const char* getRegisterName8(uint8_t index) {
    static const char* registers[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
    if (index < 8) return registers[index];
    return "err";
}

std::unique_ptr<Decoder> Decoder::instance;

// Helper to get 32-bit register name from index
const char* getRegisterName(uint8_t index) {
    static const char* registers[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
    if (index < 8) return registers[index];
    return "err";
}

// Helper to decode ModR/M byte
void decodeModRM(uint8_t modrm, DecodedInstruction& instr, bool is_lea = false) {
    uint8_t mod = (modrm >> 6) & 0x03;
    uint8_t reg_field = (modrm >> 3) & 0x07;
    uint8_t rm  = modrm & 0x07;

    if (mod == 0b11) { // Register-to-register
        DecodedOperand dest, src;
        dest.type = OperandType::REGISTER;
        // For LEA, the reg field is the destination. For others, it's often the source.
        dest.text = getRegisterName(is_lea ? reg_field : rm);
        src.type = OperandType::REGISTER;
        src.text = getRegisterName(is_lea ? rm : reg_field);
        instr.operands.push_back(dest);
        instr.operands.push_back(src);
    }
    // Other modes (memory access) not implemented for simplicity
}

Decoder::Decoder() {
    opcode_to_mnemonic = {
        {0x90, "NOP"},
        {0x66, "TWO_BYTE_OPCODE_PREFIX"}, // Example prefix
        {0x50, "PUSH"}, {0x51, "PUSH"}, {0x52, "PUSH"}, {0x53, "PUSH"}, {0x54, "PUSH"}, {0x55, "PUSH"}, {0x56, "PUSH"}, {0x57, "PUSH"},
        {0x58, "POP"}, {0x59, "POP"}, {0x5A, "POP"}, {0x5B, "POP"}, {0x5C, "POP"}, {0x5D, "POP"}, {0x5E, "POP"}, {0x5F, "POP"},
        {0x01, "ADD"},
        {0x29, "SUB"},
        {0xEB, "JMP"},
        {0xE9, "JMP"},
        {0xE8, "CALL"},
        {0x09, "OR"},
        {0x31, "XOR"},
        {0x21, "AND"},
        {0x39, "CMP"},
        {0x83, "CMP"},
        {0x75, "JNE"},
        {0x74, "JE"},
        {0x72, "JB"},
        {0x7C, "JL"},
        {0x7D, "JGE"},
        {0x73, "JAE"},
        {0x76, "JBE"},
        {0x78, "JS"},
        {0x79, "JNS"},
        {0x70, "JO"},
        {0x71, "JNO"},
        {0x87, "XCHG"},
        {0x8D, "LEA"},
        {0xC1, "GROUP_C1"}, // SHL, SHR, etc.
        {0x7F, "JG"},
        {0x77, "JA"},
        {0xB8, "MOV"},
        {0xA4, "MOVSB"},
        {0xA5, "MOVSD"}, // MOVSW is 66 A5
        {0xB9, "MOV"},
        {0xBB, "MOV"},
        {0x89, "MOV"},
        {0xF7, "GROUP_F7"}, // MUL, NOT, DIV, etc.
        {0xFF, "GROUP_FF"}, // INC, DEC
        // The below are for simple cases, but the group is more accurate
        {0x40, "INC"}, 
        {0xFF, "INC"},
        {0xCD, "INT"},
        {0xE4, "IN"},
        {0xE6, "OUT"}
    };

    two_byte_opcode_to_mnemonic = {
        {0x8E, "JLE"},
        {0xBE, "MOVSX"},
        {0xB6, "MOVZX"}
    };

    vex_opcode_to_mnemonic = {
        // VZEROUPPER
        {{1, 0x77}, "VZEROUPPER"},
        // VADDPS
        {{1, 0x58}, "VADDPS"},
        // VDIVPS
        {{1, 0x5E}, "VDIVPS"},
        // VMAXPS
        {{1, 0x5F}, "VMAXPS"},
        // VPANDN
        {{1, 0xDF}, "VPANDN"},
        // VPAND
        {{1, 0xDB}, "VPAND"},
        // VPMULLW
        {{1, 0xD5}, "VPMULLW"},
        // VMINPS
        {{1, 0x5D}, "VMINPS"},
        // VMOVUPS
        {{1, 0x10}, "VMOVUPS"},
        {{1, 0x11}, "VMOVUPS"},
        // VPXOR
        {{1, 0xEF}, "VPXOR"},
        // VRCPPS
        {{1, 0x53}, "VRCPPS"},
        // VSQRTPS
        {{1, 0x51}, "VSQRTPS"},
        // VSUBPS
        {{1, 0x5C}, "VSUBPS"},
        // VPOR
        {{1, 0xEB}, "VPOR"},
    };

    mnemonic_to_opcode = {
        {"NOP", 0x90},
        {"TWO_BYTE_OPCODE_PREFIX", 0x66}, 
        {"POP", 0x5d},
        {"PUSH", 0x55},
        {"ADD", 0x01},
        {"SUB", 0x29},
        {"JMP", 0xEB},
        {"OR", 0x09},
        {"XOR", 0x31},
        {"AND", 0x21},
        {"CMP", 0x39},
        {"JNE", 0x75},
        {"JE", 0x74},
        {"JB", 0x72},
        {"JL", 0x7C},
        {"JGE", 0x7D},
        {"JAE", 0x73},
        {"JBE", 0x76},
        {"JS", 0x78},
        {"JNS", 0x79},
        {"JO", 0x70},
        {"JLE", 0x8E},
        {"JNO", 0x71},
        {"CALL", 0xE8},
        {"SHL", 0xC1},
        {"SHR", 0xC1},
        {"SAR", 0xC1},
        {"ROL", 0xC1},
        {"ROR", 0xC1},
        {"XCHG", 0x87},
        {"IMUL", 0xF7},
        {"IDIV", 0xF7},
        {"MOVZX", 0xB6}, // As part of 0F prefix
        {"MOVSX", 0xBE}, // As part of 0F prefix
        {"LEA", 0x8D},
        {"JG", 0x7F},
        {"JA", 0x77},
        {"MOVSB", 0xA4},
        {"MOVSD", 0xA5},
        {"MOVSW", 0xA5},
        {"MOV", 0xB8},
        {"INC", 0x40},
        {"INT", 0xCD},
        {"IN", 0xE4},
        {"OUT", 0xE6}
    };

    instruction_lengths = {
        {0x90, 1}, // NOP
        {0xB8, 5}, // MOV EAX, imm32
        {0xB9, 5}, // MOV ECX, imm32
        {0xBB, 5}, // MOV EBX, imm32
        {0x89, 2}, // MOV r/m32, r32
        {0x55, 1}, // PUSH EBP
        {0x5D, 1}, // POP EBP
        {0x01, 2}, // ADD r/m32, r32
        {0x29, 2}, // SUB r/m32, r32
        {0xEB, 2}, // JMP rel8
        {0xE9, 5}, // JMP rel32
        {0xE8, 5}, // CALL rel32
        {0x09, 2}, // OR r/m32, r32
        {0x31, 2}, // XOR r/m32, r32
        {0x21, 2}, // AND r/m32, r32
        {0x39, 2}, // CMP r/m32, r32
        {0x83, 3}, // CMP r/m32, imm8
        {0x75, 2}, // JNE rel8
        {0x74, 2}, // JE rel8
        {0x72, 2}, // JB rel8
        {0x7C, 2}, // JL rel8
        {0x7D, 2}, // JGE rel8
        {0x73, 2}, // JAE rel8
        {0x76, 2}, // JBE rel8
        {0x78, 2}, // JS rel8
        {0x79, 2}, // JNS rel8
        {0x70, 2}, // JO rel8
        {0x71, 2}, // JNO rel8
        {0xC1, 3}, // SHL r/m32, imm8
        {0xC1, 3}, // SHR r/m32, imm8
        {0xC1, 3}, // SAR r/m32, imm8
        {0xC1, 3}, // ROL r/m32, imm8
        {0xC1, 3}, // ROR r/m32, imm8
        {0x87, 2}, // XCHG r/m32, r32
        {0x8D, 2}, // LEA r32, m
        {0x8E, 6}, // JLE rel32
        {0xB6, 3}, // MOVZX r32, r/m8 (with 0F prefix)
        {0xBE, 3}, // MOVSX r32, r/m8 (with 0F prefix)
        {0x7F, 2}, // JG rel8
        {0x40, 1}, // INC EAX (Legacy)
        {0xA5, 1}, // MOVSD
        {0x66, 2}, // MOVSW (66 A5)
        {0xA4, 1}, // MOVSB
        {0xFF, 2}, // INC r/m32
        {0x48, 1}, // DEC EAX (Legacy)
        {0xF7, 2}, // Group F7
        {0xCD, 2},
        {0xE4, 2},
        {0xE6, 2}
    };
}

Decoder& Decoder::getInstance() {
    if (!instance) {
        instance = std::unique_ptr<Decoder>(new Decoder());
    }
    return *instance;
}

void Decoder::resetInstance() {
    instance.reset(nullptr);
}

std::string Decoder::getMnemonic(uint8_t opcode) const {
    if (auto it = opcode_to_mnemonic.find(opcode); it != opcode_to_mnemonic.end()) {
        return it->second;
    }
    return "UNKNOWN";
}

uint8_t Decoder::getOpcode(const std::string& mnemonic) const {
    if (auto it = mnemonic_to_opcode.find(mnemonic); it != mnemonic_to_opcode.end()) {
        return it->second;
    }
    return 0; // or throw an exception for unknown mnemonic
}

std::unique_ptr<DecodedInstruction> Decoder::decodeInstruction(const Memory& memory, address_t address) {
    if (address >= memory.get_data_segment_start()) {
        return nullptr;
    }

    auto decoded_instr = std::make_unique<DecodedInstruction>();
    decoded_instr->address = address;
    uint8_t opcode = memory.read_text(address);
    address_t current_address = address;

    if (opcode == 0xC4 || opcode == 0xC5) {
        VEX_Prefix vex_prefix = decodeVEXPrefix(memory, current_address);
        uint8_t vex_opcode = memory.read_text(current_address);
	/*
        std::cout << "\n--- VEX DECODE TRACE ---" << std::endl;
        std::cout << "Address: 0x" << std::hex << address << std::endl;
        std::cout << "VEX Opcode: 0x" << std::hex << (int)vex_opcode << std::endl;
        std::cout << "Map Select: " << std::dec << vex_prefix.map_select << std::endl;
	*/
        if (auto it = vex_opcode_to_mnemonic.find({vex_prefix.map_select, vex_opcode}); it != vex_opcode_to_mnemonic.end()) {
            std::string mnemonic = it->second;
            // std::cout << "Mnemonic Found: " << mnemonic << std::endl;
            std::transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(), ::tolower);
            decoded_instr->mnemonic = mnemonic;
        } else {
	  //std::cout << "Mnemonic NOT Found" << std::endl;
            decoded_instr->mnemonic = "avx_instruction_unknown";
        }


        decodeAVXOperands(*decoded_instr, vex_prefix, memory, current_address);
        decoded_instr->length_in_bytes = vex_prefix.bytes + 1 + 1; // VEX + opcode + ModR/M
        for (const auto& op : decoded_instr->operands) {
            if (op.type == OperandType::MEMORY) {
                decoded_instr->length_in_bytes += 4; // 32-bit displacement
                break;
            }
        } 

    } else if (opcode == 0x0F) {
        current_address++;
        uint8_t next_byte = memory.read_text(current_address);
        if (next_byte == 0xB6) { // MOVZX
            decoded_instr->mnemonic = "movzx";
        } else if (next_byte == 0xBE) { // MOVSX
            decoded_instr->mnemonic = "movsx";
        } else if (next_byte == 0x8E) { // JLE
            decoded_instr->mnemonic = "jle";
        }
        auto it = two_byte_opcode_to_mnemonic.find(next_byte);
        if (it != two_byte_opcode_to_mnemonic.end()) {
            decoded_instr->mnemonic = it->second;
            std::transform(decoded_instr->mnemonic.begin(), decoded_instr->mnemonic.end(), decoded_instr->mnemonic.begin(), ::tolower);

            if (decoded_instr->mnemonic == "jle") {
                decoded_instr->length_in_bytes = 6;
                current_address++;
                int32_t offset = memory.read_text_dword(current_address);
                address_t target_address = address + decoded_instr->length_in_bytes + offset;
                DecodedOperand op;
                op.type = OperandType::IMMEDIATE;
                op.value = target_address;
                std::stringstream ss;
                ss << "0x" << std::hex << target_address;
                op.text = ss.str();
                decoded_instr->operands.push_back(op);
            } else if (decoded_instr->mnemonic == "movsx") {
                decoded_instr->length_in_bytes = 3; // 0F + BE + ModRM
                current_address++;
                uint8_t modrm = memory.read_text(current_address);
                uint8_t mod = (modrm >> 6) & 0x03;
                uint8_t reg_field = (modrm >> 3) & 0x07;
                uint8_t rm = modrm & 0x07;

                DecodedOperand dest, src;
                dest.type = OperandType::REGISTER;
                dest.text = getRegisterName(reg_field); // Destination is r32

                src.type = OperandType::REGISTER;
                src.text = getRegisterName8(rm); // Source is r8

                decoded_instr->operands.push_back(dest);
                decoded_instr->operands.push_back(src);
            } else if (decoded_instr->mnemonic == "movzx") {
                decoded_instr->length_in_bytes = 3; // 0F + B6 + ModRM
                current_address++;
                uint8_t modrm = memory.read_text(current_address);
                uint8_t mod = (modrm >> 6) & 0x03;
                uint8_t reg_field = (modrm >> 3) & 0x07;
                uint8_t rm = modrm & 0x07;

                DecodedOperand dest, src;
                dest.type = OperandType::REGISTER;
                dest.text = getRegisterName(reg_field); // Destination is r32

                src.type = OperandType::REGISTER;
                src.text = getRegisterName8(rm); // Source is r8

                decoded_instr->operands.push_back(dest);
                decoded_instr->operands.push_back(src);
            }
        }
    } else if (opcode == 0x66) { // Operand-size override prefix
        current_address++;
        uint8_t next_byte = memory.read_text(current_address);
        if (next_byte == 0xA5) {
            decoded_instr->mnemonic = "movsw";
            decoded_instr->length_in_bytes = 2;
        }
    } else {
        // Existing logic for non-AVX instructions
        current_address++;
        std::string mnemonic = decodeMnemonic(opcode);
        std::transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(), ::tolower);
        decoded_instr->mnemonic = mnemonic;
        if (decoded_instr->mnemonic == "UNKNOWN") {
            return nullptr;
        }

        if (opcode >= 0xB8 && opcode <= 0xBF) { // MOV r32, imm32
            decoded_instr->length_in_bytes = 5;
            uint32_t imm_value = memory.read_text_dword(current_address);
            DecodedOperand reg, imm;
            reg.type = OperandType::REGISTER;
            reg.text = getRegisterName(opcode - 0xB8);
            imm.type = OperandType::IMMEDIATE;
            imm.value = imm_value;
            std::stringstream ss;
            ss << "0x" << std::hex << imm_value;
            imm.text = ss.str();
            decoded_instr->operands.push_back(reg);
            decoded_instr->operands.push_back(imm);
        } else if (opcode == 0x89 || opcode == 0x01 || opcode == 0x29 || opcode == 0x39 || opcode == 0x21 || opcode == 0x09 || opcode == 0x31 || opcode == 0x87) { // Register-register operations
            decoded_instr->length_in_bytes = 2;
            uint8_t modrm = memory.read_text(current_address);
            decodeModRM(modrm, *decoded_instr, false);
        } else if (opcode == 0x8D) { // LEA r32, m
            decoded_instr->length_in_bytes = 2; // Base length
            uint8_t modrm = memory.read_text(current_address);
            uint8_t mod = (modrm >> 6) & 0x03;
            uint8_t reg_field = (modrm >> 3) & 0x07;
            uint8_t rm = modrm & 0x07;

            DecodedOperand dest_reg, src_mem;

            dest_reg.type = OperandType::REGISTER;
            dest_reg.text = getRegisterName(reg_field);

            src_mem.type = OperandType::MEMORY;
            // This is a simplified decoder for [reg] form
            if (mod == 0b00 && rm != 0b101) {
                src_mem.text = "[" + std::string(getRegisterName(rm)) + "]";
            } else {
                // More complex ModR/M forms (with displacement, SIB byte) would be handled here.
                src_mem.text = "[mem]";
            }

            decoded_instr->operands.push_back(dest_reg);
            decoded_instr->operands.push_back(src_mem);
        } else if (opcode == 0xFF) { // INC r/m32
            decoded_instr->length_in_bytes = 2;
            uint8_t modrm = memory.read_text(current_address);
            uint8_t reg_field = (modrm >> 3) & 0x07;
            if (reg_field == 0) decoded_instr->mnemonic = "inc";
            else if (reg_field == 1) decoded_instr->mnemonic = "dec";
            // other group members...

            uint8_t rm = modrm & 0x07;
            DecodedOperand op;
            op.type = OperandType::REGISTER;
            op.text = getRegisterName(rm);
            decoded_instr->operands.push_back(op);
        } else if (opcode == 0xF7) { // Group with MUL, DIV, NOT
            decoded_instr->length_in_bytes = 2;
            uint8_t modrm = memory.read_text(current_address);
            uint8_t reg_field = (modrm >> 3) & 0x07;
            if (reg_field == 2) decoded_instr->mnemonic = "not";
            else if (reg_field == 4) decoded_instr->mnemonic = "mul";
            else if (reg_field == 5) decoded_instr->mnemonic = "imul";
            else if (reg_field == 6) decoded_instr->mnemonic = "div";
            else if (reg_field == 7) decoded_instr->mnemonic = "idiv";
            // other group members...

            uint8_t rm = modrm & 0x07;
            DecodedOperand op;
            op.type = OperandType::REGISTER;
            op.text = getRegisterName(rm);
            decoded_instr->operands.push_back(op);
        } else if (opcode == 0x83) { // Group with ADD, OR, ADC, SBB, AND, SUB, XOR, CMP with imm8
            decoded_instr->length_in_bytes = 3;
            uint8_t modrm = memory.read_text(current_address++);
            uint8_t imm = memory.read_text(current_address);
            uint8_t reg_field = (modrm >> 3) & 0x07;

            // Decode the specific instruction from the /reg field
            if (reg_field == 6) {
                decoded_instr->mnemonic = "xor";
            } else if (reg_field == 7) {
                decoded_instr->mnemonic = "cmp";
            }
            // Other instructions in this group (ADD, OR, etc.) can be added here.


            DecodedOperand reg, imm_op;
            reg.type = OperandType::REGISTER;
            reg.text = getRegisterName(modrm & 0x07);
            imm_op.type = OperandType::IMMEDIATE;
            imm_op.value = imm;
            std::stringstream ss;
            ss << "0x" << std::hex << (int)imm;
            imm_op.text = ss.str();
            decoded_instr->operands.push_back(reg);
            decoded_instr->operands.push_back(imm_op);
        } else if (opcode == 0xC1) { // Group with SHL, SHR, etc. with imm8
            decoded_instr->length_in_bytes = 3;
            uint8_t modrm = memory.read_text(current_address++);
            uint8_t imm = memory.read_text(current_address);
            uint8_t reg_field = (modrm >> 3) & 0x07;

            // Decode the specific instruction from the /reg field
            if (reg_field == 0) {
                decoded_instr->mnemonic = "rol";
            } else if (reg_field == 1) {
                decoded_instr->mnemonic = "ror";
            }else if (reg_field == 4) {
                decoded_instr->mnemonic = "shl";
            } else if (reg_field == 5) {
                decoded_instr->mnemonic = "shr";
            } else if (reg_field == 7) {
                decoded_instr->mnemonic = "sar";
            }
            // Other instructions in this group (SHR, SAR, ROL, etc.) can be added here.

            DecodedOperand reg, imm_op;
            reg.type = OperandType::REGISTER;
            reg.text = getRegisterName(modrm & 0x07);
            imm_op.type = OperandType::IMMEDIATE;
            imm_op.value = imm;
            std::stringstream ss;
            ss << "0x" << std::hex << (int)imm;
            imm_op.text = ss.str();
            decoded_instr->operands.push_back(reg);
            decoded_instr->operands.push_back(imm_op);
        } else if (opcode == 0x75) { // JNE rel8
            decoded_instr->length_in_bytes = 2;
            int8_t offset = memory.read_text(current_address);
            address_t target_address = address + decoded_instr->length_in_bytes + offset;
            DecodedOperand op;
            op.type = OperandType::IMMEDIATE;
            op.value = target_address;
            std::stringstream ss;
            ss << "0x" << std::hex << target_address;
            op.text = ss.str();
            decoded_instr->operands.push_back(op);
            // Operand is the target address
        } else if (opcode == 0x74) { // JE rel8
            decoded_instr->length_in_bytes = 2;
            int8_t offset = memory.read_text(current_address);
            address_t target_address = address + decoded_instr->length_in_bytes + offset;
            DecodedOperand op;
            op.type = OperandType::IMMEDIATE;
            op.value = target_address;
            std::stringstream ss;
            ss << "0x" << std::hex << target_address;
            op.text = ss.str();
            decoded_instr->operands.push_back(op);
        } else if (opcode == 0x72) { // JB rel8
            decoded_instr->length_in_bytes = 2;
            int8_t offset = memory.read_text(current_address);
            address_t target_address = address + decoded_instr->length_in_bytes + offset;
            DecodedOperand op;
            op.type = OperandType::IMMEDIATE;
            op.value = target_address;
            std::stringstream ss;
            ss << "0x" << std::hex << target_address;
            op.text = ss.str();
            decoded_instr->operands.push_back(op);
        } else if (opcode == 0x73) { // JAE rel8
            decoded_instr->length_in_bytes = 2;
            int8_t offset = memory.read_text(current_address);
            address_t target_address = address + decoded_instr->length_in_bytes + offset;
            DecodedOperand op;
            op.type = OperandType::IMMEDIATE;
            op.value = target_address;
            std::stringstream ss;
            ss << "0x" << std::hex << target_address;
            op.text = ss.str();
            decoded_instr->operands.push_back(op);
        } else if (opcode == 0x76) { // JBE rel8
            decoded_instr->length_in_bytes = 2;
            int8_t offset = memory.read_text(current_address);
            address_t target_address = address + decoded_instr->length_in_bytes + offset;
            DecodedOperand op;
            op.type = OperandType::IMMEDIATE;
            op.value = target_address;
            std::stringstream ss;
            ss << "0x" << std::hex << target_address;
            op.text = ss.str();
            decoded_instr->operands.push_back(op);
        } else if (opcode == 0x78) { // JS rel8
            decoded_instr->length_in_bytes = 2;
            int8_t offset = memory.read_text(current_address);
            address_t target_address = address + decoded_instr->length_in_bytes + offset;
            DecodedOperand op;
            op.type = OperandType::IMMEDIATE;
            op.value = target_address;
            std::stringstream ss;
            ss << "0x" << std::hex << target_address;
            op.text = ss.str();
            decoded_instr->operands.push_back(op);
        } else if (opcode == 0x79) { // JNS rel8
            decoded_instr->length_in_bytes = 2;
            int8_t offset = memory.read_text(current_address);
            address_t target_address = address + decoded_instr->length_in_bytes + offset;
            DecodedOperand op;
            op.type = OperandType::IMMEDIATE;
            op.value = target_address;
            std::stringstream ss;
            ss << "0x" << std::hex << target_address;
            op.text = ss.str();
            decoded_instr->operands.push_back(op);
        } else if (opcode == 0x70) { // JO rel8
            decoded_instr->length_in_bytes = 2;
            int8_t offset = memory.read_text(current_address);
            address_t target_address = address + decoded_instr->length_in_bytes + offset;
            DecodedOperand op;
            op.type = OperandType::IMMEDIATE;
            op.value = target_address;
            std::stringstream ss;
            ss << "0x" << std::hex << target_address;
            op.text = ss.str();
            decoded_instr->operands.push_back(op);
        } else if (opcode == 0x71) { // JNO rel8
            decoded_instr->length_in_bytes = 2;
            int8_t offset = memory.read_text(current_address);
            address_t target_address = address + decoded_instr->length_in_bytes + offset;
            DecodedOperand op;
            op.type = OperandType::IMMEDIATE;
            op.value = target_address;
            std::stringstream ss;
            ss << "0x" << std::hex << target_address;
            op.text = ss.str();
            decoded_instr->operands.push_back(op);
        } else if (opcode == 0x7C) { // JL rel8
            decoded_instr->length_in_bytes = 2;
            int8_t offset = memory.read_text(current_address);
            address_t target_address = address + decoded_instr->length_in_bytes + offset;
            DecodedOperand op;
            op.type = OperandType::IMMEDIATE;
            op.value = target_address;
            std::stringstream ss;
            ss << "0x" << std::hex << target_address;
            op.text = ss.str();
            decoded_instr->operands.push_back(op);
        } else if (opcode == 0x7D) { // JGE rel8
            decoded_instr->length_in_bytes = 2;
            int8_t offset = memory.read_text(current_address);
            address_t target_address = address + decoded_instr->length_in_bytes + offset;
            DecodedOperand op;
            op.type = OperandType::IMMEDIATE;
            op.value = target_address;
            std::stringstream ss;
            ss << "0x" << std::hex << target_address;
            op.text = ss.str();
            decoded_instr->operands.push_back(op);
        } else if (opcode == 0x7F) { // JG rel8
            decoded_instr->length_in_bytes = 2;
            int8_t offset = memory.read_text(current_address);
            address_t target_address = address + decoded_instr->length_in_bytes + offset;
            DecodedOperand op;
            op.type = OperandType::IMMEDIATE;
            op.value = target_address;
            std::stringstream ss;
            ss << "0x" << std::hex << target_address;
            op.text = ss.str();
            decoded_instr->operands.push_back(op);
        } else if (opcode == 0xE9 || opcode == 0xE8) { // JMP rel32 or CALL rel32
            decoded_instr->length_in_bytes = 5;
            int32_t offset = memory.read_text_dword(current_address);
            address_t target_address = address + decoded_instr->length_in_bytes + offset;
            DecodedOperand op;
            op.type = OperandType::IMMEDIATE;
            op.value = target_address;
            std::stringstream ss;
            ss << "0x" << std::hex << target_address;
            op.text = ss.str();
            decoded_instr->operands.push_back(op);

        } else if (opcode >= 0x50 && opcode <= 0x57) { // PUSH r32
            decoded_instr->mnemonic = "push";
            decoded_instr->length_in_bytes = 1;
            DecodedOperand reg;
            reg.type = OperandType::REGISTER;
            reg.text = getRegisterName(opcode - 0x50);
            decoded_instr->operands.push_back(reg);
        } else if (opcode >= 0x58 && opcode <= 0x5F) { // POP r32
            decoded_instr->mnemonic = "pop";
            decoded_instr->length_in_bytes = 1;
            DecodedOperand reg;
            reg.type = OperandType::REGISTER;
            reg.text = getRegisterName(opcode - 0x58);
            decoded_instr->operands.push_back(reg);
        } else if (opcode == 0xCD) { // INT imm8
            decoded_instr->length_in_bytes = 2;
            uint8_t imm_value = memory.read_text(current_address);
            DecodedOperand imm;
            imm.type = OperandType::IMMEDIATE;
            imm.value = imm_value;
            std::stringstream ss;
            ss << "0x" << std::hex << (int)imm_value;
            imm.text = ss.str();
            decoded_instr->operands.push_back(imm);
        } else if (opcode == 0xE4) { // IN AL, imm8
            decoded_instr->length_in_bytes = 2;
            uint8_t imm_value = memory.read_text(current_address);
            DecodedOperand reg, imm;
            reg.type = OperandType::REGISTER;
            reg.text = "al";
            imm.type = OperandType::IMMEDIATE;
            imm.value = imm_value;
            std::stringstream ss;
            ss << "0x" << std::hex << (int)imm_value;
            imm.text = ss.str();
            decoded_instr->operands.push_back(reg);
            decoded_instr->operands.push_back(imm);
        } else if (opcode == 0xE6) { // OUT imm8, AL
            decoded_instr->length_in_bytes = 2;
            uint8_t imm_value = memory.read_text(current_address);
            DecodedOperand imm, reg;
            imm.type = OperandType::IMMEDIATE;
            imm.value = imm_value;
            std::stringstream ss;
            ss << "0x" << std::hex << (int)imm_value;
            imm.text = ss.str();
            reg.type = OperandType::REGISTER;
            reg.text = "al";
            decoded_instr->operands.push_back(imm);
            decoded_instr->operands.push_back(reg);
        } else {
            decoded_instr->length_in_bytes = getInstructionLength(opcode);
            // No specific operand decoding for this instruction yet
        }
    }

    return decoded_instr;
}

std::string Decoder::decodeMnemonic(uint8_t instruction_id) const {
    auto it = opcode_to_mnemonic.find(instruction_id);
    if (it != opcode_to_mnemonic.end()) {
        return it->second;
    }
    return "UNKNOWN";
}

DecodedOperand Decoder::decodeOperand(uint64_t encoded_operand) const {
    // This is a placeholder; a real implementation would be much more complex.
    DecodedOperand operand;
    operand.text = "0x" + std::to_string(encoded_operand);
    operand.value = encoded_operand;
    return operand;
}

size_t Decoder::getInstructionLength(uint8_t instruction_id) const {
    auto it = instruction_lengths.find(instruction_id);
    if (it != instruction_lengths.end()) {
        return it->second;
    }
    return 1; // Default length
}

void Decoder::decodeAVXOperands(DecodedInstruction& instr, const VEX_Prefix& vex_prefix, const Memory& memory, address_t opcode_address) {
    uint8_t modrm = memory.read_text(opcode_address + 1);
    uint8_t mod = (modrm >> 6) & 0x03;
    uint8_t reg = (modrm >> 3) & 0x07;
    uint8_t rm  = modrm & 0x07;

    if (instr.mnemonic == "vaddps" || instr.mnemonic == "vdivps" || instr.mnemonic == "vmaxps" || instr.mnemonic == "vpandn" || instr.mnemonic == "vpand" || instr.mnemonic == "vpmullw" || instr.mnemonic == "vminps" || instr.mnemonic == "vpxor" || instr.mnemonic == "vsubps" || instr.mnemonic == "vpor") {
        DecodedOperand dest, src1, src2;
        std::string reg_prefix = vex_prefix.L ? "ymm" : "xmm";

        dest.type = vex_prefix.L ? OperandType::YMM_REGISTER : OperandType::XMM_REGISTER;
        dest.text = reg_prefix + std::to_string(reg);

        src1.type = vex_prefix.L ? OperandType::YMM_REGISTER : OperandType::XMM_REGISTER;
        src1.text = reg_prefix + std::to_string(~vex_prefix.vvvv & 0b1111);

        if (mod == 0b11) { // Register-to-register
            src2.type = vex_prefix.L ? OperandType::YMM_REGISTER : OperandType::XMM_REGISTER;
            src2.text = reg_prefix + std::to_string(rm);
        } else {
            // Memory operand decoding not implemented yet.
        }

        instr.operands.push_back(dest);
        instr.operands.push_back(src1);
        instr.operands.push_back(src2);
    } else if (instr.mnemonic == "vmovups") {
        DecodedOperand op1, op2;
        std::string reg_prefix = vex_prefix.L ? "ymm" : "xmm";

        op1.type = vex_prefix.L ? OperandType::YMM_REGISTER : OperandType::XMM_REGISTER;
        op1.text = reg_prefix + std::to_string(reg);

        if (mod == 0b11) { // Register-to-register
            op2.type = vex_prefix.L ? OperandType::YMM_REGISTER : OperandType::XMM_REGISTER;
            op2.text = reg_prefix + std::to_string(rm);
        } else if (mod == 0b00 && rm == 0b101) { // Memory operand with 32-bit displacement
            // This is RIP-relative addressing. The value in memory is a 32-bit displacement
            // from the address of the *next* instruction.
            op2.type = OperandType::MEMORY;
            int32_t disp = memory.read_text_dword(opcode_address + 2);
            address_t next_instr_addr = instr.address + vex_prefix.bytes + 1 + 1 + 4;
            op2.value = next_instr_addr + disp;
            std::stringstream ss;
            ss << "[0x" << std::hex << op2.value << "]";
            op2.text = ss.str();
        }

        uint8_t vex_opcode = memory.read_text(opcode_address);
        if (vex_opcode == 0x10) { // Load from memory
            instr.operands.push_back(op1); // dest is register
            instr.operands.push_back(op2); // src is memory/register
        } else if (vex_opcode == 0x11) { // Store to memory
            instr.operands.push_back(op2); // dest is memory/register
            instr.operands.push_back(op1); // src is register
        }
    } else if (instr.mnemonic == "vrcpps" || instr.mnemonic == "vsqrtps") {
        DecodedOperand dest, src;
        std::string reg_prefix = vex_prefix.L ? "ymm" : "xmm";

        dest.type = vex_prefix.L ? OperandType::YMM_REGISTER : OperandType::XMM_REGISTER;
        dest.text = reg_prefix + std::to_string(reg);

        if (mod == 0b11) { // Register source
            src.type = vex_prefix.L ? OperandType::YMM_REGISTER : OperandType::XMM_REGISTER;
            src.text = reg_prefix + std::to_string(rm);
        } else if (mod == 0b00 && rm == 0b101) { // Memory source (RIP-relative)
            src.type = OperandType::MEMORY;
            int32_t disp = memory.read_text_dword(opcode_address + 2);
            address_t next_instr_addr = instr.address + vex_prefix.bytes + 1 + 1 + 4;
            src.value = next_instr_addr + disp;
            std::stringstream ss;
            ss << "[0x" << std::hex << src.value << "]";
            src.text = ss.str();
        }

        instr.operands.push_back(dest);
        instr.operands.push_back(src);
    } 
}

VEX_Prefix Decoder::decodeVEXPrefix(const Memory& memory, address_t& address) {
    VEX_Prefix prefix;
    uint8_t byte1 = memory.read_text(address);

    if (byte1 == 0xC5) { // 2-byte VEX
        prefix.bytes = 2;
        uint8_t byte2 = memory.read_text(address + 1);
        prefix.map_select = 1; // Implied 0F
        prefix.L = (byte2 >> 2) & 1;
        prefix.vvvv = (byte2 >> 3) & 0b1111;
        address += 2;
    } else if (byte1 == 0xC4) { // 3-byte VEX
        prefix.bytes = 3;
        uint8_t byte2 = memory.read_text(address + 1);
        uint8_t byte3 = memory.read_text(address + 2);
        prefix.map_select = byte2 & 0b11111;
        prefix.L = (byte3 >> 2) & 1;
        prefix.vvvv = (byte3 >> 3) & 0b1111;
        address += 3;
    } else {
        // This is not a VEX prefix.
        // Return a default-constructed VEX_Prefix or handle error.
    }

    return prefix;
}
