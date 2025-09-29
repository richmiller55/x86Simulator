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

bool X86Simulator::get_OF() const { return (rflags_ >> RFLAGS_OF_BIT) & 1; }
void X86Simulator::set_OF(bool value) {
  if (value) rflags_ |= (1ULL << RFLAGS_OF_BIT);
  else rflags_ &= ~(1ULL << RFLAGS_OF_BIT);
}

bool X86Simulator::get_DF() const { return (rflags_ >> RFLAGS_DF_BIT) & 1; }
void X86Simulator::set_DF(bool value) {
  if (value) rflags_ |= (1ULL << RFLAGS_DF_BIT);
  else rflags_ &= ~(1ULL << RFLAGS_DF_BIT);
}

bool X86Simulator::get_AF() const { return (rflags_ >> RFLAGS_AF_BIT) & 1; }
void X86Simulator::set_AF(bool value) {
  if (value) rflags_ |= (1ULL << RFLAGS_AF_BIT);
  else rflags_ &= ~(1ULL << RFLAGS_AF_BIT);
}

bool X86Simulator::get_PF() const { return (rflags_ >> RFLAGS_PF_BIT) & 1; }
void X86Simulator::set_PF(bool value) {
  if (value) rflags_ |= (1ULL << RFLAGS_PF_BIT);
  else rflags_ &= ~(1ULL << RFLAGS_PF_BIT);
}

void X86Simulator::update_rflags_in_register_map() {
    register_map_.set64("rflags", rflags_);
}
