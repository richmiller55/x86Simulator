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

const std::vector<std::tuple<std::string, std::string>> Registers {
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


std::map<std::string, Register>
makeMap(const std::vector<std::tuple<std::string, std::string>>& regs) {
    std::map<std::string, Register> registerMap;
    for (const auto& [name, desc] : regs) {
        registerMap.emplace(name, Register{name, desc, 1000});
    }
    return registerMap;
}

class X86Simulator {
public:
  X86Simulator(){
    stack_ = std::vector<uint64_t>(1000);
    registers_ = makeMap(Registers);
  }

  bool update(const std::string& name, uint64_t newValue) {
    auto it = registers_.find(name);
    if (it == registers_.end()) return false;
    it->second.update(newValue);
    return true;
  }

  void  display() {
    for (const auto& [name, regit] : registers_) {

      std::cout << name << ": " << regit.getValue() << "\n";
    }
  }
  
  uint64_t getValue(std::string regName)
  {
    auto it = registers_.find(regName);

    if (it != registers_.end()) {
      // it->first  is the key
      // it->second is the Register object
      uint64_t value = it->second.getValue();
      return value;
    }
    else return 0;
  }

private:
  std::map<std::string, Register>  registers_;
  std::vector<uint64_t> stack_;
};


int main() {
  X86Simulator sim;
  sim.display();
  uint64_t newValue = 6000;
  sim.update("es", newValue);
  std::cout << "---" << std::endl;
  sim.display();
  return 0;
}
