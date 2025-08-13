#include "x86_registers.h" // Include your header file with the extern declarations

// Definitions of the register vectors (without 'extern')
const std::vector<std::tuple<std::string, std::string>> Registers64 {
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

const std::vector<std::tuple<std::string, std::string>> Registers32 {
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

const std::vector<std::tuple<std::string, std::string>> Registers16 {
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

const std::vector<std::string> RegisterDisplayOrder64 = {
    "rax", "rbx", "rcx", "rdx",
    "rsi", "rdi", "rbp", "rsp",
    "r8", "r9", "r10", "r11",
    "r12", "r13", "r14", "r15",
    "rip", // Instruction pointer usually shown last
    "cs", "ds", "es", "fs", "gs", "ss", // Segment registers
    "rflags" 
};

// Define the custom display order for 32-bit registers
const std::vector<std::string> RegisterDisplayOrder32 = {
    "eax", "ebx", "ecx", "edx",
    "esi", "edi", "ebp", "esp",
    "eip",
    "eflags"
};
