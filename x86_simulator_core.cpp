#include "x86_simulator.h"
#include "ui_manager.h"
#include "decoder.h"
#include "i_database_manager.h"
/*
#include <stdexcept>
#include <vector>
#include <string>
#include <sstream>
*/
// Constructor with DatabaseManager injection
X86Simulator::X86Simulator(IDatabaseManager& dbManager, int session_id, bool headless)
    : dbManager_(dbManager),
      memory_(),
      register_map_(),
      instructionPointer_(0),
      rflags_(0),
      session_id_(session_id),
      headless_(headless),
      program_size_in_bytes_(0) {
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
  session_id_ = dbManager_.createSession(program_name);
}

 int X86Simulator::get_session_id() const {
    return session_id_;
 }

void X86Simulator::updateDisplay() {
  if (ui_) {
    address_t current_rip = register_map_.get64("rip");
    ui_->drawMainRegisters(register_map_);
    ui_->drawYmmRegisters(register_map_);
    ui_->drawTextWindow(current_rip);
    ui_->drawInstructionDescription(current_rip, register_map_); // This already calls the correct function in UIManager
    ui_->drawLegend();
    ui_->refreshAll();
  }
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

bool X86Simulator::is_headless() const {
    return headless_;
}


// Helper to check if a string is a numeric literal
bool is_number(const std::string& s) {
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

// Calculates the size of a data directive in bytes
size_t X86Simulator::calculate_data_size(const std::vector<std::string>& tokens) {
    if (tokens.empty()) {
        return 0;
    }

    std::string directive = tokens[0];
    size_t size = 0;

    if (directive == "db" || directive == ".byte") {
        size = 1;
    } else if (directive == "dw" || directive == ".word") {
        size = 2;
    } else if (directive == "dd" || directive == ".long") {
        size = 4;
    } else if (directive == "dq" || directive == ".quad") {
        size = 8;
    } else {
        return 0; // Unknown directive
    }

    // A single directive can have multiple comma-separated values
    // e.g., dq 1, 2, 3
    size_t num_operands = tokens.size() - 1;
    return size * num_operands;
}

// Calculates the size of a BSS directive
size_t X86Simulator::calculate_bss_size(const std::vector<std::string>& tokens) {
    if (tokens.empty()) {
        return 0;
    }
    
    std::string directive = tokens[0];

    if (directive == "resb") {
        if (tokens.size() > 1 && is_number(tokens[1])) {
            return std::stoul(tokens[1]);
        }
    } else if (directive == "resw") {
        if (tokens.size() > 1 && is_number(tokens[1])) {
            return 2 * std::stoul(tokens[1]);
        }
    } else if (directive == "resd") {
        if (tokens.size() > 1 && is_number(tokens[1])) {
            return 4 * std::stoul(tokens[1]);
        }
    } else if (directive == "resq") {
        if (tokens.size() > 1 && is_number(tokens[1])) {
            return 8 * std::stoul(tokens[1]);
        }
    }

    return 0; // Unknown directive or invalid operand
}
