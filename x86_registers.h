#ifndef X86_REGISTERS_H
#define X86_REGISTERS_H

#include <string>
#include <vector>
#include <tuple> 
#include "register.h"
#include <filesystem> 

// Declarations of the register vectors using 'extern'
// This tells the compiler these variables are defined elsewhere.
extern const std::vector<std::tuple<std::string, std::string>> Registers64;
extern const std::vector<std::tuple<std::string, std::string>> Registers32;
extern const std::vector<std::tuple<std::string, std::string>> Registers16;

#endif // X86_REGISTERS_H
