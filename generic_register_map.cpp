#include "generic_register_map.h"
#include <stdexcept>
#include <cstring>
#include <iostream>

GenericRegisterMap::GenericRegisterMap(const Architecture& arch) {
    std::cout << "GenericRegisterMap constructor called" << std::endl;
    for (const auto& [type, file_def] : arch.register_files) {
        size_t total_size = 0;
        for (const auto& phys_reg : file_def.registers) {
            total_size += phys_reg.size_bits / 8;
        }
        storage_[type].resize(total_size, 0);

        size_t current_offset = 0;
        for (const auto& phys_reg : file_def.registers) {
            for (const auto& alias : phys_reg.aliases) {
                RegisterLocation loc;
                loc.type = type;
                loc.size_bytes = alias.size_bits / 8;
                loc.offset_bytes = current_offset + (alias.offset_bits / 8);
                register_locations_[alias.name] = loc;
            }
            current_offset += phys_reg.size_bits / 8;
        }
    }
}

void GenericRegisterMap::getValue(const std::string& reg_name, void* dest_buffer, size_t size_bytes) const {
    auto it = register_locations_.find(reg_name);
    if (it == register_locations_.end()) {
        throw std::out_of_range("Register not found: " + reg_name);
    }

    const RegisterLocation& loc = it->second;
    if (size_bytes != loc.size_bytes) {
        throw std::runtime_error("Mismatched size for register " + reg_name);
    }

    const std::vector<uint8_t>& reg_file = storage_.at(loc.type);
    
    if (loc.offset_bytes + size_bytes > reg_file.size()) {
        throw std::out_of_range("Register access out of bounds for " + reg_name);
    }

    std::memcpy(dest_buffer, &reg_file[loc.offset_bytes], size_bytes);
}

void GenericRegisterMap::setValue(const std::string& reg_name, const void* src_buffer, size_t size_bytes) {
    auto it = register_locations_.find(reg_name);
    if (it == register_locations_.end()) {
        throw std::out_of_range("Register not found: " + reg_name);
    }

    const RegisterLocation& loc = it->second;
    if (size_bytes != loc.size_bytes) {
        throw std::runtime_error("Mismatched size for register " + reg_name);
    }

    std::vector<uint8_t>& reg_file = storage_.at(loc.type);

    if (loc.offset_bytes + size_bytes > reg_file.size()) {
        throw std::out_of_range("Register access out of bounds for " + reg_name);
    }

    std::memcpy(&reg_file[loc.offset_bytes], src_buffer, size_bytes);
}

float GenericRegisterMap::get_float(const std::string& reg_name) const {
    float value;
    getValue(reg_name, &value, sizeof(value));
    return value;
}

void GenericRegisterMap::set_float(const std::string& reg_name, float value) {
    setValue(reg_name, &value, sizeof(value));
}

double GenericRegisterMap::get_double(const std::string& reg_name) const {
    double value;
    getValue(reg_name, &value, sizeof(value));
    return value;
}

void GenericRegisterMap::set_double(const std::string& reg_name, double value) {
    setValue(reg_name, &value, sizeof(value));
}