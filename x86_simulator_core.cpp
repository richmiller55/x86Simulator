#include "x86_simulator.h"
#include "ui_manager.h"
#include "decoder.h"
#include "i_database_manager.h"
#include "architecture.h"

// Constructor with DatabaseManager injection
X86Simulator::X86Simulator(IDatabaseManager& db_manager, Memory& memory, int session_id, bool headless)
    : db_manager_(db_manager),
      memory_(memory),
      register_map_(),
      architecture_(create_x86_architecture()),
      session_id_(session_id),
      headless_(headless),
      instructionPointer_(0),
      program_size_in_bytes_(0),
      rflags_(0) {
   if (!headless_) {
    ui_ = std::make_unique<UIManager>(memory_);
    ui_->setRegisterMap(&register_map_);
  }
  register_map_.set64("rsp", memory_.get_stack_bottom());
  rflags_ |= (1ULL << RFLAGS_ALWAYS_SET_BIT_1);
}

X86Simulator::~X86Simulator() {
    if (ui_) {
        ui_->tearDown();
    }
}

void X86Simulator::init(const std::string& program_name) {
  session_id_ = db_manager_.createSession(program_name);
}
