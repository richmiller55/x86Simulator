# x86 Simulator: GEMINI Project

## Project Overview

This project is an x86 simulator built in C++17, designed to execute x86 instructions and simulate the core components of a computer system. It is integrated with a PostgreSQL database and features a text-based user interface built with ncurses.

### Key Technologies

*   **Language:** C++17
*   **Compiler:** `g++`
*   **Build System:** `Make`
*   **Database:** PostgreSQL (using `libpqxx`)
*   **UI:** `ncurses`
*   **Test Framework:** `gtest`

### Architecture

The simulator is structured around these key classes:

*   `X86Simulator`: The main class that orchestrates the overall simulation process.
*   `DatabaseManager`: Handles all interactions with the PostgreSQL database.
*   `RegisterMap`: Manages the state of the CPU's registers.
*   `Memory`: Simulates the computer's memory for instruction and data storage.
*   `UIManager`: Manages the ncurses-based text user interface.
*   `Decoder`: Responsible for decoding x86 instructions for execution.

## Building and Running

### Building the Simulator

To compile the source files and create the `x86simulator` executable, use the following `make` command:

```bash
make -f Makefile.mk


Task Requirements: Adding a New Instruction
To add a new x86 instruction to the simulator, the following files and functions need to be updated:
CodeGenerator.cpp: Update the process_line function.
decoder.cpp: Add logic to the Decoder() constructor.
instruction_describer.cpp: Add a new case to the describe function.
x86_simulator_state.cpp: Implement the new instruction's logic in the executeInstruction function.
x86_simulator_private_helpers.cpp: Add a new helper function as needed.
x86_simulator.h: Add the necessary function declaration. 
Development Conventions
Coding Style
Variable Naming: Use snake_case for all variable names.
Member Variables: Member variables will have a trailing underscore (e.g., decoder_).
Filenames: Use snake_case.cpp for all source files.
General Style: The code uses header guards and separates concerns into different classes for clarity. 
Database Integration
Connection String: The connection to the PostgreSQL database is configured via the DB_CONN_STR environment variable.
Testing
Test Framework: The project uses the gtest framework for unit testing. New instructions should be acco