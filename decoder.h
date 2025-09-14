#ifndef DECODER_H
#define DECODER_H

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include "x86_simulator.h" // For address_t

// Forward declarations if needed, though included above
// struct DecodedInstruction;
// struct DecodedOperand;

// Represents a single decoded operand
struct DecodedOperand {
  std::string text;
  uint64_t value; // The raw value of the operand
  OperandType type;
  // Add more details, e.g., type (immediate, register, memory)
};

// Represents a single fully decoded instruction
struct DecodedInstruction {
  address_t address;
  std::string mnemonic;
  std::vector<DecodedOperand> operands;
  size_t length_in_bytes;
};

// The core Decoder class
class Decoder {
private:
  // Helper function to decode the opcode to a mnemonic.
  Decoder();
  // Private static instance
  static Decoder* instance;

  const std::map<uint8_t, std::string> opcode_to_mnemonic = {
    {0x90, "NOP"},
    {0x66, "TWO_BYTE_OPCODE_PREFIX"}, // Example prefix
    {0x5D, "POP"},
    {0x55, "PUSH"},
    {0x01, "ADD"},
    {0x29, "SUB"},
    {0xEB, "JMP"},
    {0x09, "OR"},
    {0x31, "XOR"},
    {0x21, "AND"},
    {0x39, "CMP"},
    {0x75, "JNE"},
    {0xB8, "MOV"},
    {0x40, "INC"}
  };

  const std::map<std::string, uint8_t> mnemonic_to_opcode = {
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
    // ... more instructions
  };

  const std::map<uint8_t, size_t> instruction_lengths = {
    {0x90, 1}, // NOP
    {0xB8, 5}, // MOV EAX, imm32
    {0x55, 1}, // PUSH EBP
    {0x5D, 1}, // POP EBP
    
    // ADD (Register-to-Register)
    {0x01, 2}, // ADD r/m32, r32 (e.g., ADD EAX, EBX)

    // SUB (Register-to-Register)
    {0x29, 2}, // SUB r/m32, r32 (e.g., SUB EAX, EBX)

    // JMP (Relative 8-bit offset)
    {0xEB, 2}, // JMP rel8 (1 byte opcode + 1 byte immediate)

    // OR (Register-to-Register)
    {0x09, 2}, // OR r/m32, r32 (e.g., OR EAX, EBX)

    // XOR (Register-to-Register)
    {0x31, 2}, // XOR r/m32, r32 (e.g., XOR EAX, EBX)

    // AND (Register-to-Register)
    {0x21, 2}, // AND r/m32, r32 (e.g., AND EAX, EBX)

    // CMP (Register-to-Register)
    {0x39, 2}, // CMP r/m32, r32 (e.g., CMP EAX, EBX)

    // JNE (Relative 8-bit offset)
    {0x75, 2}, // JNE rel8 (1 byte opcode + 1 byte immediate)

    // INC (Register)
    // Note: In 64-bit mode, 0x4x bytes are REX prefixes.
    // Use the ModR/M version (opcode FF /0) for 32-bit and 64-bit portability.
    {0x40, 1}, // INC EAX (Legacy form, not used in 64-bit mode)
    // You will need a more complex decoder for ModR/M opcodes like FF /0.
    
    // DEC (Register, similar to INC)
    {0x48, 1}, // DEC EAX (Legacy form, not used in 64-bit mode)
  };


public:
  // Decodes the instruction at the given address in memory.

  Decoder(const Decoder&) = delete;
  Decoder(Decoder&&) = delete;
  Decoder& operator=(const Decoder&) = delete;
  Decoder& operator=(Decoder&&) = delete;

  // Static method to get the single instance
  static Decoder& getInstance();
  // Helper function to decode an encoded operand.
  DecodedOperand decodeOperand(uint64_t encoded_operand) const;
  std::string getMnemonic(uint8_t opcode) const;
  std::string decodeMnemonic(uint8_t instruction_id) const;
  std::optional<DecodedInstruction> decodeInstruction(const Memory& memory, address_t address);
  uint8_t getOpcode(const std::string& mnemonic) const;

  size_t getInstructionLength(uint8_t instruction_id) const;
};

#endif // DECODER_H