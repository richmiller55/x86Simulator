# GEMINI.md: x86 Simulator

## Project Overview

This project is a command-line x86 simulator written in C++. It is designed to execute x86 assembly programs, simulating the behavior of a CPU. The simulator includes a register map, memory management, and an instruction decoder. It also integrates with a PostgreSQL database to log simulation events, manage sessions, and store snapshots. The user interface is handled by the `ncurses` library to display register values and other information.

**Key Technologies:**

*   **Language:** C++17
*   **Compiler:** g++
*   **Build System:** Make
*   **Database:** PostgreSQL (using `libpqxx`)
*   **UI:** ncurses
*   **test framework** gtest
**Architecture:**

The simulator is structured around several key classes:

*   `X86Simulator`: The main class that orchestrates the simulation.
*   `DatabaseManager`: Handles all interactions with the PostgreSQL database.
*   `RegisterMap`: Manages the state of the CPU registers.
*   `Memory`: Simulates the computer's memory.
*   `UIManager`: Manages the ncurses-based text user interface.
*   `Decoder`: Responsible for decoding x86 instructions.

## Building and Running

### Building the Simulator

To build the simulator, use the `make` command:

```bash
make
```

This will compile the source files and create an executable named `x86simulator`.

### Running the Simulator

To run the simulator, use the `make run` command:

```bash
make run
```

This will start the simulator and load the program specified in `main.cpp` (currently `./programs/program1.asm`).

### Cleaning the Build

To remove the compiled object files and the executable, use the `make clean` command:

```bash
make clean
```

## Development Conventions

*   **Coding Style:** 
    *   Variable names should use snake_case.
    *   Member variables will have a trailing underscore (e.g., `decoder_`).
    *   Filenames will use snake_case.cpp.
    *   The code follows a consistent style with the use of header guards, and clear separation of concerns into different classes.
*   **Database Integration:** The simulator is tightly integrated with a PostgreSQL database. The connection string is supplied via the `DB_CONN_STR` environment variable.
*   **Testing:** There are no explicit tests in the provided file structure.