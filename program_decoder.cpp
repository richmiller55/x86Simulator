
#include "program_decoder.h"

ProgramDecoder::ProgramDecoder(const Memory& memory) : memory_(memory) {}

void ProgramDecoder::decode() {
    Decoder& decoder = Decoder::getInstance();
    address_t addr = memory_.get_text_segment_start();
    size_t index = 0;
    while (addr < memory_.get_text_segment_start() + memory_.get_text_segment_size()) {
        if (auto decoded_instr_opt = decoder.decodeInstruction(memory_, addr)) {
            address_to_index_map_[addr] = index++;
            addr += decoded_instr_opt->length_in_bytes;
            decoded_program_.push_back(std::move(decoded_instr_opt));
        } else {
            addr++;
        }
    }
}

const std::vector<std::unique_ptr<DecodedInstruction>>& ProgramDecoder::getDecodedProgram() const {
    return decoded_program_;
}

const std::map<address_t, size_t>& ProgramDecoder::getAddressToIndexMap() const {
    return address_to_index_map_;
}
