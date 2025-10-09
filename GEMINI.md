# x86 Simulator: GEMINI Project

## Project Overview

This project is an x86 simulator built in C++17, designed to execute x86 instructions and simulate the core components of a computer system. It is integrated with a PostgreSQL database and features a text-based user interface built with ncurses.

### Key Technologies

*   **Language:** C++17
*   **Compiler:** `g++`
*   **Build System:** `Make`
*   **Database:** PostgreSQL (using `libpqxx`)
*   **UI:** `ncurses`
*   **json:** `nlohmann/json`
*   **serialization:** `cereal`
*   **Test Framework:** `gtest`

### Architecture

The simulator is structured around these key classes:

*   `X86Simulator`: The main class that orchestrates the overall simulation process.
*   `DatabaseManager`: Handles all interactions with the PostgreSQL database.
*   `RegisterMap`: Manages the state of the CPU's registers.
*   `Memory`: Simulates the computer's memory for instruction and data storage.
*   `UIManager`: Manages the ncurses-based text user interface.
*   `Decoder`: Responsible for decoding x86 instructions for execution.
*   `SystemBus`: Responsible a vector of memory object which it gives to the simulator


## Building and Running

### Building the Simulator

To compile the source files and create the `x86simulator` executable, use the following `make` command:

```bash
make -f Makefile.mk
```

### Running Tests

To run the test suite, use the following command:

```bash
make -f Makefile.mk test
```


{
"Coding Style":
Variable Naming":  "Use snake_case for all variable names.",
"Member Variables": "Member variables will have a trailing underscore (e.g., decoder_).",
"Filenames": 	    "Use snake_case.cpp for all new source files.",
"General Style": "The code uses header guards and separates concerns into different classes for clarity."
"Database Integration": {
"Connection String": "The connection to the PostgreSQL database is configured via the DB_CONN_STR environment variable."
}
"Testing": {
"Test Framework": "The project uses gtest".
"Current Tests": [
"tests decoder.cpp": "tests/decoder_test.cpp",
"tests formating_utils.cpp": "tests/formatting_utils_test.cpp",
"tests instruction_describer.cpp": "tests/instruction_describer_test.cpp",
"memory bounds testing":	   "tests/memory_test.cpp",
tests/mock_database_manager.cpp
tests/operand_parser_test.cpp
tests/program_decoder_test.cpp
tests/register_map_test.cpp
tests/rflags_test.cpp
tests/simulator_core_test.cpp
tests/system_bus_test.cpp
tests/test_main.cpp

Target directory: `/home/rmiller/src/cpp/simulators/x86/`

## Future Work: Multi-ISA Support

This section outlines a design for a multi-ISA (Instruction Set Architecture) simulator. The core idea is to create a generalized simulation engine by abstracting away the specifics of any single architecture like x86, ARM, or RISC-V. This is achieved through a powerful Intermediate Representation (IR) and a modular design.

### Summary of Design Goals

The simulator should be refactored to support multiple architectures by creating a generic core that operates on an abstract instruction set. This involves creating "front-end" parsers for each specific ISA (like x86, ARM) that translate assembly code into a universal, architecture-agnostic format (the IR). The main simulator engine will then execute these abstract instructions, managing a generalized model of registers and memory.

### Specific TODO List

1.  **Intermediate Representation (IR) Development:**
    *   Design a canonical set of operations for the IR (e.g., `Move`, `Add`, `Load`, `Store`) that can represent instructions from various ISAs.
    *   Define a flexible structure within the IR to represent diverse operand types and complex addressing modes (e.g., x86's `base + index*scale + displacement`).
    *   Abstract control flow instructions in the IR, creating a generic `Branch` operation with properties for the target address and condition codes.
    *   Create a generic representation for system calls (e.g., `Syscall`) that can be mapped to specific ISA implementations like `syscall` (x86) or `ecall` (RISC-V).

2.  **Abstraction Layer Implementation:**
    *   Abstract the register file. Instead of using specific names like `RAX` or `R0`, use generalized identifiers (e.g., `GPR[0]`, `StackPointer`).
    *   Implement a configurable memory model that can handle different endianness (big vs. little) based on the target architecture.
    *   Develop a mapping system to translate ISA-specific registers and system call mechanisms to and from the abstract IR.

3.  **Modular Architecture Refactoring:**
    *   Create distinct "front-end" parsers for each target ISA (e.g., one for x86, another for ARM) responsible for translating assembly into the IR.
    *   Refactor the core simulator engine to operate exclusively on the abstract IR, removing any direct dependencies on x86-specific logic.
    *   Implement "back-end" modules that can translate the simulator's state (from the IR) back into an architecture-specific view for debugging and output.
    *   Add a configuration system that allows the user to specify the target architecture (ISA, endianness, etc.) at startup.
