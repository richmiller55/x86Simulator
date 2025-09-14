#include "decoder.h"
#include <map>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <iomanip>

Decoder* Decoder::instance = nullptr;

Decoder::Decoder() {
 }

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

    // Read the primary opcode byte from memory.
    uint8_t opcode = memory.read_text(address);
    address_t current_address = address + 1;

    // Simplified logic for prefixes and multi-byte opcodes.
    // Example: Handle a simple two-byte opcode prefix.
    if (opcode == 0x66) {
        if (current_address >= memory.data_segment_start) {
            return std::nullopt; // Incomplete instruction
        }
        opcode = memory.read_text(current_address);
        current_address++;
    }

    // Decode the mnemonic.
    decoded_instr.mnemonic = decodeMnemonic(opcode);

    // If mnemonic is "UNKNOWN", return an empty optional.
    if (decoded_instr.mnemonic == "UNKNOWN") {
      return std::nullopt;
    }

    // Decode operands based on the opcode.
    if (opcode >= 0xB8 && opcode <= 0xBF) { // MOV r32, imm32
        if (current_address + 3 >= memory.data_segment_start) { // Need 4 bytes for imm32
            return std::nullopt; // Incomplete instruction
        }
        uint32_t imm_value = 0;
        imm_value |= (memory.read_text(current_address + 0) & 0xFF) << 0;
        imm_value |= (memory.read_text(current_address + 1) & 0xFF) << 8;
        imm_value |= (memory.read_text(current_address + 2) & 0xFF) << 16;
        imm_value |= (memory.read_text(current_address + 3) & 0xFF) << 24;

        DecodedOperand operand;
        std::stringstream ss;
        ss << "0x" << std::hex << imm_value;
        operand.text = ss.str();
        operand.value = imm_value;
        decoded_instr.operands.push_back(operand);
        decoded_instr.length_in_bytes = 1 + 4; // 1 byte opcode + 4 bytes immediate
    } else {
      // Handle other instructions or assume zero operands for simplicity.
      decoded_instr.length_in_bytes = getInstructionLength(opcode);
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
    // Return a default length for unknown or complex instructions.
    return 1;
  }