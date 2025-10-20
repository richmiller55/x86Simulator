
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
      register_map_(architecture_),
      session_id_(session_id),
      headless_(headless),
      instructionPointer_(0),
      program_size_in_bytes_(0),
      rflags_(0) {
   pipeline_ = std::make_unique<Pipeline>(*this);
   if (!headless_) {
    ui_ = std::make_unique<UIManager>(memory_);
    ui_->setRegisterMap(&register_map_);
  }
  register_map_.set64("rsp", memory_.get_stack_bottom());
  rflags_ |= (1ULL << RFLAGS_ALWAYS_SET_BIT_1);
}

X86Simulator::~X86Simulator() {
}

void X86Simulator::init(const std::string& program_name) {
  session_id_ = db_manager_.createSession(program_name);
}

void X86Simulator::accept(IRVisitor& visitor, const IRInstruction& instr) {
    visitor.visit(instr, *this);
}

uint64_t X86Simulator::get_system_register(const std::string& name) {
    throw std::runtime_error("get_system_register not implemented for x86");
}

void X86Simulator::set_system_register(const std::string& name, uint64_t value) {
    throw std::runtime_error("set_system_register not implemented for x86");
}
