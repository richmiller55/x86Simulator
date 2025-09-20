#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "memory.h" // For address_t
#include "decoder.h"
#include <vector>
#include <string>

class CodeGenerator {
public:
    CodeGenerator(std::map<std::string, address_t>& symbol_table);
    std::vector<uint8_t> generate_code(const std::vector<std::string>& program_lines);

private:
    const std::map<std::string, address_t>& symbol_table_;
    address_t current_address_;
    std::vector<uint8_t> machine_code_;

    void process_line(const std::string& line);
    std::vector<std::string> parse_line(const std::string& line);
    std::string trim(const std::string& str);
};

#endif // CODE_GENERATOR_H
