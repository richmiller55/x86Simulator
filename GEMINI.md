# Simulator: GEMINI Project

## Project Overview

This project is an simulator built in C++17, designed to execute instructions and simulate the core components of a computer system. It is integrated with a PostgreSQL database and features a text-based user interface built with ncurses. It strives to be a ISA simulator with a IR stucture . currently limited coverage is implemented for Intel 86 and even less coverage for ARM, Plans for future support for powerPC RISC-V, The simulator will support pipelining

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

*   `SystemBus`: Reads a config file. system_bus.json that sets process up
*   `ISimulator`: An interface for a generic instruction set simulator
*   `DatabaseManager`: Handles all interactions with the PostgreSQL database.
*   `RegisterMap`: Manages the state of the CPU's registers.
*   `Memory`: Simulates the computer's memory for instruction and data storage.
*   `UIManager`: Manages the ncurses-based text user interface.
*   `Decoder`: Responsible for decoding x86 instructions for execution.
*   `IROpcode`: Generic/Scalar Operations


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

Target directory: `/home/rmiller/src/cpp/simulators/x86/`

## todays todo: flesh out ARM support
resolve failing tests,
add firstPass secondPass functions to Arm_simulator_core

## Future Work: Multi-ISA Support

This section outlines a design for a multi-ISA (Instruction Set Architecture) simulator. The core idea is to create a generalized simulation engine by abstracting away the specifics of any single architecture like x86, ARM, or RISC-V. This is achieved through a powerful Intermediate Representation (IR) and a modular design.

### Summary of Design Goals

The simulator is being be refactored to support multiple architectures by creating a generic core that operates on an abstract instruction set. This involves creating "front-end" parsers for each specific ISA (like x86, ARM) that translate assembly code into a universal, architecture-agnostic format (the IR). The main simulator engine will then execute these abstract instructions, managing a generalized model of registers and memory.

### Specific TODO List

