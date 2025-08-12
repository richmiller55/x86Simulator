#ifndef REGISTER_MAP_H
#define REGISTER_MAP_H

#include <map>    // For std::map
#include <string> // For std::string
#include <vector> // For std::vector
#include <tuple>  // For std::tuple

#include "register.h" // Include the Register class header

// Forward declaration of the makeMap function
// This function takes a const reference to a vector of tuples and returns a map.
std::map<std::string, Register>
makeMap(const std::vector<std::tuple<std::string, std::string>>& regs);

class RegisterMap {
public:
    RegisterMap(const std::vector<std::tuple<std::string, std::string>>& regs);

    auto begin()    { return map_.begin(); }
    auto end()      { return map_.end();   }
    auto find(const std::string& n) { return map_.find(n); }

private:
    std::map<std::string, Register> map_;
};

#endif // REGISTER_MAP_H
