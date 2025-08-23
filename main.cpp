#include "x86_simulator.h"
#include <iostream>

int main() {

    X86Simulator sim;
    sim.displayRegistersControlled(); // Call the new display method


    sim.ReadProgram("./programs/program1.asm");    



 
    // To see the update, you'd call displayRegistersControlled again,
    // or ideally, have a more granular update mechanism.
 
    sim.displayRegistersControlled();
  return 0;
}
