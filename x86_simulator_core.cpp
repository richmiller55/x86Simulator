#include "x86_simulator.h"
#include "ui_manager.h"
#include "decoder.h"

// Constructor with DatabaseManager injection
X86Simulator::X86Simulator(DatabaseManager& dbManager, int session_id)
    : dbManager_(dbManager),
      register_map_(),
      memory_(),
      rflags_(0),
      session_id_(session_id) {
  ui_ = std::make_unique<UIManager>(memory_);
  rflags_ |= (1ULL << RFLAGS_ALWAYS_SET_BIT_1);
  ui_->setRegisterMap(&register_map_);

};

X86Simulator::~X86Simulator() {
    ui_->tearDown();
}

void X86Simulator::init(const std::string& program_name) {
  session_id_ = dbManager_.createSession(program_name);
};

 int X86Simulator::get_session_id() const {
    return session_id_;
 };

void X86Simulator::updateDisplay() {
  address_t current_rip = register_map_.get64("rip");
  ui_->drawRegisters(register_map_);
  ui_->drawTextWindow(current_rip);
  ui_->drawInstructionDescription(current_rip, register_map_);
  ui_->refreshAll();
}
// X86Simulator.cpp (using the new RegisterMap)
void X86Simulator::push(uint64_t value) {
    uint64_t current_rsp = register_map_.get64("rsp");
    current_rsp -= 8;
    register_map_.set64("rsp", current_rsp);

    // Stack overflow check (example, adjust as needed)
    if (current_rsp < memory_.stack_bottom) { // stack grows down
        throw std::runtime_error("Stack overflow");
    }
    // 8 is the size of a pushed value (64-bit)
    memory_.write_stack(current_rsp, value);
}

// Get the value of a register, handling different sizes with fallthrough
uint64_t X86Simulator::getRegister(const std::string& register_name) {
    // 1. Try to find a 64-bit register.
    const auto& map64 = register_map_.getRegisterNameMap64();
    if (auto it = map64.find(register_name); it != map64.end()) {
      return register_map_.get64(register_name);
    }

    // 2. Fall back to finding a 32-bit register.
    const auto& map32 = register_map_.getRegisterNameMap32();
    if (auto it = map32.find(register_name); it != map32.end()) {
        // Special logic for 32-bit registers, if needed.
        // For example, reading the lower 32 bits from the corresponding 64-bit register.
        // For now, let's assume a separate 32-bit register map.
      return register_map_.get32(register_name);
    }

    // 3. Fall back to finding a segment register.
    /*    const auto& seg_map = register_map_.getSegmentRegisterMap();
    if (auto it = seg_map.find(register_name); it != seg_map.end()) {
        // Assume you have a separate map for segment registers.
        return seg_map.get(it->second);
    }
    */
    // 4. No register found.
      throw std::runtime_error("Invalid register name: " + register_name);
}
void X86Simulator::log(int session_id,
			  const std::string& message, 
			  const std::string& level,
			  uint64_t instruction_pointer,
			  const std::string& source_file,
			  int source_line) {
  dbManager_.log(session_id, message, level, instruction_pointer, source_file, source_line);
}
