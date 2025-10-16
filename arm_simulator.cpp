#include "arm_simulator.h"
#include "ir_visitor.h"
#include "program_decoder.h"
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
        case IROpcode::Jump: return "Jump";
        case IROpcode::Branch: return "Branch";
        // Add other opcodes as needed
        default: return "Unknown";
    }
}

// A basic visitor for executing ARM IR instructions.
class ArmIRVisitor : public IRVisitor {
public:
    void visit(const IRInstruction& instr, ISimulator& simulator) override {
        simulator.getDatabaseManager().log(
            simulator.get_session_id(), 
            "Executing ARM IR instruction: " + ir_op_to_string(instr.opcode), 
            "INFO", 
            0, 
            __FILE__, 
            __LINE__
        );
    }
};

ArmSimulator::ArmSimulator(IDatabaseManager& db_manager, Memory& memory, int session_id, bool headless)
    : db_manager_(db_manager),
      memory_(memory),
      architecture_(create_arm_cortex_r8_architecture()),
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
