#include "x86_simulator.h"
#include <iostream>
#include <cstdlib>

int main() {
    const char* conn_str_env = std::getenv("DB_CONN_STR");
    if (conn_str_env == nullptr) {
        std::cerr << "Error: DB_CONN_STR environment variable not set." << std::endl;
        return 1;
    }
    DatabaseManager dbManager(conn_str_env);

    int session_id = dbManager.createSession("testPrg");

    X86Simulator sim(dbManager);

  sim.set_session_id(session_id);
  sim.loadProgram("./programs/program1.asm");
  sim.firstPass();
  sim.secondPass(); // This now sets RIP to the start address
  sim.dumpTextSegment("text_segment_dump.txt");

  uint64_t instruction_pointer = sim.getRegister("rip");
  sim.log(session_id, "Loaded program, starting execution", "INFO", instruction_pointer, "main.cpp", 21);

  sim.runProgram();


  return 0;
}
