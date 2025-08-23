#include "x86_simulator.h"


    bool X86Simulator::get_CF() const { return (rflags_ >> RFLAGS_CF_BIT) & 1; }
    void X86Simulator::set_CF(bool value) {
        if (value) rflags_ |= (1ULL << RFLAGS_CF_BIT);
        else rflags_ &= ~(1ULL << RFLAGS_CF_BIT);
    }

    bool X86Simulator::get_ZF() const { return (rflags_ >> RFLAGS_ZF_BIT) & 1; }
    void X86Simulator::set_ZF(bool value) {
        if (value) rflags_ |= (1ULL << RFLAGS_ZF_BIT);
        else rflags_ &= ~(1ULL << RFLAGS_ZF_BIT);
    }

    bool X86Simulator::get_SF() const { return (rflags_ >> RFLAGS_SF_BIT) & 1; }
    void X86Simulator::set_SF(bool value) {
        if (value) rflags_ |= (1ULL << RFLAGS_SF_BIT);
        else rflags_ &= ~(1ULL << RFLAGS_SF_BIT);
    }

