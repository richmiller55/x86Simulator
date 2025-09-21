#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <ncurses.h>
#include "formatting_utils.h" // Moved here to avoid potential conflicts
#include <vector>
#include <map>
#include <memory> // For std::unique_ptr
#include <fstream>
#include "register_map.h"
#include "memory.h"
#include "program_decoder.h"

struct DecodedInstruction; // Forward declaration

class UIManager {
public:
  UIManager(const Memory& memory_instance);
  ~UIManager();

  void tearDown();
  void drawRegisters(const RegisterMap& regs);
  void drawTextWindow(address_t current_rip);
  void drawInstructionDescription(address_t current_rip, const RegisterMap& regs);
  void drawLegend();
  void refreshAll();
  bool waitForInput();
  void setProgramDecoder(std::unique_ptr<ProgramDecoder> decoder);
  void setRegisterMap(const RegisterMap* regs); // New method to set current RegisterMap

private:
  void drawRegisterWindow(WINDOW* win, const std::string& title,
			  const RegisterMap& regs,
			  std::vector<std::string> order,
			  YmmViewMode current_ymm_view_mode, DisplayBase current_display_base);
  void drawTextSegment(WINDOW* win, const std::string& title, address_t current_rip);

  WINDOW *win32_;
  WINDOW *win64_;
  WINDOW *win_text_segment_;
  WINDOW *win_ymm_;
  WINDOW *win_instruction_description_;
  WINDOW *win_legend_;
  const Memory& memory_;
  size_t text_scroll_offset_;

  YmmViewMode ymm_view_mode_;
  DisplayBase display_base_;
  const RegisterMap* current_regs_; // Pointer to the current RegisterMap for input handling
  const std::vector<std::string> RegisterDisplayOrderYMM_ = {
    "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7",
    "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15"
  };


  std::unique_ptr<ProgramDecoder> program_decoder_;
};

#endif // UI_MANAGER_H