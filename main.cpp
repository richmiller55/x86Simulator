#include "x86_simulator.h"
#include <iostream>

int main() {

  DatabaseManager dbManager("dbname=simulators user=rich password=cljy57zeGJV39M8rxKGE host=steelwork port=5432");
  X86Simulator sim(dbManager);

    //    sim.displayRegistersControlled(); // Call the new display method
  int session_id = dbManager.createSession("testPrg");
  sim.set_session_id(session_id);
    sim.loadProgram("./programs/program1.asm");    
    uint64_t instruction_pointer = sim.getRegister("RIP");
    sim.log(session_id, "Loaded program", "INFO", instruction_pointer, "main.cpp", 14); 
    sim.firstPass();    
    sim.secondPass();    



 
    // To see the update, you'd call displayRegistersControlled again,
    // or ideally, have a more granular update mechanism.
 
    // sim.displayRegistersControlled();
  return 0;
}
