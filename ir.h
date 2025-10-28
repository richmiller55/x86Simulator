#ifndef IR_H
#define IR_H

#include <cstdint>
#include <vector>
#include <string>
#include <variant>
#include <optional>
#include <memory>

enum class FunctionalUnitType {
    ALU, // Integer Arithmetic Logic Unit
    FPU, // Floating-Point Unit
    VPU  // Vector Processing Unit
};

enum class IROpcode {
    Move,   
    Load,   
    Store,  
    Swap,   
    Add,    
    Sub,    
    AddC,
    SubC,
    Mul,    
    IMul,   
    Div,    
    SaturatingAdd,
    SaturatingSub,
    SaturatingDoubleAdd,
    SaturatingDoubleSub,
    MultiplyAccumulate,
    MultiplySubtract,
    UnsignedMultiplyLong,
    SignedMultiplyLong,
    UnsignedMultiplyAccumulateLong,
    SignedMultiplyAccumulateLong,
    And,    
    Or,     
    Xor,    
    Not,    
    MoveNot,
    AndNot,
    Shl,    
    Shr,    
    Sar,    
    Jump,   
    Branch, 
    CompareAndBranchIfNotZero, 
    Call,
    Push,
    Pop,
    Dec,
    Inc,   
    Ret,
    PackedAddPS,
    PackedSubPS,
    PackedMulPS,
    PackedDivPS,
    PackedMaxPS,
    PackedMinPS,
    PackedSqrtPS,
    PackedReciprocalPS, 
    PackedReciprocalSqrtPS, 
    PackedAnd,
    PackedAndNot, 
    PackedOr,
    PackedXor,
    PackedAddI8,
    PackedAddI16,
    PackedAddI32,
    PackedAddI64,
    PackedSubI8,
    PackedSubI16,
    PackedSubI32,
    PackedSubI64,
    PackedMulLowI16, 
    PackedMulLowI32, 
    PackedMulU32,    
    VectorMove,
    VectorZero, 
    VectorZeroUpper, 
    In,
    Out,
    Syscall,
    Nop,
    MoveToSystemRegister,
    MoveFromSystemRegister,
    CountLeadingZeros,
    ReverseBits,
    ReverseBytes,
    ReverseBytes16,
    ReverseBytesSignedHalfword,
    Breakpoint,
    WaitForInterrupt,
    WaitForEvent,
    SendEvent,
    Cmp,    
    Tst,
    Teq,
    Cmn,
    FloatAddS, 
    FloatSubS, 
    FloatMulS, 
    FloatDivS, 
    FloatSqrtS, 
    FloatAddD, 
    FloatSubD, 
    FloatMulD, 
    FloatDivD, 
    FloatSqrtD, 
    FloatCmpS, 
    FloatCmpD, 
    FloatToS, 
    FloatToD, 
    IntToFloatS, 
    IntToFloatD, 
    FloatToIntS, 
    FloatToIntD,
    Bubble 
};

enum class IRConditionCode {
    Equal,          
    NotEqual,       
    Below,          
    BelowOrEqual,   
    Above,          
    AboveOrEqual,   
    Less,           
    GreaterOrEqual, 
    LessOrEqual,    
    Greater,        
    Overflow,       
    NotOverflow,    
    Sign,           
    NotSign,        
    ParityEven,     
    ParityOdd,      
};

enum class IRRegisterType {
    GPR,        
    VECTOR,     
    FLAGS,      
    IP,         
    SEGMENT,    
};

struct IRMemoryOperand {
    std::optional<std::string> base_reg;
    std::optional<std::string> index_reg;
    uint32_t scale = 1;
    int64_t displacement = 0;
    uint32_t size = 32;
};

using IROperand = std::variant<
    std::string,    // Register name or Label
    IRMemoryOperand,
    uint64_t,       // Immediate value
    IRConditionCode // For branch conditions
>;

class IRInstruction {
public:
    IRInstruction();
    // TODO: Add original_instruction to the constructor
    IRInstruction(IROpcode op, std::vector<IROperand> ops = {})
        : opcode(op), operands(std::move(ops)) {}

    IRInstruction(IROpcode op, std::vector<IROperand>&& ops, FunctionalUnitType fu_type)
        : opcode(op), operands(std::move(ops)), functional_unit_type(fu_type) {}

    IROpcode opcode;
    std::vector<IROperand> operands;
    FunctionalUnitType functional_unit_type; // New field

    // Metadata for pipeline simulation and debugging
    std::string original_instruction;

    uint64_t original_address = 0;
    uint32_t original_size = 0;
};

using IRProgram = std::vector<std::unique_ptr<IRInstruction>>;

#endif // IR_H