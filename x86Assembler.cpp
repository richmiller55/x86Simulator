 include <iostream>
#include <vector>
#include <map>

const std::vector<RegisterDescription> BIT64_REGISTERS = {
  {"rdi", "Destination Index Register"},
  {"rsi", "Source Index Register"},
  {"rsp", "Stack Pointer Register"},
  {"rbp", "Base Pointer Register"},
  {"rax", "Accumulator Register"},
  {"rbx", "Base Register"},
  {"rcx", "Counter Register"},
  {"rdx", "Data Register"},
  {"rip", "Instruction Pointer Register"},
  {"r8", "General Purpose Register 8"},
  {"r9", "General Purpose Register 9"},
  {"r10", "General Purpose Register 10"},
  {"r11", "General Purpose Register 11"},
  {"r12", "General Purpose Register 12"},
  {"r13", "General Purpose Register 13"},
  {"r14", "General Purpose Register 14"},
  {"r15", "General Purpose Register 15"},
  {"cs", "Code Segment Register"},
  {"ds", "Data Segment Register"},
  {"fs", "Extra Segment Register"},
  {"ss", "Stack Segment Register"},
  {"es", "Extra Segment Register"},
  {"gs", "General Segment Register"},
  {"cf", "Carry Flag"},
  {"zf", "Zero Flag"},
  {"pf", "Parity Flag"},
  {"af", "Auxiliary Carry Flag"},
  {"sf", "Sign Flag"},
  {"tf", "Trap Flag"},
  {"if", "Interrupt Enable Flag"},
  {"df", "Direction Flag"},
  {"of", "Overflow Flag"},
  {"rflags", "Flags Register"} };

const std::vector<RegisterDescription> BIT32_REGISTERS = {
  {"edi", "Destination Index Register"},
  {"esi", "Source Index Register"},
  {"esp", "Stack Pointer Register"},
  {"ebp", "Base Pointer Register"},
  {"eax", "Accumulator Register"},
  {"ebx", "Base Register"},
  {"ecx", "Counter Register"},
  {"edx", "Data Register"},
  {"eip", "Instruction Pointer Register"},
  {"eflags", "Flags Register"} };

const std::vector<RegisterDescription> BIT16_REGISTERS = {
  {"ax", "Accumulator Register"},
  {"bx", "Base Register"},
  {"cx", "Counter Register"},
  {"dx", "Data Register"},
  {"si", "Source Index Register"},
  {"di", "Destination Index Register"},
  {"sp", "Stack Pointer Register"},
  {"bp", "Base Pointer Register"},
  {"cs", "Code Segment Register"},
  {"ds", "Data Segment Register"},
  {"ss", "Stack Segment Register"},
  {"es", "Extra Segment Register"},
  {"ip", "Instruction Pointer Register"},
  {"flags", "Flags Register"} };



  std::vector<uint8_t> memory; // Represents main memory    }
    // Helper functions for memory access, flag manipulation, etc.
    uint32_t read_dword(uint32_t address) { /* ... */ }
    void write_dword(uint32_t address, uint32_t value) { /* ... */ }
    // ...
};

class X86Simulator {
public:
  BIT16_REGISTERS bit16_regs;
    BIT32_REGISTERS bit32_regs;
    BIT64_REGISTERS bit64_regs;


    std::vector<uint8_t> memory;

    X86Simulator(size_t mem_size) : memory(mem_size, 0) {
        // Initialize registers to 0 or a known state
        bit16_regs = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	bit32_regs = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	bit64_regs = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		
    std::vector<uint8_t> memory; // Represents main memory    }

    void load_program(const std::vector<uint8_t>& program_bytes, uint32_t entry_point) {
        // Copy program bytes into simulated memory
        // Set EIP to the entry_point
    }

    void run() {
        while (true) {
            // Fetch instruction from memory at bit32_regs.eip
            // Decode instruction
            // Execute instruction (update various regs and memory)
            // Handle program termination or errors
        }
    }

    // Helper functions for memory access, flag manipulation, etc.
    uint32_t read_dword(uint32_t address) { /* ... */ }
    void write_dword(uint32_t address, uint32_t value) { /* ... */ }
    // ...
};


int main() {
    X86Simulator simulator(1024 * 1024); // 1MB memory
    // Load a sample x86 program (e.g., from a file or hardcoded bytes)
    // simulator.load_program(my_program_bytes, 0x1000);
    // simulator.run();
    return 0;
}

    void load_program(const std::vector<uint8_t>& program_bytes, uint32_t entry_point) {
        // Copy program bytes into simulated memory
        // Set EIP to the entry_point
    }

    void run() {
        while (true) {
            // Fetch instruction from memory at regs.eip
            // Decode instruction
            // Execute instruction (update regs and memory)
            // Handle program termination or errors
        }
    }
