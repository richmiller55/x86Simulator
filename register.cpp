#include "register.h" // Include the header with the class declaration

Register::Register(std::string shortName,
                   std::string description)
  : name_(shortName), description_(description), value_(0) {} // Initialize value_

Register::Register(std::string shortName,
                   std::string description,
                   uint64_t value)
  : name_(shortName),
    description_(description),
    value_(value) {}

bool Register::update(uint64_t newValue){
    if (value_ != newValue) {
        previousValue_ = value_; // Store the current value as previous before updating
        value_ = newValue;
        return true; // Value changed
    }
    return false;
}


uint64_t Register::getValue() const {
    return value_;
}

void Register::display() const {
    std::cout << "reg: " << name_ << "  " << value_ << std::endl;
}
