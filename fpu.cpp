#include "fpu.h"
#include "i_simulator.h"
#include "i_database_manager.h"
#include "architecture.h"
#include <cmath>
#include <stdexcept>
#include "i_register_map.h"
#include "execution_helpers.h"

namespace {

// Helper to get a float value from any operand type
float get_float_operand(const IROperand& op, ISimulator& simulator) {
    if (const std::string* reg_name = std::get_if<std::string>(&op)) {
        return simulator.getRegisterMap().get_float(*reg_name);
    } else if (const uint64_t* imm = std::get_if<uint64_t>(&op)) {
        return static_cast<float>(*imm);
    }
    // Note: Add memory operand support if needed
    throw std::runtime_error("Unsupported operand type for get_float_operand");
}

// Helper to get a double value from any operand type
double get_double_operand(const IROperand& op, ISimulator& simulator) {
    if (const std::string* reg_name = std::get_if<std::string>(&op)) {
        return simulator.getRegisterMap().get_double(*reg_name);
    } else if (const uint64_t* imm = std::get_if<uint64_t>(&op)) {
        return static_cast<double>(*imm);
    }
    // Note: Add memory operand support if needed
    throw std::runtime_error("Unsupported operand type for get_double_operand");
}

// Helper to set a float value to a register
void set_float_register(const std::string& reg_name, float value, ISimulator& simulator) {
    simulator.getRegisterMap().set_float(reg_name, value);
}

// Helper to set a double value to a register
void set_double_register(const std::string& reg_name, double value, ISimulator& simulator) {
    simulator.getRegisterMap().set_double(reg_name, value);
}

// Template for binary floating-point operations (single precision)
template<typename Func>
void handle_binary_float_s_op(const IRInstruction& ir_instr, ISimulator& simulator, Func op_func) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src1_op = ir_instr.operands[1];
    const auto& src2_op = ir_instr.operands[2];

    float src1 = get_float_operand(src1_op, simulator);
    float src2 = get_float_operand(src2_op, simulator);
    float result = op_func(src1, src2);

    set_float_register(std::get<std::string>(dest_op), result, simulator);
}

// Template for unary floating-point operations (single precision)
template<typename Func>
void handle_unary_float_s_op(const IRInstruction& ir_instr, ISimulator& simulator, Func op_func) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];

    float src = get_float_operand(src_op, simulator);
    float result = op_func(src);

    set_float_register(std::get<std::string>(dest_op), result, simulator);
}

// Template for binary floating-point operations (double precision)
template<typename Func>
void handle_binary_float_d_op(const IRInstruction& ir_instr, ISimulator& simulator, Func op_func) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src1_op = ir_instr.operands[1];
    const auto& src2_op = ir_instr.operands[2];

    double src1 = get_double_operand(src1_op, simulator);
    double src2 = get_double_operand(src2_op, simulator);
    double result = op_func(src1, src2);

    set_double_register(std::get<std::string>(dest_op), result, simulator);
}

// Template for unary floating-point operations (double precision)
template<typename Func>
void handle_unary_float_d_op(const IRInstruction& ir_instr, ISimulator& simulator, Func op_func) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];

    double src = get_double_operand(src_op, simulator);
    double result = op_func(src);

    set_double_register(std::get<std::string>(dest_op), result, simulator);
}

template<typename Func>
void handle_d_to_s_op(const IRInstruction& ir_instr, ISimulator& simulator, Func op_func) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];

    double src = get_double_operand(src_op, simulator);
    float result = op_func(src);

    set_float_register(std::get<std::string>(dest_op), result, simulator);
}

template<typename Func>
void handle_binary_float_s_op_no_result(const IRInstruction& ir_instr, ISimulator& simulator, Func op_func) {
    const auto& src1_op = ir_instr.operands[0];
    const auto& src2_op = ir_instr.operands[1];

    float src1 = get_float_operand(src1_op, simulator);
    float src2 = get_float_operand(src2_op, simulator);
    op_func(src1, src2);
}

template<typename Func>
void handle_binary_float_d_op_no_result(const IRInstruction& ir_instr, ISimulator& simulator, Func op_func) {
    const auto& src1_op = ir_instr.operands[0];
    const auto& src2_op = ir_instr.operands[1];

    double src1 = get_double_operand(src1_op, simulator);
    double src2 = get_double_operand(src2_op, simulator);
    op_func(src1, src2);
}

template<typename Func>
void handle_s_to_d_op(const IRInstruction& ir_instr, ISimulator& simulator, Func op_func) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];

    float src = get_float_operand(src_op, simulator);
    double result = op_func(src);

    set_double_register(std::get<std::string>(dest_op), result, simulator);
}

template<typename Func>
void handle_int_to_s_op(const IRInstruction& ir_instr, ISimulator& simulator, Func op_func) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];

    int64_t src = getOperandValue(src_op, simulator);
    float result = op_func(src);

    set_float_register(std::get<std::string>(dest_op), result, simulator);
}

template<typename Func>
void handle_int_to_d_op(const IRInstruction& ir_instr, ISimulator& simulator, Func op_func) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];

    int64_t src = getOperandValue(src_op, simulator);
    double result = op_func(src);

    set_double_register(std::get<std::string>(dest_op), result, simulator);
}

template<typename Func>
void handle_s_to_int_op(const IRInstruction& ir_instr, ISimulator& simulator, Func op_func) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];

    float src = get_float_operand(src_op, simulator);
    int64_t result = op_func(src);

    setRegisterValue(std::get<std::string>(dest_op), result, simulator);
}

template<typename Func>
void handle_d_to_int_op(const IRInstruction& ir_instr, ISimulator& simulator, Func op_func) {
    const auto& dest_op = ir_instr.operands[0];
    const auto& src_op = ir_instr.operands[1];

    double src = get_double_operand(src_op, simulator);
    int64_t result = op_func(src);

    setRegisterValue(std::get<std::string>(dest_op), result, simulator);
}

} // anonymous namespace

void FPU::execute(PipelinedInstruction& instruction, ISimulator& simulator) {
    switch (instruction.ir_instruction.opcode) {
        case IROpcode::FloatAddS:         handle_ir_float_add_s(instruction.ir_instruction, simulator); break;
        case IROpcode::FloatSubS:         handle_ir_float_sub_s(instruction.ir_instruction, simulator); break;
        case IROpcode::FloatMulS:         handle_ir_float_mul_s(instruction.ir_instruction, simulator); break;
        case IROpcode::FloatDivS:         handle_ir_float_div_s(instruction.ir_instruction, simulator); break;
        case IROpcode::FloatSqrtS:        handle_ir_float_sqrt_s(instruction.ir_instruction, simulator); break;
        case IROpcode::FloatAddD:         handle_ir_float_add_d(instruction.ir_instruction, simulator); break;
        case IROpcode::FloatSubD:         handle_ir_float_sub_d(instruction.ir_instruction, simulator); break;
        case IROpcode::FloatMulD:         handle_ir_float_mul_d(instruction.ir_instruction, simulator); break;
        case IROpcode::FloatDivD:         handle_ir_float_div_d(instruction.ir_instruction, simulator); break;
        case IROpcode::FloatSqrtD:        handle_ir_float_sqrt_d(instruction.ir_instruction, simulator); break;
        case IROpcode::FloatCmpS:         handle_ir_float_cmp_s(instruction.ir_instruction, simulator); break;
        case IROpcode::FloatCmpD:         handle_ir_float_cmp_d(instruction.ir_instruction, simulator); break;
        case IROpcode::FloatToS:          handle_ir_float_to_s(instruction.ir_instruction, simulator); break;
        case IROpcode::FloatToD:          handle_ir_float_to_d(instruction.ir_instruction, simulator); break;
        case IROpcode::IntToFloatS:       handle_ir_int_to_float_s(instruction.ir_instruction, simulator); break;
        case IROpcode::IntToFloatD:       handle_ir_int_to_float_d(instruction.ir_instruction, simulator); break;
        case IROpcode::FloatToIntS:       handle_ir_float_to_int_s(instruction.ir_instruction, simulator); break;
        case IROpcode::FloatToIntD:       handle_ir_float_to_int_d(instruction.ir_instruction, simulator); break;
        default:
            break;
    }
}

bool FPU::is_busy() const { return false; }
int FPU::latency() const { return 3; } // Example latency

void FPU::handle_ir_float_add_s(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_float_s_op(ir_instr, simulator, [](float a, float b) { return a + b; });
}

void FPU::handle_ir_float_sub_s(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_float_s_op(ir_instr, simulator, [](float a, float b) { return a - b; });
}

void FPU::handle_ir_float_mul_s(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_float_s_op(ir_instr, simulator, [](float a, float b) { return a * b; });
}

void FPU::handle_ir_float_div_s(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_float_s_op(ir_instr, simulator, [](float a, float b) { return a / b; });
}

void FPU::handle_ir_float_sqrt_s(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_unary_float_s_op(ir_instr, simulator, [](float a) { return std::sqrt(a); });
}

void FPU::handle_ir_float_add_d(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_float_d_op(ir_instr, simulator, [](double a, double b) { return a + b; });
}

void FPU::handle_ir_float_sub_d(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_float_d_op(ir_instr, simulator, [](double a, double b) { return a - b; });
}

void FPU::handle_ir_float_mul_d(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_float_d_op(ir_instr, simulator, [](double a, double b) { return a * b; });
}

void FPU::handle_ir_float_div_d(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_float_d_op(ir_instr, simulator, [](double a, double b) { return a / b; });
}

void FPU::handle_ir_float_sqrt_d(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_unary_float_d_op(ir_instr, simulator, [](double a) { return std::sqrt(a); });
}

void FPU::handle_ir_float_cmp_s(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_float_s_op_no_result(ir_instr, simulator, [&](float src1, float src2) {
        if (src1 == src2) {
            simulator.set_ZF(true);
            simulator.set_CF(false);
        } else if (src1 < src2) {
            simulator.set_ZF(false);
            simulator.set_CF(true);
        } else {
            simulator.set_ZF(false);
            simulator.set_CF(false);
        }
    });
}

void FPU::handle_ir_float_cmp_d(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_binary_float_d_op_no_result(ir_instr, simulator, [&](double src1, double src2) {
        if (src1 == src2) {
            simulator.set_ZF(true);
            simulator.set_CF(false);
        } else if (src1 < src2) {
            simulator.set_ZF(false);
            simulator.set_CF(true);
        } else {
            simulator.set_ZF(false);
            simulator.set_CF(false);
        }
    });
}

void FPU::handle_ir_float_to_s(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_d_to_s_op(ir_instr, simulator, [](double a) { return static_cast<float>(a); });
}

void FPU::handle_ir_float_to_d(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_s_to_d_op(ir_instr, simulator, [](float a) { return static_cast<double>(a); });
}

void FPU::handle_ir_int_to_float_s(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_int_to_s_op(ir_instr, simulator, [](int64_t a) { return static_cast<float>(a); });
}

void FPU::handle_ir_int_to_float_d(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_int_to_d_op(ir_instr, simulator, [](int64_t a) { return static_cast<double>(a); });
}

void FPU::handle_ir_float_to_int_s(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_s_to_int_op(ir_instr, simulator, [](float a) { return static_cast<int64_t>(a); });
}

void FPU::handle_ir_float_to_int_d(const IRInstruction& ir_instr, ISimulator& simulator) {
    handle_d_to_int_op(ir_instr, simulator, [](double a) { return static_cast<int64_t>(a); });
}