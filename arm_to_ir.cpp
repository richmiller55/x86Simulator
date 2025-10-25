#include "arm_to_ir.h"
#include <sstream>
#include <vector>
#include <stdexcept>

// Helper to trim leading/trailing whitespace
static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

ArmToIrConverter::ArmToIrConverter(const Architecture& arm_arch, const std::map<std::string, address_t>* symbol_table)
    : architecture_(arm_arch), symbol_table_(symbol_table), current_assembly_line_index_(0) {}

IRProgram ArmToIrConverter::convert(const std::string& arm_assembly) {
    IRProgram program;
    current_assembly_line_index_ = 0;
    std::stringstream ss(arm_assembly);
    std::string line;

    while (std::getline(ss, line)) {
        std::string original_line = line;
        line = trim(line);
        if (line.empty() || line[0] == '@' || line.find(':') != std::string::npos) {
            current_assembly_line_index_++;
            continue;
        }

        auto ir_instr = parse_line(line);
        if (ir_instr) {
            ir_instr->original_address = current_assembly_line_index_;
            ir_instr->original_size = 4; // Assuming 4 bytes per ARM instruction
            program.push_back(std::move(ir_instr));
        }
        current_assembly_line_index_++;
    }

    return program;
}

std::unique_ptr<IRInstruction> ArmToIrConverter::parse_line(const std::string& line) {
    std::string processed_line = line;
    size_t comment_pos = processed_line.find('@');
    if (comment_pos != std::string::npos) {
        processed_line = processed_line.substr(0, comment_pos);
    }
    processed_line = trim(processed_line);

    std::stringstream line_ss(processed_line);
    std::string mnemonic;
    line_ss >> mnemonic;

    std::vector<std::string> operands;
    std::string operand_part;
    std::getline(line_ss, operand_part);
    std::stringstream operand_ss(operand_part);
    std::string operand;
    while (std::getline(operand_ss, operand, ',')) {
        operands.push_back(trim(operand));
    }

    for (char& c : mnemonic) { c = tolower(c); }

    if (mnemonic.rfind("v", 0) == 0) {
        return translate_vfp_instruction(mnemonic, operands);
    } else if (mnemonic == "mov" || mnemonic == "add" || mnemonic == "sub" || mnemonic == "cmp" || mnemonic == "orr" || mnemonic == "eor" || mnemonic == "rsb" || mnemonic == "adc" || mnemonic == "sbc" || mnemonic == "rsc" || mnemonic == "tst" || mnemonic == "teq" || mnemonic == "cmn" || mnemonic == "mvn" || mnemonic == "bic" || mnemonic == "clz" || mnemonic == "rbit" || mnemonic == "rev" || mnemonic == "rev16" || mnemonic == "revsh" || mnemonic == "qadd" || mnemonic == "qsub" || mnemonic == "qdadd" || mnemonic == "qdsub" || mnemonic == "mla" || mnemonic == "mls" || mnemonic == "umull" || mnemonic == "smull" || mnemonic == "umlal" || mnemonic == "smlal" || mnemonic == "movw") {
        return translate_data_processing(mnemonic, operands);
    } else if (mnemonic == "ldr" || mnemonic == "str") {
        return translate_load_store(mnemonic, operands);
    } else if (mnemonic[0] == 'b') {
        return translate_branch(mnemonic, operands);
    } else if (mnemonic == "cbnz") {
        return translate_compare_and_branch(mnemonic, operands);
    } else if (mnemonic == "push" || mnemonic == "pop") {
        return translate_push_pop(mnemonic, operands);
    } else if (mnemonic == "swi") {
        return translate_swi(mnemonic, operands);
    } else if (mnemonic == "swp") {
        return translate_swap(mnemonic, operands);
    } else if (mnemonic == "mrs" || mnemonic == "msr") {
        return translate_mrs_msr(mnemonic, operands);
    } else if (mnemonic == "bkpt" || mnemonic == "wfi" || mnemonic == "wfe" || mnemonic == "sev") {
        return translate_special(mnemonic, operands);
    }

    return nullptr;
}

std::string ArmToIrConverter::parse_register(const std::string& reg_str) {
    std::string lower_str = reg_str;
    for (char& c : lower_str) { c = tolower(c); }
    if (architecture_.is_register(lower_str)) {
        return lower_str;
    }
    throw std::runtime_error("Unknown or invalid ARM register: " + reg_str);
}

IROperand ArmToIrConverter::parse_operand(const std::string& operand_str) {
    if (operand_str.empty()) {
        throw std::runtime_error("Empty operand string provided to parse_operand.");
    }

    if (operand_str[0] == '#') {
        std::string immediate_str = operand_str.substr(1);
        if (immediate_str.rfind("0x", 0) == 0 || immediate_str.rfind("0X", 0) == 0) {
            return static_cast<uint64_t>(std::stoull(immediate_str, nullptr, 16));
        } else {
            return static_cast<uint64_t>(std::stoull(immediate_str, nullptr, 10));
        }
    }

    if (operand_str[0] == '[' && operand_str.back() == ']') {
        return parse_memory_operand(operand_str);
    }

    if (symbol_table_ && symbol_table_->count(operand_str)) {
        return symbol_table_->at(operand_str);
    }

    try {
        size_t pos;
        uint64_t value = std::stoull(operand_str, &pos, 0); // 0 for auto-detect base
        if (pos == operand_str.length()) { // Ensure whole string was parsed
            return value;
        }
    } catch (const std::invalid_argument&) {
        // Not a number, proceed to parsing as a register
    } catch (const std::out_of_range&) {
        // Number is out of range for uint64_t, treat as error or specific handling
        throw std::runtime_error("Operand out of range: " + operand_str);
    }

    try {
        return parse_register(operand_str);
    } catch (const std::runtime_error&) {
        throw std::runtime_error("Invalid operand: " + operand_str);
    }
}

IRMemoryOperand ArmToIrConverter::parse_memory_operand(const std::string& mem_str) {
    std::string inner = trim(mem_str.substr(1, mem_str.length() - 2));
    IRMemoryOperand mem_op;
    mem_op.size = 32;

    size_t comma_pos = inner.find(',');
    if (comma_pos == std::string::npos) {
        mem_op.base_reg = parse_register(inner);
    } else {
        std::string base_reg_str = trim(inner.substr(0, comma_pos));
        std::string offset_str = trim(inner.substr(comma_pos + 1));
        mem_op.base_reg = parse_register(base_reg_str);
        if (offset_str[0] == '#') {
            mem_op.displacement = std::stoll(offset_str.substr(1));
        } else {
            mem_op.index_reg = parse_register(offset_str);
        }
    }
    return mem_op;
}

std::unique_ptr<IRInstruction> ArmToIrConverter::translate_data_processing(const std::string& mnemonic, const std::vector<std::string>& operands) {
    IROpcode opcode;
    if (mnemonic == "mov" || mnemonic == "movw") opcode = IROpcode::Move;
    else if (mnemonic == "add") opcode = IROpcode::Add;
    else if (mnemonic == "sub") opcode = IROpcode::Sub;
    else if (mnemonic == "cmp") opcode = IROpcode::Cmp;
    else if (mnemonic == "orr") opcode = IROpcode::Or;
    else if (mnemonic == "eor") opcode = IROpcode::Xor;
    else if (mnemonic == "rsb") opcode = IROpcode::Sub;
    else if (mnemonic == "adc") opcode = IROpcode::AddC;
    else if (mnemonic == "sbc") opcode = IROpcode::SubC;
    else if (mnemonic == "rsc") opcode = IROpcode::SubC;
    else if (mnemonic == "tst") opcode = IROpcode::Tst;
    else if (mnemonic == "teq") opcode = IROpcode::Teq;
    else if (mnemonic == "cmn") opcode = IROpcode::Cmn;
    else if (mnemonic == "mvn") opcode = IROpcode::MoveNot;
    else if (mnemonic == "bic") opcode = IROpcode::AndNot;
    else return nullptr;

    std::vector<IROperand> ir_operands;
    for (const auto& op_str : operands) {
        ir_operands.push_back(parse_operand(op_str));
    }

    if (mnemonic == "rsb" || mnemonic == "rsc") {
        auto op = (mnemonic == "rsb") ? IROpcode::Sub : IROpcode::SubC;
        return std::make_unique<IRInstruction>(op, std::vector<IROperand>{ir_operands[0], ir_operands[2], ir_operands[1]});
    }

    return std::make_unique<IRInstruction>(opcode, std::move(ir_operands));
}

std::unique_ptr<IRInstruction> ArmToIrConverter::translate_load_store(const std::string& mnemonic, const std::vector<std::string>& operands) {
    IROpcode opcode = (mnemonic == "ldr") ? IROpcode::Load : IROpcode::Store;
    if (operands.size() < 2) throw std::runtime_error("Invalid operands for " + mnemonic);

    IROperand op1 = parse_operand(operands[0]);
    IROperand op2 = parse_operand(operands[1]);

    if (opcode == IROpcode::Store) {
        return std::make_unique<IRInstruction>(opcode, std::vector<IROperand>{op2, op1});
    } else {
        return std::make_unique<IRInstruction>(opcode, std::vector<IROperand>{op1, op2});
    }
}

std::unique_ptr<IRInstruction> ArmToIrConverter::translate_branch(const std::string& mnemonic, const std::vector<std::string>& operands) {
    if (operands.empty()) throw std::runtime_error("Missing operand for branch: " + mnemonic);
    IROperand target = parse_operand(operands[0]);
    if (mnemonic == "b") return std::make_unique<IRInstruction>(IROpcode::Jump, std::vector<IROperand>{target});
    if (mnemonic == "bl") return std::make_unique<IRInstruction>(IROpcode::Call, std::vector<IROperand>{target});
    if (mnemonic == "bx") return std::make_unique<IRInstruction>(IROpcode::Jump, std::vector<IROperand>{target});
    if (mnemonic == "blx") return std::make_unique<IRInstruction>(IROpcode::Call, std::vector<IROperand>{target});

    std::string cond_str = mnemonic.substr(1);
    static const std::map<std::string, IRConditionCode> cond_map = {
        {"eq", IRConditionCode::Equal}, {"ne", IRConditionCode::NotEqual},
        {"gt", IRConditionCode::Greater}, {"ge", IRConditionCode::GreaterOrEqual},
        {"lt", IRConditionCode::Less}, {"le", IRConditionCode::LessOrEqual},
        {"hi", IRConditionCode::Above}, {"ls", IRConditionCode::BelowOrEqual},
        {"hs", IRConditionCode::AboveOrEqual}, {"cs", IRConditionCode::AboveOrEqual},
        {"lo", IRConditionCode::Below}, {"cc", IRConditionCode::Below},
        {"mi", IRConditionCode::Sign}, {"pl", IRConditionCode::NotSign},
        {"vs", IRConditionCode::Overflow}, {"vc", IRConditionCode::NotOverflow}
    };
    auto it = cond_map.find(cond_str);
    if (it != cond_map.end()) {
        return std::make_unique<IRInstruction>(IROpcode::Branch, std::vector<IROperand>{target, it->second});
    }
    throw std::runtime_error("Unsupported branch: " + mnemonic);
}

std::unique_ptr<IRInstruction> ArmToIrConverter::translate_compare_and_branch(const std::string& mnemonic, const std::vector<std::string>& operands) {
    if (operands.size() != 2) throw std::runtime_error("Invalid operands for " + mnemonic);
    IROperand reg = parse_operand(operands[0]);
    IROperand label = parse_operand(operands[1]);
    return std::make_unique<IRInstruction>(IROpcode::CompareAndBranchIfNotZero, std::vector<IROperand>{reg, label});
}

std::unique_ptr<IRInstruction> ArmToIrConverter::translate_push_pop(const std::string& mnemonic, const std::vector<std::string>& operands) {
    if (operands.size() != 1) throw std::runtime_error("Only single-register push/pop supported.");
    IROpcode opcode = (mnemonic == "push") ? IROpcode::Push : IROpcode::Pop;
    return std::make_unique<IRInstruction>(opcode, std::vector<IROperand>{parse_operand(operands[0])});
}

std::unique_ptr<IRInstruction> ArmToIrConverter::translate_swi(const std::string& mnemonic, const std::vector<std::string>& operands) {
    if (operands.empty()) throw std::runtime_error("Missing operand for SWI");
    return std::make_unique<IRInstruction>(IROpcode::Syscall, std::vector<IROperand>{parse_operand(operands[0])});
}

std::unique_ptr<IRInstruction> ArmToIrConverter::translate_swap(const std::string& mnemonic, const std::vector<std::string>& operands) {
    if (operands.size() != 3) throw std::runtime_error("Invalid operands for SWP");
    return std::make_unique<IRInstruction>(IROpcode::Swap, std::vector<IROperand>{parse_operand(operands[0]), parse_operand(operands[1]), parse_operand(operands[2])});
}

std::unique_ptr<IRInstruction> ArmToIrConverter::translate_mrs_msr(const std::string& mnemonic, const std::vector<std::string>& operands) {
    if (operands.size() != 2) throw std::runtime_error("Invalid operands for " + mnemonic);
    if (mnemonic == "mrs") {
        return std::make_unique<IRInstruction>(IROpcode::MoveFromSystemRegister, std::vector<IROperand>{parse_operand(operands[0]), parse_operand(operands[1])});
    } else {
        return std::make_unique<IRInstruction>(IROpcode::MoveToSystemRegister, std::vector<IROperand>{parse_operand(operands[0]), parse_operand(operands[1])});
    }
}

std::unique_ptr<IRInstruction> ArmToIrConverter::translate_special(const std::string& mnemonic, const std::vector<std::string>& operands) {
    IROpcode opcode;
    if (mnemonic == "bkpt") opcode = IROpcode::Breakpoint;
    else if (mnemonic == "wfi") opcode = IROpcode::WaitForInterrupt;
    else if (mnemonic == "wfe") opcode = IROpcode::WaitForEvent;
    else if (mnemonic == "sev") opcode = IROpcode::SendEvent;
    else return nullptr;
    return std::make_unique<IRInstruction>(opcode, std::vector<IROperand>{});
}

std::unique_ptr<IRInstruction> ArmToIrConverter::translate_vfp_instruction(const std::string& mnemonic, const std::vector<std::string>& operands) {
    size_t dot_pos = mnemonic.find('.');
    if (dot_pos == std::string::npos) return nullptr;

    std::string base_mnemonic = mnemonic.substr(0, dot_pos);
    std::string type_suffix = mnemonic.substr(dot_pos + 1);

    bool is_double = (type_suffix == "f64");

    IROpcode opcode;
    if (base_mnemonic == "vadd") opcode = is_double ? IROpcode::FloatAddD : IROpcode::FloatAddS;
    else if (base_mnemonic == "vsub") opcode = is_double ? IROpcode::FloatSubD : IROpcode::FloatSubS;
    else if (base_mnemonic == "vmul") opcode = is_double ? IROpcode::FloatMulD : IROpcode::FloatMulS;
    else if (base_mnemonic == "vdiv") opcode = is_double ? IROpcode::FloatDivD : IROpcode::FloatDivS;
    else if (base_mnemonic == "vsqrt") opcode = is_double ? IROpcode::FloatSqrtD : IROpcode::FloatSqrtS;
    else if (base_mnemonic == "vcmp") opcode = is_double ? IROpcode::FloatCmpD : IROpcode::FloatCmpS;
    else return nullptr;

    std::vector<IROperand> ir_operands;
    for (const auto& op_str : operands) {
        ir_operands.push_back(parse_operand(op_str));
    }

    return std::make_unique<IRInstruction>(opcode, std::move(ir_operands));
}
