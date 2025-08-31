#include "x86_simulator.h"

// Constructor with DatabaseManager injection
X86Simulator::X86Simulator(DatabaseManager& dbManager)
    : dbManager_(dbManager), // Initialize the new member
      regs32_(Registers32),
      regs64_(Registers64),
      memory_(),
      rflags_(0) {
  rflags_ |= (1ULL << RFLAGS_ALWAYS_SET_BIT_1);
};

void X86Simulator::init(const std::string& program_name) {
  session_id_ = dbManager_.createSession(program_name);
};

 int X86Simulator::get_session_id() const {
    return session_id_;
 };

void X86Simulator::set_session_id(int session_id) {
    session_id_ = session_id;
};


uint64_t X86Simulator::getRegister(const std::string& regName) {
  if (auto it = regs32_.find(regName); it != regs32_.end()) {
    const Register& r = it->second;
    return r.getValue();
  } else {
    if (auto it = regs64_.find(regName); it != regs64_.end()) {
      const Register& r = it->second;
      return r.getValue();
    }
  }
  return 0;
}


void X86Simulator::log(int session_id,
			  const std::string& message, 
			  const std::string& level,
			  uint64_t instruction_pointer,
			  const std::string& source_file,
			  int source_line) {
  dbManager_.log(session_id, message, level, instruction_pointer, source_file, source_line);
}
