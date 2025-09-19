#include "decoder.h"
#include <map>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <iomanip>

std::unique_ptr<Decoder> Decoder::instance;

// Helper to get 32-bit register name from index
const char* getRegisterName(uint8_t index) {
    static const char* registers[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
    if (index < 8) return registers[index];
    return "err";
}

// Helper to decode ModR/M byte
void decodeModRM(uint8_t modrm, DecodedInstruction& instr) {
    [[maybe_unused]] uint8_t mod = (modrm >> 6) & 0x03;
    [[maybe_unused]] uint8_t reg = (modrm >> 3) & 0x07;
    uint8_t rm  = modrm & 0x07;

    if (mod == 0b11) { // Register-to-register
        DecodedOperand dest, src;
        dest.type = OperandType::REGISTER;
        dest.text = getRegisterName(rm);
        src.type = OperandType::REGISTER;
        src.text = getRegisterName(reg);
        instr.operands.push_back(dest);
        instr.operands.push_back(src);
    }
    // Other modes (memory access) not implemented for simplicity
}

Decoder::Decoder() {
    opcode_to_mnemonic = {
        {0x90, "NOP"},
        {0x66, "TWO_BYTE_OPCODE_PREFIX"}, // Example prefix
        {0x5D, "POP"},
        {0x55, "PUSH"},
        {0x01, "ADD"},
        {0x29, "SUB"},
        {0xEB, "JMP"},
        {0xE9, "JMP"},
        {0x09, "OR"},
        {0x31, "XOR"},
        {0x21, "AND"},
        {0x39, "CMP"},
        {0x83, "CMP"},
        {0x75, "JNE"},
        {0xB8, "MOV"},
        {0xB9, "MOV"},
        {0xBB, "MOV"},
        {0x89, "MOV"},
        {0xF7, "GROUP_F7"}, // MUL, NOT, DIV, etc.
        {0xFF, "GROUP_FF"}, // INC, DEC
        // The below are for simple cases, but the group is more accurate
        {0x40, "INC"}, 
        {0xFF, "INC"},
        {0xCD, "INT"}
    };

    vex_opcode_to_mnemonic = {
        // VZEROUPPER
        {{1, 0x77}, "VZEROUPPER"},
        // VADDPS
        {{1, 0x58}, "VADDPS"},
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
        {"MOV", 0xB8},
        {"INC", 0x40},
        {"INT", 0xCD}
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
        {0x09, 2}, // OR r/m32, r32
        {0x31, 2}, // XOR r/m32, r32
        {0x21, 2}, // AND r/m32, r32
        {0x39, 2}, // CMP r/m32, r32
        {0x83, 3}, // CMP r/m32, imm8
        {0x75, 2}, // JNE rel8
        {0x40, 1}, // INC EAX (Legacy)
        {0xFF, 2}, // INC r/m32
        {0x48, 1}, // DEC EAX (Legacy)
        {0xF7, 2}, // Group F7
        {0xCD, 2}  // INT imm8
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
        if (auto it = vex_opcode_to_mnemonic.find({vex_prefix.map_select, vex_opcode}); it != vex_opcode_to_mnemonic.end()) {
            decoded_instr->mnemonic = it->second;
        } else {
            decoded_instr->mnemonic = "AVX_INSTRUCTION_UNKNOWN";
        }
        decodeAVXOperands(*decoded_instr, vex_prefix, memory, current_address);
        // The length will be determined by the specific AVX instruction.
        // This is just a placeholder.
        decoded_instr->length_in_bytes = vex_prefix.bytes + 2; 

    } else {
        // Existing logic for non-AVX instructions
        current_address++;
        decoded_instr->mnemonic = decodeMnemonic(opcode);
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
        } else if (opcode == 0x89 || opcode == 0x01 || opcode == 0x29 || opcode == 0x39 || opcode == 0x21 || opcode == 0x09 || opcode == 0x31) { // Register-register operations
            decoded_instr->length_in_bytes = 2;
            uint8_t modrm = memory.read_text(current_address);
            decodeModRM(modrm, *decoded_instr);
        } else if (opcode == 0xFF) { // INC r/m32
            decoded_instr->length_in_bytes = 2;
            uint8_t modrm = memory.read_text(current_address);
            uint8_t reg_field = (modrm >> 3) & 0x07;
            if (reg_field == 0) decoded_instr->mnemonic = "INC";
            else if (reg_field == 1) decoded_instr->mnemonic = "DEC";
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
            if (reg_field == 2) decoded_instr->mnemonic = "NOT";
            else if (reg_field == 4) decoded_instr->mnemonic = "MUL";
            else if (reg_field == 6) decoded_instr->mnemonic = "DIV";
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
                decoded_instr->mnemonic = "XOR";
            } else if (reg_field == 7) {
                decoded_instr->mnemonic = "CMP";
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
        } else if (opcode == 0xE9) { // JMP rel32
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
            // Operand is the target address
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

    if (instr.mnemonic == "VADDPS") {
        DecodedOperand dest, src1, src2;

        dest.type = OperandType::YMM_REGISTER;
        dest.text = "ymm" + std::to_string(reg);

        src1.type = OperandType::YMM_REGISTER;
        src1.text = "ymm" + std::to_string(vex_prefix.vvvv);

        if (mod == 0b11) { // Register-to-register
            src2.type = OperandType::YMM_REGISTER;
            src2.text = "ymm" + std::to_string(rm);
        } else {
            // Memory operand decoding not implemented yet.
        }

        instr.operands.push_back(dest);
        instr.operands.push_back(src1);
        instr.operands.push_back(src2);
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
        prefix.vvvv = (~byte2 >> 3) & 0b1111;
        address += 2;
    } else if (byte1 == 0xC4) { // 3-byte VEX
        prefix.bytes = 3;
        uint8_t byte2 = memory.read_text(address + 1);
        uint8_t byte3 = memory.read_text(address + 2);
        prefix.map_select = byte2 & 0b11111;
        prefix.L = (byte3 >> 2) & 1;
        prefix.vvvv = (~byte3 >> 3) & 0b1111;
        address += 3;
    } else {
        // This is not a VEX prefix.
        // Return a default-constructed VEX_Prefix or handle error.
    }

    return prefix;
}
