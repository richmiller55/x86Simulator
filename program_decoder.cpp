
#include "program_decoder.h"

ProgramDecoder::ProgramDecoder(const Memory& memory) : memory_(memory) {}

void ProgramDecoder::decode() {
    // Implementation of decode
}

void ProgramDecoder::add_instruction(std::shared_ptr<DecodedInstruction> instr) {
    address_to_index_map_[instr->address] = decoded_program_.size();
    decoded_program_.push_back(std::move(instr));
}

const std::vector<std::unique_ptr<DecodedInstruction>>& ProgramDecoder::getDecodedProgram() const {
    return decoded_program_;
}

const std::map<address_t, size_t>& ProgramDecoder::getAddressToIndexMap() const {
    return address_to_index_map_;
}
