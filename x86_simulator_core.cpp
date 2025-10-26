
#include "x86_simulator.h"
#include "ui_manager.h"
#include "decoder.h"
#include "i_database_manager.h"
#include "architecture.h"
#include "ir_visitor.h"

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

bool X86Simulator::is_headless() const {
    return headless_;
}

