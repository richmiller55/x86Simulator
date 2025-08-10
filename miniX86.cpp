#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <iomanip>
#include <cstdint>


class Register {
public:
Register(std::string shortName,
	 std::string description):
  name_(shortName),  description_(description) {}
     
  Register(std::string shortName,
	   std::string description,
	   uint64_t value)
    : name_(shortName),
      description_(description),
      value_(value) {}

  bool update(uint64_t newValue){
    value_ = newValue;
    return true;
  }
  uint64_t getValue() const {
    return value_;
  }
  void display() const {
    std::cout << "reg: " << name_ << "  " << value_ << std::endl;
  }

private:
  std::string name_;
  std::string description_;
  uint64_t value_ ;  
};


static std::map<std::string, Register>
makeMap(const std::vector<std::tuple<std::string, std::string>>& regs) {
  std::map<std::string, Register> registerMap;
  for (auto& [name, desc] : regs) {
    registerMap.emplace(name, Register{name, desc, 0});
  }
  return registerMap;
}

class RegisterMap {
public:
  RegisterMap(const std::vector<std::tuple<std::string, std::string>>& regs)
    : map_(makeMap(regs)) {}

  auto begin()    { return map_.begin(); }
  auto end()      { return map_.end();   }
  auto find(const std::string& n) { return map_.find(n); }

private:
  std::map<std::string, Register> map_;
};

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


class X86Simulator {
public:
  X86Simulator()
    : regs32_(Registers32), regs64_(Registers64) {}

  bool update(const std::string& name, uint64_t val) {
    if (auto it = regs32_.find(name); it != regs32_.end()) {
      it->second.update(val);
      return true;
    }
    if (auto it = regs64_.find(name); it != regs64_.end()) {
      it->second.update(val);
      return true;
    }
    return false;
  }

  void display32() {
    for (auto& [n,r] : regs32_)
      std::cout << n << ": " << r.getValue() << "\n";
  }

  void display64() {
    for (auto& [n,r] : regs64_)
      std::cout << n << ": " << r.getValue() << "\n";
  }

private:
  RegisterMap regs32_;
  RegisterMap regs64_;
};


int main() {
  X86Simulator sim;
  sim.display32();
  sim.display64();
  uint64_t newValue = 6000;
  sim.update("es", newValue);
  std::cout << "---" << std::endl;
  sim.display32();
  sim.display64();
  return 0;
}
