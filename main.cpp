#include "x86_simulator.h"
#include <iostream>

int main() {

  DatabaseManager dbManager("dbname=simulators user=rich password=cljy57zeGJV39M8rxKGE host=steelwork port=5432");
  X86Simulator sim(dbManager);

    //    sim.displayRegistersControlled(); // Call the new display method

    sim.loadProgram("./programs/program1.asm");    
    sim.firstPass();    
    sim.secondPass();    



 
    // To see the update, you'd call displayRegistersControlled again,
    // or ideally, have a more granular update mechanism.
 
    // sim.displayRegistersControlled();
  return 0;
}
