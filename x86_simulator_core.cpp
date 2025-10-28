
#include "x86_simulator.h"
#include "ui_manager.h"
#include "decoder.h"
#include "i_database_manager.h"
#include "architecture.h"
#include "ir_visitor.h"
#include "x86_to_ir.h"

// Constructor with DatabaseManager injection
X86Simulator::X86Simulator(IDatabaseManager& db_manager, Memory& memory, int session_id, bool headless)
    : db_manager_(db_manager),
      memory_(memory),
      architecture_(create_x86_architecture()),
      session_id_(session_id),
      headless_(headless),
      instructionPointer_(0),
      program_size_in_bytes_(0)
{
    std::cout << "X86Simulator constructor: &architecture_ = " << &architecture_ << std::endl;
    alu_ = std::make_unique<ALU>();
    fpu_ = std::make_unique<FPU>();
    vpu_ = std::make_unique<VPU>();
    register_map_ = std::make_unique<RegisterMap>(architecture_);
    pipeline_ = std::make_unique<Pipeline>(*this);
    program_decoder_ = std::make_unique<ProgramDecoder>(memory_);
    if (!headless_) {
        ui_ = std::make_unique<UIManager>(memory_);
        ui_->setRegisterMap(register_map_.get());
    }
    register_map_->set64("rsp", memory_.get_stack_bottom());
    // rflags_ |= (1ULL << RFLAGS_ALWAYS_SET_BIT_1); // This bit is always set.
}

X86Simulator::~X86Simulator() {
}

uint64_t X86Simulator::get_system_register(const std::string& name) {
    // TODO: Implement
    return 0;
}

void X86Simulator::set_system_register(const std::string& name, uint64_t value) {
    // TODO: Implement
}

void X86Simulator::accept(IRVisitor& visitor, const IRInstruction& instr) {
    visitor.visit(instr, *this);
}

const char* X86Simulator::get_stack_pointer_name() const {
    return "rsp";
}

const char* X86Simulator::get_instruction_pointer_name() const {
    return "rip";
}

bool X86Simulator::get_CF() const {
    return (register_map_->get64("rflags") >> RFLAGS_CF_BIT) & 1;
}

void X86Simulator::set_CF(bool value) {
    uint64_t rflags = register_map_->get64("rflags");
    if (value) {
        rflags |= (1ULL << RFLAGS_CF_BIT);
    } else {
        rflags &= ~(1ULL << RFLAGS_CF_BIT);
    }
    register_map_->set64("rflags", rflags);
}

bool X86Simulator::get_ZF() const {
    return (register_map_->get64("rflags") >> RFLAGS_ZF_BIT) & 1;
}

void X86Simulator::set_ZF(bool value) {
    uint64_t rflags = register_map_->get64("rflags");
    if (value) {
        rflags |= (1ULL << RFLAGS_ZF_BIT);
    } else {
        rflags &= ~(1ULL << RFLAGS_ZF_BIT);
    }
    register_map_->set64("rflags", rflags);
}

bool X86Simulator::get_SF() const {
    return (register_map_->get64("rflags") >> RFLAGS_SF_BIT) & 1;
}

void X86Simulator::set_SF(bool value) {
    uint64_t rflags = register_map_->get64("rflags");
    if (value) {
        rflags |= (1ULL << RFLAGS_SF_BIT);
    } else {
        rflags &= ~(1ULL << RFLAGS_SF_BIT);
    }
    register_map_->set64("rflags", rflags);
}

bool X86Simulator::get_OF() const {
    return (register_map_->get64("rflags") >> RFLAGS_OF_BIT) & 1;
}

void X86Simulator::set_OF(bool value) {
    uint64_t rflags = register_map_->get64("rflags");
    if (value) {
        rflags |= (1ULL << RFLAGS_OF_BIT);
    } else {
        rflags &= ~(1ULL << RFLAGS_OF_BIT);
    }
    register_map_->set64("rflags", rflags);
}

bool X86Simulator::get_DF() const {
    return (register_map_->get64("rflags") >> RFLAGS_DF_BIT) & 1;
}

void X86Simulator::set_DF(bool value) {
    uint64_t rflags = register_map_->get64("rflags");
    if (value) {
        rflags |= (1ULL << RFLAGS_DF_BIT);
    } else {
        rflags &= ~(1ULL << RFLAGS_DF_BIT);
    }
    register_map_->set64("rflags", rflags);
}

bool X86Simulator::get_AF() const {
    return (register_map_->get64("rflags") >> RFLAGS_AF_BIT) & 1;
}

void X86Simulator::set_AF(bool value) {
    uint64_t rflags = register_map_->get64("rflags");
    if (value) {
        rflags |= (1ULL << RFLAGS_AF_BIT);
    } else {
        rflags &= ~(1ULL << RFLAGS_AF_BIT);
    }
    register_map_->set64("rflags", rflags);
}

bool X86Simulator::get_PF() const {
    return (register_map_->get64("rflags") >> RFLAGS_PF_BIT) & 1;
}

void X86Simulator::set_PF(bool value) {
    uint64_t rflags = register_map_->get64("rflags");
    if (value) {
        rflags |= (1ULL << RFLAGS_PF_BIT);
    } else {
        rflags &= ~(1ULL << RFLAGS_PF_BIT);
    }
    register_map_->set64("rflags", rflags);
}



bool X86Simulator::is_headless() const {
    return headless_;
}

void X86Simulator::runProgram() {}

bool X86Simulator::executeInstruction(const DecodedInstruction& decoded_instr) {
    auto ir_instr = translate_to_ir(decoded_instr);
    if (ir_instr) {
        execute_ir_instruction(*ir_instr);
        return true;
    }
    return false;
}

void X86Simulator::execute_ir_instruction(const IRInstruction& ir_instr) {
    PipelinedInstruction p_instr(ir_instr, InstructionState::Executing);

    switch (ir_instr.functional_unit_type) {
        case FunctionalUnitType::ALU:
            alu_->execute(p_instr, *this);
            break;
        case FunctionalUnitType::FPU:
            fpu_->execute(p_instr, *this);
            break;
        case FunctionalUnitType::VPU:
            vpu_->execute(p_instr, *this);
            break;
        default:
            db_manager_.log(session_id_, "Unknown functional unit type for IR instruction", "ERROR", register_map_->get32(get_instruction_pointer_name()), __FILE__, __LINE__);
            break;
    }
}


