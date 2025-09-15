#include "decoder.h"
#include <map>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <iomanip>

Decoder* Decoder::instance = nullptr;

// Helper to get 32-bit register name from index
const char* getRegisterName(uint8_t index) {
    static const char* registers[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
    if (index < 8) return registers[index];
    return "err";
}

// Helper to decode ModR/M byte
void decodeModRM(uint8_t modrm, DecodedInstruction& instr) {
    uint8_t mod = (modrm >> 6) & 0x03;
    uint8_t reg = (modrm >> 3) & 0x07;
    uint8_t rm = modrm & 0x07;

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

Decoder::Decoder() {}

Decoder& Decoder::getInstance() {
    if (instance == nullptr) {
        instance = new Decoder();
    }
    return *instance;
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

std::optional<DecodedInstruction> Decoder::decodeInstruction(const Memory& memory, address_t address) {
    if (address >= memory.data_segment_start) {
        return std::nullopt;
    }

    DecodedInstruction decoded_instr;
    decoded_instr.address = address;
    uint8_t opcode = memory.read_text(address);
    address_t current_address = address + 1;

    decoded_instr.mnemonic = decodeMnemonic(opcode);
    if (decoded_instr.mnemonic == "UNKNOWN") {
        return std::nullopt;
    }

    decoded_instr.length_in_bytes = getInstructionLength(opcode);

    if (opcode >= 0xB8 && opcode <= 0xBF) { // MOV r32, imm32
        uint32_t imm_value = memory.read_text_dword(current_address);
        DecodedOperand reg, imm;
        reg.type = OperandType::REGISTER;
        reg.text = getRegisterName(opcode - 0xB8);
        imm.type = OperandType::IMMEDIATE;
        imm.value = imm_value;
        std::stringstream ss;
        ss << "0x" << std::hex << imm_value;
        imm.text = ss.str();
        decoded_instr.operands.push_back(reg);
        decoded_instr.operands.push_back(imm);
    } else if (opcode == 0x89 || opcode == 0x01 || opcode == 0x29 || opcode == 0x39) { // Register-register operations
        uint8_t modrm = memory.read_text(current_address);
        decodeModRM(modrm, decoded_instr);
    } else if (opcode == 0xFF) { // INC r/m32
        uint8_t modrm = memory.read_text(current_address);
        uint8_t rm = modrm & 0x07;
        DecodedOperand op;
        op.type = OperandType::REGISTER;
        op.text = getRegisterName(rm);
        decoded_instr.operands.push_back(op);
    } else if (opcode == 0x83) { // CMP r/m32, imm8
        uint8_t modrm = memory.read_text(current_address++);
        uint8_t imm = memory.read_text(current_address);
        DecodedOperand reg, imm_op;
        reg.type = OperandType::REGISTER;
        reg.text = getRegisterName(modrm & 0x07);
        imm_op.type = OperandType::IMMEDIATE;
        imm_op.value = imm;
        std::stringstream ss;
        ss << "0x" << std::hex << (int)imm;
        imm_op.text = ss.str();
        decoded_instr.operands.push_back(reg);
        decoded_instr.operands.push_back(imm_op);
    } else if (opcode == 0x75) { // JNE rel8
        int8_t offset = memory.read_text(current_address);
        // Operand is the target address
    } else if (opcode == 0xE9) { // JMP rel32
        int32_t offset = memory.read_text_dword(current_address);
        // Operand is the target address
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
