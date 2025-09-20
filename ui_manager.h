#ifndef UI_MANAGER_H
#define UI_MANAGER_H


#include <ncurses.h>
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
  void refreshAll();
  bool waitForInput();
  void setProgramDecoder(std::unique_ptr<ProgramDecoder> decoder);

private:
  void drawRegisterWindow(WINDOW* win, const std::string& title,
			  const RegisterMap& regs,
			  std::vector<std::string> order);
  void drawTextSegment(WINDOW* win, const std::string& title, address_t current_rip);

  WINDOW *win32_;
  WINDOW *win64_;
  WINDOW *win_text_segment_;
  WINDOW *win_ymm_;
  WINDOW *win_instruction_description_;
  const Memory& memory_;
  size_t text_scroll_offset_;

  std::unique_ptr<ProgramDecoder> program_decoder_;
};

#endif // UI_MANAGER_H
