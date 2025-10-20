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

ArmToIrConverter::ArmToIrConverter(const Architecture& arm_arch)
    : architecture_(arm_arch), current_address_(0) {}

IRProgram ArmToIrConverter::convert(const std::string& arm_assembly) {
    first_pass(arm_assembly);
    return second_pass(arm_assembly);
}

void ArmToIrConverter::first_pass(const std::string& arm_assembly) {
    symbol_table_.clear();
    current_address_ = 0;
    std::stringstream ss(arm_assembly);
    std::string line;

    while (std::getline(ss, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '@') continue; // Skip comments and empty lines

        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string label = trim(line.substr(0, colon_pos));
            symbol_table_[label] = current_address_;
        }

        // For simplicity, assume all instructions are 4 bytes. A real implementation
        // would need to properly decode the instruction to find its length.
        current_address_ += 4;
    }
}

IRProgram ArmToIrConverter::second_pass(const std::string& arm_assembly) {
    IRProgram program;
    current_address_ = 0;
    std::stringstream ss(arm_assembly);
    std::string line;

    while (std::getline(ss, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '@') continue;

        // Strip label from line if it exists
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            line = trim(line.substr(colon_pos + 1));
        }
        if (line.empty()) continue;

        auto ir_instr = parse_line(line);
        if (ir_instr) {
            ir_instr->original_address = current_address_;
            ir_instr->original_size = 4; // Assume 4-byte instructions
            program.push_back(std::move(ir_instr));
        }

        current_address_ += 4;
    }
    return program;
}

std::unique_ptr<IRInstruction> ArmToIrConverter::parse_line(const std::string& line) {
    std::stringstream line_ss(line);
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

    // Convert mnemonic to lower case for consistent matching
    for (char& c : mnemonic) { c = tolower(c); }

    if (mnemonic == "mov" || mnemonic == "add" || mnemonic == "sub" || mnemonic == "cmp" || mnemonic == "orr" || mnemonic == "eor" || mnemonic == "rsb" || mnemonic == "adc" || mnemonic == "sbc" || mnemonic == "rsc" || mnemonic == "tst" || mnemonic == "teq" || mnemonic == "cmn" || mnemonic == "mvn" || mnemonic == "bic" || mnemonic == "clz" || mnemonic == "rbit" || mnemonic == "rev" || mnemonic == "rev16" || mnemonic == "revsh" || mnemonic == "qadd" || mnemonic == "qsub" || mnemonic == "qdadd" || mnemonic == "qdsub" || mnemonic == "mla" || mnemonic == "mls" || mnemonic == "umull" || mnemonic == "smull" || mnemonic == "umlal" || mnemonic == "smlal" || mnemonic == "movw") {
        return translate_data_processing(mnemonic, operands);
    } else if (mnemonic == "ldr" || mnemonic == "str") {
        return translate_load_store(mnemonic, operands);
    } else if (mnemonic[0] == 'b') { // b, bl, beq, bx, blx, etc.
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

std::unique_ptr<IRInstruction> ArmToIrConverter::translate_compare_and_branch(const std::string& mnemonic, const std::vector<std::string>& operands) {
    if (operands.size() != 2) {
        throw std::runtime_error("Invalid number of operands for " + mnemonic);
    }

    IROperand reg = parse_operand(operands[0]);
    IROperand label = parse_operand(operands[1]);

    if (mnemonic == "cbnz") {
        return std::make_unique<IRInstruction>(IROpcode::CompareAndBranchIfNotZero, std::vector<IROperand>{reg, label});
    }

    return nullptr;
}

std::unique_ptr<IRInstruction> ArmToIrConverter::translate_special(const std::string& mnemonic, const std::vector<std::string>& operands) {
    IROpcode opcode;
    if (mnemonic == "bkpt") {
        opcode = IROpcode::Breakpoint;
    } else if (mnemonic == "wfi") {
        opcode = IROpcode::WaitForInterrupt;
    } else if (mnemonic == "wfe") {
        opcode = IROpcode::WaitForEvent;
    } else if (mnemonic == "sev") {
        opcode = IROpcode::SendEvent;
    } else {
        return nullptr;
    }

    std::vector<IROperand> ir_operands;
    for (const auto& op_str : operands) {
        if (!op_str.empty()) {
            ir_operands.push_back(parse_operand(op_str));
        }
    }

    return std::make_unique<IRInstruction>(opcode, std::move(ir_operands));
}

std::unique_ptr<IRInstruction> ArmToIrConverter::translate_swi(const std::string& mnemonic, const std::vector<std::string>& operands) {
    if (operands.empty()) {
        throw std::runtime_error("Missing operand for SWI instruction");
    }

    // The operand is an immediate value, which we pass to the Syscall IR.
    IROperand interrupt_vector = parse_operand(operands[0]);

    return std::make_unique<IRInstruction>(IROpcode::Syscall, std::vector<IROperand>{interrupt_vector});
}

std::unique_ptr<IRInstruction> ArmToIrConverter::translate_swap(const std::string& mnemonic, const std::vector<std::string>& operands) {
    if (operands.size() != 3) {
        throw std::runtime_error("Invalid number of operands for SWP");
    }

    IROperand rd = parse_operand(operands[0]);
    IROperand rm = parse_operand(operands[1]);
    IROperand rn_mem = parse_operand(operands[2]);

    return std::make_unique<IRInstruction>(IROpcode::Swap, std::vector<IROperand>{rd, rm, rn_mem});
}

std::unique_ptr<IRInstruction> ArmToIrConverter::translate_mrs_msr(const std::string& mnemonic, const std::vector<std::string>& operands) {
    if (operands.size() != 2) {
        throw std::runtime_error("Invalid number of operands for " + mnemonic);
    }

    if (mnemonic == "mrs") {
        // MRS Rd, CPSR -> MoveFromSystemRegister(Rd, "cpsr")
        IROperand rd = parse_operand(operands[0]);
        std::string sys_reg = operands[1];
        for (char& c : sys_reg) { c = tolower(c); }
        return std::make_unique<IRInstruction>(IROpcode::MoveFromSystemRegister, std::vector<IROperand>{rd, sys_reg});
    } else { // msr
        // MSR CPSR_fs, Rm -> MoveToSystemRegister("cpsr_fs", Rm)
        std::string sys_reg = operands[0];
        for (char& c : sys_reg) { c = tolower(c); }
        IROperand rm = parse_operand(operands[1]);
        return std::make_unique<IRInstruction>(IROpcode::MoveToSystemRegister, std::vector<IROperand>{sys_reg, rm});
    }
}

IROperand ArmToIrConverter::parse_operand(const std::string& operand_str) {
    if (operand_str.empty()) {
        throw std::runtime_error("Empty operand string provided to parse_operand.");
    }

    // Immediate values (e.g., #10, #0xA)
    if (operand_str[0] == '#') {
        std::string immediate_str = operand_str.substr(1);
        if (immediate_str.rfind("0x", 0) == 0 || immediate_str.rfind("0X", 0) == 0) {
            return static_cast<uint64_t>(std::stoull(immediate_str, nullptr, 16));
        } else {
            return static_cast<uint64_t>(std::stoull(immediate_str, nullptr, 10));
        }
    }

    // Memory operands (e.g., [r0], [r1, #4])
    if (operand_str[0] == '[' && operand_str.back() == ']') {
        return parse_memory_operand(operand_str);
    }

    // Label (for branch or load from literal pool)
    if (symbol_table_.count(operand_str)) {
        return symbol_table_.at(operand_str);
    }

    // Otherwise, assume it's a register
    return parse_register(operand_str);
}

IRRegister ArmToIrConverter::parse_register(const std::string& reg_str) {
    // This is a simplified parser. A real implementation would do a reverse lookup
    // on the architecture's register map to find the abstract IRRegister.
    std::string lower_str = reg_str;
    for (char& c : lower_str) { c = tolower(c); }

    if (lower_str == "sp") return {IRRegisterType::GPR, 13, 32};
    if (lower_str == "lr") return {IRRegisterType::GPR, 14, 32};
    if (lower_str == "pc") return {IRRegisterType::IP, 0, 32};

    if (lower_str[0] == 'r' && isdigit(lower_str[1])) {
        uint32_t index = std::stoul(lower_str.substr(1));
        if (index <= 12) {
            return {IRRegisterType::GPR, index, 32};
        }
    }
    throw std::runtime_error("Unknown or invalid ARM register: " + reg_str);
}

IRMemoryOperand ArmToIrConverter::parse_memory_operand(const std::string& mem_str) {
    std::string inner = trim(mem_str.substr(1, mem_str.length() - 2));

    IRMemoryOperand mem_op;
    mem_op.size = 32; // Assume 32-bit (word) access for LDR/STR

    // Find comma for offset, if it exists
    size_t comma_pos = inner.find(',');
    if (comma_pos == std::string::npos) {
        // No comma, must be register direct: [rN]
        mem_op.base_reg = parse_register(inner);
    } else {
        // Comma found, must be register with offset: [rN, #imm]
        std::string base_reg_str = trim(inner.substr(0, comma_pos));
        std::string offset_str = trim(inner.substr(comma_pos + 1));

        mem_op.base_reg = parse_register(base_reg_str);

        if (offset_str[0] == '#') {
            mem_op.displacement = std::stoll(offset_str.substr(1));
        } else {
            // TODO: Handle register offset, e.g., [r0, r1]
            throw std::runtime_error("Register offset in memory operand not yet supported: " + mem_str);
        }
    }

    return mem_op;
}

std::unique_ptr<IRInstruction> ArmToIrConverter::translate_data_processing(const std::string& mnemonic, const std::vector<std::string>& operands) {
    IROpcode opcode;
    if (mnemonic == "mov" || mnemonic == "movw") {
        opcode = IROpcode::Move;
    } else if (mnemonic == "add") {
        opcode = IROpcode::Add;
    } else if (mnemonic == "sub") {
        opcode = IROpcode::Sub;
    } else if (mnemonic == "cmp") {
        opcode = IROpcode::Cmp;
    } else if (mnemonic == "orr") {
        opcode = IROpcode::Or;
    } else if (mnemonic == "eor") {
        opcode = IROpcode::Xor;
    } else if (mnemonic == "rsb") {
        opcode = IROpcode::Sub;
    } else if (mnemonic == "adc") {
        opcode = IROpcode::AddC;
    } else if (mnemonic == "sbc") {
        opcode = IROpcode::SubC;
    } else if (mnemonic == "rsc") {
        opcode = IROpcode::SubC;
    } else if (mnemonic == "tst") {
        opcode = IROpcode::Tst;
    } else if (mnemonic == "teq") {
        opcode = IROpcode::Teq;
    } else if (mnemonic == "cmn") {
        opcode = IROpcode::Cmn;
    } else if (mnemonic == "mvn") {
        opcode = IROpcode::MoveNot;
    } else if (mnemonic == "bic") {
        opcode = IROpcode::AndNot;
    } else if (mnemonic == "clz") {
        opcode = IROpcode::CountLeadingZeros;
    } else if (mnemonic == "rbit") {
        opcode = IROpcode::ReverseBits;
    } else if (mnemonic == "rev") {
        opcode = IROpcode::ReverseBytes;
    } else if (mnemonic == "rev16") {
        opcode = IROpcode::ReverseBytes16;
    } else if (mnemonic == "revsh") {
        opcode = IROpcode::ReverseBytesSignedHalfword;
    } else if (mnemonic == "qadd") {
        opcode = IROpcode::SaturatingAdd;
    } else if (mnemonic == "qsub") {
        opcode = IROpcode::SaturatingSub;
    } else if (mnemonic == "qdadd") {
        opcode = IROpcode::SaturatingDoubleAdd;
    } else if (mnemonic == "qdsub") {
        opcode = IROpcode::SaturatingDoubleSub;
    } else if (mnemonic == "mla") {
        opcode = IROpcode::MultiplyAccumulate;
    } else if (mnemonic == "mls") {
        opcode = IROpcode::MultiplySubtract;
    } else if (mnemonic == "umull") {
        opcode = IROpcode::UnsignedMultiplyLong;
    } else if (mnemonic == "smull") {
        opcode = IROpcode::SignedMultiplyLong;
    } else if (mnemonic == "umlal") {
        opcode = IROpcode::UnsignedMultiplyAccumulateLong;
    } else if (mnemonic == "smlal") {
        opcode = IROpcode::SignedMultiplyAccumulateLong;
    } else {
        return nullptr; // Not a data processing op we handle yet
    }

    std::vector<IROperand> ir_operands;
    for (const auto& op_str : operands) {
        ir_operands.push_back(parse_operand(op_str));
    }

    // CMP is a special case - it's like SUB but the destination is implicit.
    // The IR for Cmp, Tst, Teq, Cmn expects two operands, not three.
    if (opcode == IROpcode::Cmp || opcode == IROpcode::Tst || opcode == IROpcode::Teq || opcode == IROpcode::Cmn) {
        return std::make_unique<IRInstruction>(opcode, std::vector<IROperand>{ir_operands[0], ir_operands[1]});
    }

    // MVN is a 2-operand instruction (dest, src)
    if (opcode == IROpcode::MoveNot) {
        return std::make_unique<IRInstruction>(opcode, std::vector<IROperand>{ir_operands[0], ir_operands[1]});
    }

    // RSB/RSC are special cases of SUB/SUBC with swapped operands.
    if (mnemonic == "rsb" || mnemonic == "rsc") {
        // RSB rd, rn, op2  =>  rd = op2 - rn
        // RSC rd, rn, op2  =>  rd = op2 - rn - !CF
        // We map this to Sub(rd, op2, rn) or SubC(rd, op2, rn)
        auto opcode = (mnemonic == "rsb") ? IROpcode::Sub : IROpcode::SubC;
        return std::make_unique<IRInstruction>(opcode, std::vector<IROperand>{ir_operands[0], ir_operands[2], ir_operands[1]});
    }

    // Standard data processing is `DEST, SRC, OP2`
    return std::make_unique<IRInstruction>(opcode, std::move(ir_operands));
}

std::unique_ptr<IRInstruction> ArmToIrConverter::translate_load_store(const std::string& mnemonic, const std::vector<std::string>& operands) {
    IROpcode opcode;
    if (mnemonic == "ldr") {
        opcode = IROpcode::Load;
    } else if (mnemonic == "str") {
        opcode = IROpcode::Store;
    } else {
        return nullptr;
    }

    if (operands.size() < 2) {
        throw std::runtime_error("Invalid number of operands for " + mnemonic);
    }

    IROperand op1 = parse_operand(operands[0]);
    IROperand op2 = parse_operand(operands[1]);

    // LDR Rd, [Rn] -> IR Load(Rd, [Rn])
    // STR Rd, [Rn] -> IR Store([Rn], Rd)
    // The IR Store instruction expects the destination (memory) as the first operand.
    if (opcode == IROpcode::Store) {
        return std::make_unique<IRInstruction>(opcode, std::vector<IROperand>{op2, op1});
    } else {
        return std::make_unique<IRInstruction>(opcode, std::vector<IROperand>{op1, op2});
    }
}

std::unique_ptr<IRInstruction> ArmToIrConverter::translate_branch(const std::string& mnemonic, const std::vector<std::string>& operands) {
    if (operands.empty()) {
        throw std::runtime_error("Missing operand for branch instruction: " + mnemonic);
    }

    // The target is always the first operand for a branch.
    IROperand target = parse_operand(operands[0]);

    // Unconditional Branch: B <label>
    if (mnemonic == "b") {
        return std::make_unique<IRInstruction>(IROpcode::Jump, std::vector<IROperand>{target});
    }

    // Branch with Link: BL <label>
    if (mnemonic == "bl") {
        return std::make_unique<IRInstruction>(IROpcode::Call, std::vector<IROperand>{target});
    }

    // Branch and Exchange: BX <register>
    if (mnemonic == "bx") {
        // Note: Thumb mode exchange is not supported.
        return std::make_unique<IRInstruction>(IROpcode::Jump, std::vector<IROperand>{target});
    }

    // Branch with Link and Exchange: BLX <register>
    if (mnemonic == "blx") {
        // Note: Thumb mode exchange is not supported.
        return std::make_unique<IRInstruction>(IROpcode::Call, std::vector<IROperand>{target});
    }

    // --- Conditional Branches ---
    // Extract the condition code suffix from the mnemonic (e.g., "eq" from "beq").
    std::string cond_str = mnemonic.substr(1);

    // Map ARM condition suffixes to our generic IR Condition Codes
    static const std::map<std::string, IRConditionCode> cond_map = {
        {"eq", IRConditionCode::Equal},         {"ne", IRConditionCode::NotEqual},
        {"gt", IRConditionCode::Greater},        {"ge", IRConditionCode::GreaterOrEqual},
        {"lt", IRConditionCode::Less},           {"le", IRConditionCode::LessOrEqual},
        {"hi", IRConditionCode::Above},          {"ls", IRConditionCode::BelowOrEqual},
        {"hs", IRConditionCode::AboveOrEqual},   {"cs", IRConditionCode::AboveOrEqual}, // Synonyms
        {"lo", IRConditionCode::Below},          {"cc", IRConditionCode::Below}, // Synonyms
        {"mi", IRConditionCode::Sign},           {"pl", IRConditionCode::NotSign},
        {"vs", IRConditionCode::Overflow},       {"vc", IRConditionCode::NotOverflow}
    };

    auto it = cond_map.find(cond_str);
    if (it != cond_map.end()) {
        // Create a Branch instruction with the target address and the condition code.
        return std::make_unique<IRInstruction>(IROpcode::Branch, std::vector<IROperand>{target, it->second});
    }

    throw std::runtime_error("Unsupported branch instruction: " + mnemonic);
}

std::unique_ptr<IRInstruction> ArmToIrConverter::translate_push_pop(const std::string& mnemonic, const std::vector<std::string>& operands) {
    IROpcode opcode = (mnemonic == "push") ? IROpcode::Push : IROpcode::Pop;

    // This is a simplified handler for single-register push/pop, e.g., PUSH r0
    // The standard ARM syntax is PUSH {r0, r1, ...}
    if (operands.size() != 1) {
        throw std::runtime_error("Unsupported multi-register PUSH/POP syntax. Only single register is supported for now.");
    }

    // The operand is the register to push or pop.
    IROperand reg_op = parse_operand(operands[0]);

    return std::make_unique<IRInstruction>(opcode, std::vector<IROperand>{reg_op});
}