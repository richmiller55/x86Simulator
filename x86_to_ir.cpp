#include "x86_to_ir.h"
#include "architecture.h"
#include <stdexcept>

// Forward declaration for our new operand translation helper
IROperand translate_operand(const DecodedOperand& decoded_op, const Architecture& arch);

IROperand translate_operand(const DecodedOperand& decoded_op, const Architecture& arch) {
    switch (decoded_op.type) {
        case OperandType::REGISTER:
        case OperandType::YMM_REGISTER: // YMM registers are identified by their string names
        {
            return decoded_op.text;
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
            // The size of memory operands is often implicit from the instruction/other operands.
            // The IR executor will have to handle this. For now, we leave it default.
            return mem_op;
        }
        default:
            throw std::runtime_error("Unsupported operand type in translate_operand");
    }
}

std::unique_ptr<IRInstruction> translate_to_ir(const DecodedInstruction& decoded_instr) {
    static Architecture x86_arch = create_x86_architecture();

    std::vector<IROperand> ops;
    IROpcode opcode = IROpcode::Nop;
    bool supported = true;

    // Helper lambda to translate all operands
    auto translate_all_operands = [&]() {
        for (const auto& op : decoded_instr.operands) {
            ops.push_back(translate_operand(op, x86_arch));
        }
    };

    const auto& mnemonic = decoded_instr.mnemonic;

    if (mnemonic == "mov" || mnemonic == "movsx" || mnemonic == "movzx") {
        opcode = IROpcode::Move;
        translate_all_operands();
    } else if (mnemonic == "vmovups") {
        opcode = IROpcode::VectorMove;
        translate_all_operands();
    } else if (mnemonic == "add") {
        opcode = IROpcode::Add;
        translate_all_operands();
    } else if (mnemonic == "vaddps") {
        opcode = IROpcode::PackedAddPS;
        translate_all_operands();
    } else if (mnemonic == "sub") {
        opcode = IROpcode::Sub;
        translate_all_operands();
    } else if (mnemonic == "cmp") {
        opcode = IROpcode::Cmp;
        translate_all_operands();
    } else if (mnemonic == "jmp") {
        opcode = IROpcode::Jump;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value));
    } else if (mnemonic == "jne" || mnemonic == "jnz") {
        opcode = IROpcode::Branch;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value));
        ops.push_back(IRConditionCode::NotEqual);
    } else if (mnemonic == "jg" || mnemonic == "jnle") {
        opcode = IROpcode::Branch;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value));
        ops.push_back(IRConditionCode::Greater);
    } else if (mnemonic == "jge" || mnemonic == "jnl") {
        opcode = IROpcode::Branch;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value));
        ops.push_back(IRConditionCode::GreaterOrEqual);
    } else if (mnemonic == "je" || mnemonic == "jz") {
        opcode = IROpcode::Branch;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value));
        ops.push_back(IRConditionCode::Equal);
    } else if (mnemonic == "jl" || mnemonic == "jnge") {
        opcode = IROpcode::Branch;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value));
        ops.push_back(IRConditionCode::Less);
    } else if (mnemonic == "jle" || mnemonic == "jng") {
        opcode = IROpcode::Branch;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value));
        ops.push_back(IRConditionCode::LessOrEqual);
    } else if (mnemonic == "ja" || mnemonic == "jnbe") {
        opcode = IROpcode::Branch;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value));
        ops.push_back(IRConditionCode::Above);
    } else if (mnemonic == "jae" || mnemonic == "jnb" || mnemonic == "jnc") {
        opcode = IROpcode::Branch;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value));
        ops.push_back(IRConditionCode::AboveOrEqual);
    } else if (mnemonic == "jb" || mnemonic == "jnae" || mnemonic == "jc") {
        opcode = IROpcode::Branch;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value));
        ops.push_back(IRConditionCode::Below);
    } else if (mnemonic == "jbe" || mnemonic == "jna") {
        opcode = IROpcode::Branch;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value));
        ops.push_back(IRConditionCode::BelowOrEqual);
    } else if (mnemonic == "jo") {
        opcode = IROpcode::Branch;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value));
        ops.push_back(IRConditionCode::Overflow);
    } else if (mnemonic == "jno") {
        opcode = IROpcode::Branch;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value));
        ops.push_back(IRConditionCode::NotOverflow);
    } else if (mnemonic == "js") {
        opcode = IROpcode::Branch;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value));
        ops.push_back(IRConditionCode::Sign);
    } else if (mnemonic == "jns") {
        opcode = IROpcode::Branch;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value));
        ops.push_back(IRConditionCode::NotSign);
    } else if (mnemonic == "jp" || mnemonic == "jpe") {
        opcode = IROpcode::Branch;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value));
        ops.push_back(IRConditionCode::ParityEven);
    } else if (mnemonic == "jnp" || mnemonic == "jpo") {
        opcode = IROpcode::Branch;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value));
        ops.push_back(IRConditionCode::ParityOdd);
    } else if (mnemonic == "call") {
        opcode = IROpcode::Call;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value));
    } else if (mnemonic == "push") {
        opcode = IROpcode::Push;
        translate_all_operands();
    } else if (mnemonic == "pop") {
        opcode = IROpcode::Pop;
        translate_all_operands();
    } else if (mnemonic == "dec") {
        opcode = IROpcode::Dec;
        translate_all_operands();
    } else if (mnemonic == "inc") {
        opcode = IROpcode::Inc;
        translate_all_operands();
    } else if (mnemonic == "vaddps") {
        opcode = IROpcode::PackedAddPS;
        translate_all_operands();
    } else if (mnemonic == "vsubps") {
        opcode = IROpcode::PackedSubPS;
        translate_all_operands();
    } else if (mnemonic == "xor" || mnemonic == "vpxor") {
        opcode = (mnemonic == "xor") ? IROpcode::Xor : IROpcode::PackedXor;
        translate_all_operands();
    } else if (mnemonic == "int") {
        opcode = IROpcode::Syscall;
        ops.push_back(static_cast<uint64_t>(decoded_instr.operands[0].value));
    } else if (mnemonic == "ret") {
        opcode = IROpcode::Ret;
    } else if (mnemonic == "in") {
        opcode = IROpcode::In;
        translate_all_operands();
    } else if (mnemonic == "out") {
        opcode = IROpcode::Out;
        translate_all_operands();
    } else if (mnemonic == "div" || mnemonic == "idiv") {
        opcode = (mnemonic == "div") ? IROpcode::Div : IROpcode::IMul;
        translate_all_operands();
    } else if (mnemonic == "mul" || mnemonic == "imul") {
        opcode = (mnemonic == "mul") ? IROpcode::Mul : IROpcode::IMul;
        translate_all_operands();
    } else if (mnemonic == "vmulps") {
        opcode = IROpcode::PackedMulPS;
        translate_all_operands();
    } else if (mnemonic == "vdivps") {
        opcode = IROpcode::PackedDivPS;
        translate_all_operands();
    } else if (mnemonic == "vmaxps") {
        opcode = IROpcode::PackedMaxPS;
        translate_all_operands();
    } else if (mnemonic == "vminps") {
        opcode = IROpcode::PackedMinPS;
        translate_all_operands();
    } else if (mnemonic == "vsqrtps") {
        opcode = IROpcode::PackedSqrtPS;
        translate_all_operands();
    } else if (mnemonic == "vrcpps") {
        opcode = IROpcode::PackedReciprocalPS;
        translate_all_operands();
    } else if (mnemonic == "vpand") {
        opcode = IROpcode::PackedAnd;
        translate_all_operands();
    } else if (mnemonic == "vpandn") {
        opcode = IROpcode::PackedAndNot;
        translate_all_operands();
    } else if (mnemonic == "vpor") {
        opcode = IROpcode::PackedOr;
        translate_all_operands();
    } else if (mnemonic == "vpmullw") {
        opcode = IROpcode::PackedMulLowI16;
        translate_all_operands();
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