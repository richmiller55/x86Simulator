#include "x86_to_ir.h"
#include "architecture.h" // TODO: This is not ideal, see translate_operand
#include <stdexcept>

// Forward declaration for our new operand translation helper
IROperand translate_operand(const DecodedOperand& decoded_op, const Architecture& arch, uint32_t size_hint = 32);

// TODO: This is a temporary, inefficient way to do reverse lookups.
// The Architecture class should be improved with a dedicated reverse map.
IRRegister find_ir_register_by_name(const std::string& name, const Architecture& arch) {
    for (const auto& pair : arch.register_map) {
        if (pair.second == name) {
            return IRRegister{pair.first.type, pair.first.index, pair.first.size};
        }
    }
    throw std::runtime_error("Unknown register name in translate_operand: " + name);
}


IROperand translate_operand(const DecodedOperand& decoded_op, const Architecture& arch, uint32_t size_hint) {
    switch (decoded_op.type) {
        case OperandType::REGISTER:
        case OperandType::YMM_REGISTER: // Treat YMM like any other register for now
        {
            return find_ir_register_by_name(decoded_op.text, arch);
        }
        case OperandType::IMMEDIATE:
        {
            return static_cast<uint64_t>(decoded_op.value);
        }
        case OperandType::MEMORY:
        {
            // This is a simplified translation. A full implementation would need to parse
            // the complex memory addressing from decoded_op.text (e.g., "[eax + ecx*4]")
            IRMemoryOperand mem_op;
            mem_op.displacement = decoded_op.value;
            mem_op.size = size_hint; // Use the hint for memory access size
            return mem_op;
        }
        default:
            throw std::runtime_error("Unsupported operand type in translate_operand");
    }
}

std::unique_ptr<IRInstruction> translate_to_ir(const DecodedInstruction& decoded_instr) {
    // For this to work, we need an Architecture object. For now, we create one on the fly.
    // In a real scenario, this would be passed in or be globally available.
    static Architecture x86_arch = create_x86_architecture();

    std::vector<IROperand> ops;
    IROpcode opcode = IROpcode::Nop;
    bool supported = true;

    if (decoded_instr.mnemonic == "mov") {
        opcode = IROpcode::Move;
        // Get the size from the destination operand
        auto dest_reg = find_ir_register_by_name(decoded_instr.operands[0].text, x86_arch);
        ops.push_back(dest_reg);
        ops.push_back(translate_operand(decoded_instr.operands[1], x86_arch, dest_reg.size));

    } else if (decoded_instr.mnemonic == "add") {
        opcode = IROpcode::Add;
        auto dest_reg = find_ir_register_by_name(decoded_instr.operands[0].text, x86_arch);
        ops.push_back(dest_reg);
        ops.push_back(translate_operand(decoded_instr.operands[1], x86_arch, dest_reg.size));

    } else if (decoded_instr.mnemonic == "sub") {
        opcode = IROpcode::Sub;
        auto dest_reg = find_ir_register_by_name(decoded_instr.operands[0].text, x86_arch);
        ops.push_back(dest_reg);
        ops.push_back(translate_operand(decoded_instr.operands[1], x86_arch, dest_reg.size));

    } else if (decoded_instr.mnemonic == "cmp") {
        opcode = IROpcode::Cmp;
        auto op1_reg = find_ir_register_by_name(decoded_instr.operands[0].text, x86_arch);
        ops.push_back(op1_reg);
        ops.push_back(translate_operand(decoded_instr.operands[1], x86_arch, op1_reg.size));

    } else if (decoded_instr.mnemonic == "jmp") {
        opcode = IROpcode::Jump;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value)); // Jump target address

    } else if (decoded_instr.mnemonic == "jne") {
        opcode = IROpcode::Branch;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value)); // Target address
        ops.push_back(IRConditionCode::NotEqual); // The condition

    } else if (decoded_instr.mnemonic == "jg") {
        opcode = IROpcode::Branch;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value)); // Target address
        ops.push_back(IRConditionCode::Greater); // The condition

    } else if (decoded_instr.mnemonic == "jge") {
        opcode = IROpcode::Branch;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value)); // Target address
        ops.push_back(IRConditionCode::GreaterOrEqual); // The condition

    } else if (decoded_instr.mnemonic == "call") {
        opcode = IROpcode::Call;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value)); // Target address

    } else if (decoded_instr.mnemonic == "xor") {
        opcode = IROpcode::Xor;
        auto dest_reg = find_ir_register_by_name(decoded_instr.operands[0].text, x86_arch);
        ops.push_back(dest_reg);
        ops.push_back(translate_operand(decoded_instr.operands[1], x86_arch, dest_reg.size));

    } else if (decoded_instr.mnemonic == "int") {
        opcode = IROpcode::Syscall;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value)); // Interrupt vector

    } else {
        supported = false;
    }

    if (!supported) {
        return nullptr; // Instruction not supported for translation
    }

    auto ir_instr = std::make_unique<IRInstruction>(opcode, std::move(ops));
    ir_instr->original_address = decoded_instr.address;
    ir_instr->original_size = decoded_instr.length_in_bytes;

    return ir_instr;
}
