#include "x86_simulator.h"

bool X86Simulator::get_CF() const {
    return (register_map_->get64("rflags") >> RFLAGS_CF_BIT) & 1;
}

void X86Simulator::set_CF(bool value) {
    uint64_t rflags = register_map_->get64("rflags");
    if (value) {
        rflags |= (1ULL << RFLAGS_CF_BIT);
    } else {
        rflags &= ~(1ULL << RFLAGS_CF_BIT);
    }
    register_map_->set64("rflags", rflags);
}

bool X86Simulator::get_ZF() const {
    return (register_map_->get64("rflags") >> RFLAGS_ZF_BIT) & 1;
}

void X86Simulator::set_ZF(bool value) {
    uint64_t rflags = register_map_->get64("rflags");
    if (value) {
        rflags |= (1ULL << RFLAGS_ZF_BIT);
    } else {
        rflags &= ~(1ULL << RFLAGS_ZF_BIT);
    }
    register_map_->set64("rflags", rflags);
}

bool X86Simulator::get_SF() const {
    return (register_map_->get64("rflags") >> RFLAGS_SF_BIT) & 1;
}

void X86Simulator::set_SF(bool value) {
    uint64_t rflags = register_map_->get64("rflags");
    if (value) {
        rflags |= (1ULL << RFLAGS_SF_BIT);
    } else {
        rflags &= ~(1ULL << RFLAGS_SF_BIT);
    }
    register_map_->set64("rflags", rflags);
}

bool X86Simulator::get_OF() const {
    return (register_map_->get64("rflags") >> RFLAGS_OF_BIT) & 1;
}

void X86Simulator::set_OF(bool value) {
    uint64_t rflags = register_map_->get64("rflags");
    if (value) {
        rflags |= (1ULL << RFLAGS_OF_BIT);
    } else {
        rflags &= ~(1ULL << RFLAGS_OF_BIT);
    }
    register_map_->set64("rflags", rflags);
}

bool X86Simulator::get_DF() const {
    return (register_map_->get64("rflags") >> RFLAGS_DF_BIT) & 1;
}

void X86Simulator::set_DF(bool value) {
    uint64_t rflags = register_map_->get64("rflags");
    if (value) {
        rflags |= (1ULL << RFLAGS_DF_BIT);
    } else {
        rflags &= ~(1ULL << RFLAGS_DF_BIT);
    }
    register_map_->set64("rflags", rflags);
}

bool X86Simulator::get_AF() const {
    return (register_map_->get64("rflags") >> RFLAGS_AF_BIT) & 1;
}

void X86Simulator::set_AF(bool value) {
    uint64_t rflags = register_map_->get64("rflags");
    if (value) {
        rflags |= (1ULL << RFLAGS_AF_BIT);
    } else {
        rflags &= ~(1ULL << RFLAGS_AF_BIT);
    }
    register_map_->set64("rflags", rflags);
}

bool X86Simulator::get_PF() const {
    return (register_map_->get64("rflags") >> RFLAGS_PF_BIT) & 1;
}

void X86Simulator::set_PF(bool value) {
    uint64_t rflags = register_map_->get64("rflags");
    if (value) {
        rflags |= (1ULL << RFLAGS_PF_BIT);
    } else {
        rflags &= ~(1ULL << RFLAGS_PF_BIT);
    }
    register_map_->set64("rflags", rflags);
}
