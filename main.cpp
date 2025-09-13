#include "x86_simulator.h"
#include <iostream>
#include <cstdlib>

int main() {
  DatabaseManager dbManager("dbname=simulators user=rich host=steelwork port=5432");

  int session_id = dbManager.createSession("testPrg");

  X86Simulator sim(dbManager);

  sim.set_session_id(session_id);
  sim.loadProgram("./programs/program1.asm");    
  uint64_t instruction_pointer = sim.getRegister("rip");
  sim.log(session_id, "Loaded program", "INFO",
	  instruction_pointer, "main.cpp", 14); 
  sim.updateDisplay(); 
  sim.firstPass();    
  sim.secondPass();    
  sim.runProgram();


  return 0;
}
