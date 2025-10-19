#include "arm_simulator.h"
#include "ir_visitor.h"
#include "program_decoder.h"
#include "ir_executor_helpers.h"
#include <iostream>
#include <fstream>
#include <sstream>

// Helper function to convert IROpcode to string for logging
std::string ir_op_to_string(IROpcode opcode) {
    switch (opcode) {
        case IROpcode::Move: return "Move";
        case IROpcode::Load: return "Load";
        case IROpcode::Store: return "Store";
        case IROpcode::Add: return "Add";
        case IROpcode::Sub: return "Sub";
        case IROpcode::Mul: return "Mul";
        case IROpcode::IMul: return "IMul";
        case IROpcode::Div: return "Div";
        case IROpcode::And: return "And";
        case IROpcode::Or: return "Or";
        case IROpcode::Xor: return "Xor";
        case IROpcode::Not: return "Not";
        case IROpcode::Shl: return "Shl";
        case IROpcode::Shr: return "Shr";
        case IROpcode::Sar: return "Sar";
        case IROpcode::Jump: return "Jump";
        case IROpcode::Branch: return "Branch";
        case IROpcode::Call: return "Call";
        case IROpcode::Ret: return "Ret";
        case IROpcode::Push: return "Push";
        case IROpcode::Pop: return "Pop";
        case IROpcode::Inc: return "Inc";
        case IROpcode::Dec: return "Dec";
        case IROpcode::Cmp: return "Cmp";
        case IROpcode::Syscall: return "Syscall";
        default: return "Unknown";
    }
}

// A visitor for executing ARM IR instructions by dispatching to generic handlers.
class ArmIRVisitor : public IRVisitor {
public:
    void visit(const IRInstruction& instr, ISimulator& simulator) override {
        switch (instr.opcode) {
            case IROpcode::Move:    handle_ir_move(instr, simulator); break;
            case IROpcode::Load:    handle_ir_load(instr, simulator); break;
            case IROpcode::Store:   handle_ir_store(instr, simulator); break;
            case IROpcode::Add:     handle_ir_add(instr, simulator); break;
            case IROpcode::Sub:     handle_ir_sub(instr, simulator); break;
            case IROpcode::AddC:    handle_ir_addc(instr, simulator); break;
            case IROpcode::SubC:    handle_ir_subc(instr, simulator); break;
            case IROpcode::Mul:     handle_ir_mul(instr, simulator); break;
            case IROpcode::IMul:    handle_ir_imul(instr, simulator); break;
            case IROpcode::Div:     handle_ir_div(instr, simulator); break;
            case IROpcode::And:     handle_ir_and(instr, simulator); break;
            case IROpcode::Or:      handle_ir_or(instr, simulator); break;
            case IROpcode::Xor:     handle_ir_xor(instr, simulator); break;
            case IROpcode::Not:     handle_ir_not(instr, simulator); break;
            case IROpcode::Shl:     handle_ir_shl(instr, simulator); break;
            case IROpcode::Shr:     handle_ir_shr(instr, simulator); break;
            case IROpcode::Sar:     handle_ir_sar(instr, simulator); break;
            case IROpcode::Jump:    handle_ir_jump(instr, simulator); break;
            case IROpcode::Branch:  handle_ir_branch(instr, simulator); break;
            case IROpcode::Call:    handle_ir_call(instr, simulator); break;
            case IROpcode::Ret:     handle_ir_ret(instr, simulator); break;
            case IROpcode::Push:    handle_ir_push(instr, simulator); break;
            case IROpcode::Pop:     handle_ir_pop(instr, simulator); break;
            case IROpcode::Inc:     handle_ir_inc(instr, simulator); break;
            case IROpcode::Dec:     handle_ir_dec(instr, simulator); break;
            case IROpcode::Cmp:     handle_ir_cmp(instr, simulator); break;
            case IROpcode::Tst:     handle_ir_tst(instr, simulator); break;
            case IROpcode::Teq:     handle_ir_teq(instr, simulator); break;
            case IROpcode::Cmn:     handle_ir_cmn(instr, simulator); break;
            case IROpcode::MoveNot: handle_ir_movenot(instr, simulator); break;
            case IROpcode::AndNot:  handle_ir_andnot(instr, simulator); break;
            case IROpcode::Syscall: handle_ir_syscall(instr, simulator); break;
            default:
                simulator.getDatabaseManager().log(
                    simulator.get_session_id(),
                    "Unsupported IR Opcode in ArmIRVisitor: " + ir_op_to_string(instr.opcode),
                    "WARNING",
                    0,
                    __FILE__,
                    __LINE__
                );
                break;
        }
    }
};

ArmSimulator::ArmSimulator(IDatabaseManager& db_manager, Memory& memory, int session_id, bool headless)
    : db_manager_(db_manager),
      memory_(memory),
      architecture_(create_arm_cortex_r8_architecture()),
      register_map_(architecture_),
      session_id_(session_id),
      headless_(headless) {}

ArmSimulator::~ArmSimulator() {}

// ARM CPSR flag bit positions
constexpr int N_BIT = 31;
constexpr int Z_BIT = 30;
constexpr int C_BIT = 29;
constexpr int V_BIT = 28;

void ArmSimulator::set_ZF(bool value) { 
    if (value) cpsr_ |= (1 << Z_BIT); else cpsr_ &= ~(1 << Z_BIT); 
}
void ArmSimulator::set_SF(bool value) { 
    if (value) cpsr_ |= (1 << N_BIT); else cpsr_ &= ~(1 << N_BIT); 
}
void ArmSimulator::set_CF(bool value) { 
    if (value) cpsr_ |= (1 << C_BIT); else cpsr_ &= ~(1 << C_BIT); 
}
void ArmSimulator::set_OF(bool value) { 
    if (value) cpsr_ |= (1 << V_BIT); else cpsr_ &= ~(1 << V_BIT); 
}

bool ArmSimulator::get_ZF() const { return (cpsr_ >> Z_BIT) & 1; }
bool ArmSimulator::get_SF() const { return (cpsr_ >> N_BIT) & 1; }
bool ArmSimulator::get_CF() const { return (cpsr_ >> C_BIT) & 1; }
bool ArmSimulator::get_OF() const { return (cpsr_ >> V_BIT) & 1; }

void ArmSimulator::set_PF(bool value) {
    // No-op for ARM, as it doesn't have a Parity Flag like x86.
}

bool ArmSimulator::get_PF() const {
    // No-op for ARM, return a default value.
    return false;
}

bool ArmSimulator::loadProgram(const std::string& program_path) {
    std::ifstream file(program_path);
    if (!file) {
        std::cerr << "Error: Cannot open ARM program file: " << program_path << std::endl;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string assembly_content = buffer.str();

    ArmToIrConverter converter(architecture_);
    ir_program_ = converter.convert(assembly_content);

    std::cout << "Successfully converted ARM assembly to IR. " << ir_program_.size() << " IR instructions generated." << std::endl;

    program_decoder_ = std::make_unique<ProgramDecoder>(memory_);

    return true;
}

void ArmSimulator::runProgram() {
    std::cout << "\n--- Starting ARM Simulation ---" << std::endl;
    std::cout << "Executing " << ir_program_.size() << " IR instructions." << std::endl;

    for (const auto& instr : ir_program_) {
        execute_ir_instruction(*instr);
    }

    std::cout << "--- ARM Simulation Finished ---" << std::endl;
}

void ArmSimulator::accept(IRVisitor& visitor, const IRInstruction& instr) {
    visitor.visit(instr, *this);
}

void ArmSimulator::execute_ir_instruction(const IRInstruction& ir_instr) {
    ArmIRVisitor visitor;
    accept(visitor, ir_instr);
}

ProgramDecoder* ArmSimulator::getProgramDecoder() {
    return program_decoder_.get();
}
