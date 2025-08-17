#include "x86_simulator.h"
#include <iostream>

int main() {

    X86Simulator sim;

    sim.update("eax", 0x12345678);
    sim.update("rax", 0xABCDEF0123456789);
    sim.displayRegistersControlled(); // Call the new display method

    sim.update("ebx", 0xAAAA); // Update another register
    // To see the update, you'd call displayRegistersControlled again,
    // or ideally, have a more granular update mechanism.
    sim.displayRegistersControlled();

    std::cout << "\n--- Loading Program Example ---" << std::endl;
    sim.ReadProgram("./programs/program1.asm");
  return 0;
}
