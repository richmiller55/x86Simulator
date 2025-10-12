#include "arm_to_ir.h"
#include <sstream>
#include <vector>
#include <stdexcept>

// A helper function to trim whitespace from a string
static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// A helper to parse a register string like "r0" or "sp" into an IRRegister
// This is a simplified parser for this initial phase.
static IRRegister parse_register(const std::string& reg_str, const Architecture& arch) {
    // In a real implementation, we would do a reverse lookup on the architecture's register map.
    // For now, we'll parse it manually.
    std::string lower_str = reg_str;
    for (char& c : lower_str) { c = tolower(c); }

    if (lower_str == "sp") return {IRRegisterType::GPR, 13, 32};
    if (lower_str == "lr") return {IRRegisterType::GPR, 14, 32};
    if (lower_str == "pc") return {IRRegisterType::IP, 0, 32};

    if (lower_str[0] == 'r') {
        uint32_t index = std::stoul(lower_str.substr(1));
        return {IRRegisterType::GPR, index, 32}; // Assuming 32-bit for now
    }
    throw std::runtime_error("Unknown register: " + reg_str);
}


ArmToIrConverter::ArmToIrConverter(const Architecture& arm_arch)
    : architecture_(arm_arch) {}

IRProgram ArmToIrConverter::convert(const std::string& arm_assembly) {
    IRProgram program;
    std::stringstream ss(arm_assembly);
    std::string line;
    uint64_t current_address = 0; // Simplified address tracking

    while (std::getline(ss, line)) {
        // Basic cleanup
        line = trim(line);
        if (line.empty() || line[0] == '@' || line[0] == '.') {
            continue; // Skip comments, directives, and empty lines
        }

        // Handle labels
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            // For now, we just ignore the label but it would be stored in a map
            // from label name to address/index in a real implementation.
            line = trim(line.substr(colon_pos + 1));
            if (line.empty()) continue;
        }

        std::string mnemonic;
        std::stringstream line_ss(line);
        line_ss >> mnemonic;

        std::vector<std::string> operands;
        std::string operand;
        while (line_ss >> operand) {
            if (operand.back() == ',') operand.pop_back();
            operands.push_back(operand);
        }

        // This is a highly simplified parser for demonstration purposes.
        if (mnemonic == "mov") {
            IRRegister dest = parse_register(operands[0], architecture_);
            uint64_t immediate = std::stoull(operands[1].substr(1)); // Remove #
            program.push_back(std::make_unique<IRInstruction>(IROpcode::Move, std::vector<IROperand>{dest, immediate}));
        } else if (mnemonic == "add") {
            IRRegister dest = parse_register(operands[0], architecture_);
            IRRegister src1 = parse_register(operands[1], architecture_);
            IRRegister src2 = parse_register(operands[2], architecture_);
            program.push_back(std::make_unique<IRInstruction>(IROpcode::Add, std::vector<IROperand>{dest, src1, src2}));
        } else if (mnemonic == "sub") {
            IRRegister dest = parse_register(operands[0], architecture_);
            IRRegister src = parse_register(operands[1], architecture_);
            uint64_t immediate = std::stoull(operands[2].substr(1)); // Remove #
            program.push_back(std::make_unique<IRInstruction>(IROpcode::Sub, std::vector<IROperand>{dest, src, immediate}));
        } else if (mnemonic == "ldr") {
            IRRegister dest = parse_register(operands[0], architecture_);
            std::string label = operands[1].substr(1); // Remove =
            program.push_back(std::make_unique<IRInstruction>(IROpcode::Load, std::vector<IROperand>{dest, label}));
        } else if (mnemonic == "str") {
            IRRegister src = parse_register(operands[0], architecture_);
            std::string mem_op_str = operands[1].substr(1, operands[1].length() - 2); // Remove [ and ]
            IRRegister base_reg = parse_register(mem_op_str, architecture_);
            IRMemoryOperand mem_dest;
            mem_dest.base_reg = base_reg;
            mem_dest.size = 32;
            program.push_back(std::make_unique<IRInstruction>(IROpcode::Store, std::vector<IROperand>{mem_dest, src}));
        } else if (mnemonic == "b") {
            program.push_back(std::make_unique<IRInstruction>(IROpcode::Jump, std::vector<IROperand>{operands[0]}));
        }

        // In a real implementation, instruction size would be determined by the instruction itself.
        // For ARM, it's almost always 4 bytes.
        if (!program.empty()) {
            program.back()->original_address = current_address;
            program.back()->original_size = 4;
            current_address += 4;
        }
    }

    return program;
}
