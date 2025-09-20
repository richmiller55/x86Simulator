
#ifndef PROGRAM_DECODER_H
#define PROGRAM_DECODER_H

#include <vector>
#include <map>
#include <memory>
#include "memory.h"
#include "decoder.h"

class ProgramDecoder {
public:
    ProgramDecoder(const Memory& memory);
    void decode();
    const std::vector<std::unique_ptr<DecodedInstruction>>& getDecodedProgram() const;
    const std::map<address_t, size_t>& getAddressToIndexMap() const;

private:
    const Memory& memory_;
    std::vector<std::unique_ptr<DecodedInstruction>> decoded_program_;
    std::map<address_t, size_t> address_to_index_map_;
};

#endif // PROGRAM_DECODER_H
