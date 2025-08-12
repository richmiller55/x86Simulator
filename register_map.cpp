#include "register_map.h" // Include the header with the class and makeMap declarations

// Definition of the makeMap function
std::map<std::string, Register>
makeMap(const std::vector<std::tuple<std::string, std::string>>& regs) {
    std::map<std::string, Register> registerMap;
    for (auto& [name, desc] : regs) {
        registerMap.emplace(name, Register{name, desc, 0});
    }
    return registerMap;
}

RegisterMap::RegisterMap(const std::vector<std::tuple<std::string, std::string>>& regs)
  : map_(makeMap(regs)) {}
