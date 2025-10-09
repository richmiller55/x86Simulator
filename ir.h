#ifndef IR_H
#define IR_H

#include <cstdint>
#include <vector>
#include <string>
#include <variant>
#include <optional>
#include <memory>

/**
 * @brief Defines the canonical, architecture-agnostic operations for the IR.
 *
 * These operations abstract away ISA-specific instructions like 'mov', 'ldr', 'addi'.
 */
enum class IROpcode {
    // === Generic/Scalar Operations ===

    // Data Movement
    Move,   // reg_dest, src (reg or immediate)
    Load,   // reg_dest, mem_src
    Store,  // mem_dest, reg_src

    // Arithmetic
    Add,    // dest, src1, src2 (or dest, src1 for inc)
    Sub,    // dest, src1, src2 (or dest, src1 for dec)
    Mul,    // dest, src1, src2
    IMul,   // dest, src1, src2 (signed multiply)
    Div,    // dest, src1, src2

    // Logical
    And,    // dest, src1, src2
    Or,     // dest, src1, src2
    Xor,    // dest, src1, src2
    Not,    // dest, src
    Shl,    // dest, src, count
    Shr,    // dest, src, count
    Sar,    // dest, src, count

    // Control Flow
    Jump,   // target
    Branch, // target, condition
    Call,   // target
    Ret,

    // === SIMD/Vector Operations ===

    // Packed Arithmetic (PS: Packed Single-Precision Float)
    PackedAddPS,
    PackedSubPS,
    PackedMulPS,
    PackedDivPS,
    PackedMaxPS,
    PackedMinPS,
    PackedSqrtPS,
    PackedReciprocalPS, // For instructions like RCPPS

    // Packed Logical (Integer)
    PackedAnd,
    PackedAndNot, // For instructions like VPANDN
    PackedOr,
    PackedXor,

    // Packed Integer Arithmetic
    PackedMulLowI16, // For VPMULLW

    // Other SIMD
    VectorZero, // For instructions like VZEROUPPER or XORing a register with itself

    // System
    Out,
    Syscall,
    Nop,

    // Comparison
    Cmp,    // src1, src2 (sets flags)
};

/**
 * @brief Defines the condition codes for branch instructions.
 *
 * These abstract away ISA-specific flag combinations.
 */
enum class IRConditionCode {
    // Based on Zero Flag (ZF)
    Equal,          // JE, JZ
    NotEqual,       // JNE, JNZ

    // Based on Carry Flag (CF) - for unsigned comparisons
    Below,          // JB, JNAE
    AboveOrEqual,   // JAE, JNB

    // Based on Sign (SF) and Overflow (OF) - for signed comparisons
    Less,           // JL, JNGE
    GreaterOrEqual, // JGE, JNL
    LessOrEqual,    // JLE, JNG
    Greater,        // JG, JNLE

    // Based on single flags
    Overflow,       // JO
    NotOverflow,    // JNO
    Sign,           // JS
    NotSign,        // JNS
};

/**
 * @brief Defines the type of an abstract register.
 */
enum class IRRegisterType {
    GPR,        // General Purpose Register
    VECTOR,     // SIMD/Vector Register (XMM, YMM, ZMM)
    FLAGS,      // Flags register (e.g., RFLAGS)
    IP,         // Instruction Pointer
    SEGMENT,    // Segment Register
};

/**
 * @brief Represents an abstract register, independent of ISA-specific names like 'eax' or 'r0'.
 */
struct IRRegister {
    IRRegisterType type;
    uint32_t index; // e.g., 0 for GPR0, 1 for GPR1
    uint32_t size;  // Size in bits: 8, 16, 32, 64, 128, 256

    bool operator==(const IRRegister& other) const {
        return type == other.type && index == other.index && size == other.size;
    }
};

/**
 * @brief Represents a flexible memory addressing mode, capable of modeling modes from x86, ARM, etc.
 *
 * Can represent complex modes like: [base + index*scale + displacement]
 */
struct IRMemoryOperand {
    std::optional<IRRegister> base_reg;
    std::optional<IRRegister> index_reg;
    uint32_t scale = 1;
    int64_t displacement = 0;
    uint32_t size = 32; // Size of memory access in bits (e.g., 8, 16, 32, 64)
};

/**
 * @brief A variant type representing any possible operand in an IR instruction.
 */
using IROperand = std::variant<
    IRRegister,
    IRMemoryOperand,
    uint64_t,       // Immediate value
    std::string,    // Label
    IRConditionCode // For branch conditions
>;

/**
 * @brief Represents a single, architecture-agnostic instruction in the Intermediate Representation.
 */
class IRInstruction {
public:
    IRInstruction(IROpcode op, std::vector<IROperand> ops = {})
        : opcode(op), operands(std::move(ops)) {}

    IROpcode opcode;
    std::vector<IROperand> operands;

    // Optional: Metadata about the original instruction
    uint64_t original_address = 0;
    uint32_t original_size = 0;
};

// A program is a sequence of IR instructions.
using IRProgram = std::vector<std::unique_ptr<IRInstruction>>;

#endif // IR_H
