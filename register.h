#ifndef REGISTER_H
#define REGISTER_H

#include <string>
#include <iostream> // For std::cout in display()
#include <cstdint>  // For uint64_t

class Register {
public:
    Register(std::string shortName,
             std::string description);

    Register(std::string shortName,
             std::string description,
             uint64_t value);

    bool update(uint64_t newValue);
    uint64_t getValue() const;
    void display() const;

private:
    std::string name_;
    std::string description_;
    uint64_t value_;
};

#endif // REGISTER_H
