#ifndef DECODER_H
#define DECODER_H

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <map>
#include "memory.h" // For address_t
#include "operand_types.h" // For address_t

// Represents a VEX prefix, which is used for AVX instructions.
struct VEX_Prefix {
    int bytes; // 2 or 3
    int map_select; // Implied 0F, 0F 38, or 0F 3A
    int L; // Vector length
    int vvvv; // Non-destructive source register
};

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
  static std::unique_ptr<Decoder> instance;

  std::map<uint8_t, std::string> opcode_to_mnemonic;
  std::map<uint8_t, std::string> two_byte_opcode_to_mnemonic;
  std::map<std::tuple<int, uint8_t>, std::string> vex_opcode_to_mnemonic;
  std::map<std::string, uint8_t> mnemonic_to_opcode;
  std::map<uint8_t, size_t> instruction_lengths;

public:
  // Decodes the instruction at the given address in memory.

  Decoder(const Decoder&) = delete;
  Decoder(Decoder&&) = delete;
  Decoder& operator=(const Decoder&) = delete;
  Decoder& operator=(Decoder&&) = delete;
  ~Decoder() = default;

  // Static method to get the single instance
  static Decoder& getInstance();
  // Static method to reset the single instance (for testing)
  static void resetInstance();

  // Helper function to decode an encoded operand.
  DecodedOperand decodeOperand(uint64_t encoded_operand) const;
  std::string getMnemonic(uint8_t opcode) const;
  std::string decodeMnemonic(uint8_t instruction_id) const;  
  std::unique_ptr<DecodedInstruction> decodeInstruction(const Memory& memory, address_t address);
  VEX_Prefix decodeVEXPrefix(const Memory& memory, address_t& address);
  void decodeAVXOperands(DecodedInstruction& instr, const VEX_Prefix& vex_prefix, const Memory& memory, address_t opcode_address);
  uint8_t getOpcode(const std::string& mnemonic) const;

  size_t getInstructionLength(uint8_t instruction_id) const;
};

#endif // DECODER_H
