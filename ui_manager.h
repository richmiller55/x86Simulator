#ifndef UI_MANAGER_H
#define UI_MANAGER_H


#include <ncurses.h>
#include "register_map.h"
#include "memory.h"

class UIManager {
public:
  UIManager(const Memory& memory_instance);
  ~UIManager();

  void tearDown();
  void drawRegisters(const RegisterMap& regs64);
  void drawTextWindow();
  void drawTextSegment(WINDOW* win, const std::string& title);

  void refreshAll();
  bool waitForInput();
private:
  WINDOW *win32;  
  WINDOW *win64;
  WINDOW *win_text_segment;
  const Memory& memory_;
  void drawRegisterWindow(WINDOW* win, const std::string& title,
			  const RegisterMap& regs,
			  std::vector<std::string> order);

};

#endif // UI_MANAGER_H
