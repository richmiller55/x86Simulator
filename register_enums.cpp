#include "x86_simulator.h"


RegisterEnum stringToRegister(const std::string& reg_str){
    // A static map is efficient as it's initialized only once.
    // For case-insensitivity, convert reg_str to uppercase first.
    static const std::map<std::string, RegisterEnum> reg_map = {
        {"EAX", RegisterEnum::EAX},
        {"EBX", RegisterEnum::EBX},
        {"ECX", RegisterEnum::ECX},
        {"EDX", RegisterEnum::EDX},
        {"ESI", RegisterEnum::ESI},
        {"EDI", RegisterEnum::EDI},
        {"EBP", RegisterEnum::EBP},
        {"ESP", RegisterEnum::ESP},

        {"RAX", RegisterEnum::RAX},
        {"RBX", RegisterEnum::RBX},
        {"RCX", RegisterEnum::RCX},
        {"RDX", RegisterEnum::RDX},
        {"RSI", RegisterEnum::RSI},
        {"RDI", RegisterEnum::RDI},
        {"RBP", RegisterEnum::RBP},
        {"RSP", RegisterEnum::RSP},
        {"R8", RegisterEnum::R8},
        {"R9", RegisterEnum::R9},
        {"R10", RegisterEnum::R10},
        {"R11", RegisterEnum::R11},
        {"R12", RegisterEnum::R12},
        {"R13", RegisterEnum::R13},
        {"R14", RegisterEnum::R14},
        {"R15", RegisterEnum::R15},

        {"RIP", RegisterEnum::RIP},
        {"EFLAGS", RegisterEnum::EFLAGS},
        {"RFLAGS", RegisterEnum::RFLAGS},

        {"CS", RegisterEnum::CS},
        {"DS", RegisterEnum::DS},
        {"ES", RegisterEnum::ES},
        {"FS", RegisterEnum::FS},
        {"GS", RegisterEnum::GS},
        {"SS", RegisterEnum::SS}
    };

    // Convert input string to uppercase for case-insensitive matching
    std::string upper_reg_str = reg_str;
    for (char &c : upper_reg_str) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }

    auto it = reg_map.find(upper_reg_str);
    if (it != reg_map.end()) {
        return it->second;
    }
    return RegisterEnum::UNKNOWN_REG; // Return unknown if not found
}
