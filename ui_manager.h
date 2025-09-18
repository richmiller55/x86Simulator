#ifndef UI_MANAGER_H
#define UI_MANAGER_H


#include <ncurses.h>
#include <vector>
#include <map>
#include <memory> // For std::unique_ptr
#include "register_map.h"
#include "memory.h"

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
  void preDecodeProgram();

private:
  void drawRegisterWindow(WINDOW* win, const std::string& title,
			  const RegisterMap& regs,
			  std::vector<std::string> order);
  void drawTextSegment(WINDOW* win, const std::string& title, address_t current_rip);

  WINDOW *win32_;
  WINDOW *win64_;
  WINDOW *win_text_segment_;
  WINDOW *win_instruction_description_;
  const Memory& memory_;
  size_t text_scroll_offset_;

  std::vector<std::unique_ptr<DecodedInstruction>> decoded_program_;
  std::map<address_t, size_t> address_to_index_map_;
};

#endif // UI_MANAGER_H
