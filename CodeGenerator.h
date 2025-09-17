#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "x86_simulator.h"
#include "decoder.h"
#include <vector>
#include <string>

class CodeGenerator {
public:
    CodeGenerator(Memory& memory, const std::map<std::string, address_t>& symbol_table);
    size_t generate_code(const std::vector<std::string>& program_lines);

private:
    Memory& memory_;
    const std::map<std::string, address_t>& symbol_table_;
    address_t current_address_;

    void process_line(const std::string& line);
    std::vector<std::string> parse_line(const std::string& line);
    std::string trim(const std::string& str);
};

#endif // CODE_GENERATOR_H
