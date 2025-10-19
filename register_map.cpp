#include "register_map.h"
#include <iostream>
#include <stdexcept>

RegisterMap::RegisterMap(const Architecture& arch) : ip_register_(0), flags_register_(0) {
    if (arch.register_map.empty()) {
        std::cerr << "ERROR: Architecture register map is empty!" << std::endl;
    }
    size_t max_gpr_index = 0;
    size_t max_vector_index = 0;
    size_t max_segment_index = 0;
    bool gpr_found = false;
    bool vector_found = false;
    bool segment_found = false;

    for (const auto& pair : arch.register_map) {
        const IRRegisterKey& key = pair.first;
        const std::string& name = pair.second;

        switch (key.type) {
            case IRRegisterType::GPR:
                gpr_found = true;
                if (key.index > max_gpr_index) {
                    max_gpr_index = key.index;
                }
                if (key.size == 64) gpr_map_64_[name] = key.index;
                else if (key.size == 32) gpr_map_32_[name] = key.index;
                else if (key.size == 16) gpr_map_16_[name] = key.index;
                else if (key.size == 8) gpr_map_8_[name] = key.index;
                break;
            case IRRegisterType::IP:
                ip_map_[name] = key.index;
                break;
            case IRRegisterType::FLAGS:
                flags_map_[name] = key.index;
                break;
            case IRRegisterType::VECTOR:
                vector_found = true;
                if (key.index > max_vector_index) {
                    max_vector_index = key.index;
                }
                vector_map_[name] = key.index;
                break;
            case IRRegisterType::SEGMENT:
                segment_found = true;
                if (key.index > max_segment_index) {
                    max_segment_index = key.index;
                }
                segment_map_[name] = key.index;
                break;
        }
    }

    if (gpr_found) {
        gpr_registers_.resize(max_gpr_index + 1, 0);
    }
    if (vector_found) {
        vector_registers_.resize(max_vector_index + 1, _mm256_setzero_si256_sim());
    }
    if (segment_found) {
        segment_registers_.resize(max_segment_index + 1, 0);
    }
}

uint64_t RegisterMap::get64(const std::string& reg_name) const {
    if (auto it = gpr_map_64_.find(reg_name); it != gpr_map_64_.end()) {
        return gpr_registers_.at(it->second);
    }
    if (auto it = ip_map_.find(reg_name); it != ip_map_.end()) {
        return ip_register_;
    }
    if (auto it = flags_map_.find(reg_name); it != flags_map_.end()) {
        return flags_register_;
    }
    throw std::out_of_range("Invalid 64-bit register name: " + reg_name);
}

void RegisterMap::set64(const std::string& reg_name, uint64_t value) {
    if (auto it = gpr_map_64_.find(reg_name); it != gpr_map_64_.end()) {
        gpr_registers_.at(it->second) = value;
        return;
    }
    if (auto it = ip_map_.find(reg_name); it != ip_map_.end()) {
        ip_register_ = value;
        return;
    }
    if (auto it = flags_map_.find(reg_name); it != flags_map_.end()) {
        flags_register_ = value;
        return;
    }
    throw std::out_of_range("Invalid 64-bit register name: " + reg_name);
}

uint32_t RegisterMap::get32(const std::string& reg_name) const {
    if (auto it = gpr_map_32_.find(reg_name); it != gpr_map_32_.end()) {
        return static_cast<uint32_t>(gpr_registers_.at(it->second));
    }
    if (auto it = ip_map_.find(reg_name); it != ip_map_.end()) {
        return static_cast<uint32_t>(ip_register_);
    }
    if (auto it = flags_map_.find(reg_name); it != flags_map_.end()) {
        return static_cast<uint32_t>(flags_register_);
    }
    throw std::out_of_range("Invalid 32-bit register name: " + reg_name);
}

void RegisterMap::set32(const std::string& reg_name, uint32_t value) {
    if (auto it = gpr_map_32_.find(reg_name); it != gpr_map_32_.end()) {
        gpr_registers_.at(it->second) = value; // Zero-extends to 64 bits
        return;
    }
    if (auto it = ip_map_.find(reg_name); it != ip_map_.end()) {
        ip_register_ = value;
        return;
    }
    if (auto it = flags_map_.find(reg_name); it != flags_map_.end()) {
        flags_register_ = value;
        return;
    }
    throw std::out_of_range("Invalid 32-bit register name: " + reg_name);
}

uint16_t RegisterMap::get16(const std::string& reg_name) const {
    if (auto it = gpr_map_16_.find(reg_name); it != gpr_map_16_.end()) {
        return static_cast<uint16_t>(gpr_registers_.at(it->second));
    }
    if (auto it = ip_map_.find(reg_name); it != ip_map_.end()) {
        return static_cast<uint16_t>(ip_register_);
    }
    if (auto it = flags_map_.find(reg_name); it != flags_map_.end()) {
        return static_cast<uint16_t>(flags_register_);
    }
    if (auto it = segment_map_.find(reg_name); it != segment_map_.end()) {
        return segment_registers_.at(it->second);
    }
    throw std::out_of_range("Invalid 16-bit register name: " + reg_name);
}

void RegisterMap::set16(const std::string& reg_name, uint16_t value) {
    if (auto it = gpr_map_16_.find(reg_name); it != gpr_map_16_.end()) {
        gpr_registers_.at(it->second) = (gpr_registers_.at(it->second) & 0xFFFFFFFFFFFF0000) | value;
        return;
    }
    if (auto it = ip_map_.find(reg_name); it != ip_map_.end()) {
        ip_register_ = (ip_register_ & 0xFFFFFFFFFFFF0000) | value;
        return;
    }
    if (auto it = flags_map_.find(reg_name); it != flags_map_.end()) {
        flags_register_ = (flags_register_ & 0xFFFFFFFFFFFF0000) | value;
        return;
    }
    if (auto it = segment_map_.find(reg_name); it != segment_map_.end()) {
        segment_registers_.at(it->second) = value;
        return;
    }
    throw std::out_of_range("Invalid 16-bit register name: " + reg_name);
}

uint8_t RegisterMap::get8(const std::string& reg_name) const {
    if (auto it = gpr_map_8_.find(reg_name); it != gpr_map_8_.end()) {
        return static_cast<uint8_t>(gpr_registers_.at(it->second));
    }
    throw std::out_of_range("Invalid 8-bit register name: " + reg_name);
}

void RegisterMap::set8(const std::string& reg_name, uint8_t value) {
    if (auto it = gpr_map_8_.find(reg_name); it != gpr_map_8_.end()) {
        gpr_registers_.at(it->second) = (gpr_registers_.at(it->second) & 0xFFFFFFFFFFFFFF00) | value;
        return;
    }
    throw std::out_of_range("Invalid 8-bit register name: " + reg_name);
}

m256i_t RegisterMap::getYmm(const std::string& reg_name) const {
    if (auto it = vector_map_.find(reg_name); it != vector_map_.end()) {
        return vector_registers_.at(it->second);
    }
    throw std::out_of_range("Invalid YMM register name: " + reg_name);
}

void RegisterMap::setYmm(const std::string& reg_name, m256i_t value) {
    if (auto it = vector_map_.find(reg_name); it != vector_map_.end()) {
        vector_registers_.at(it->second) = value;
        return;
    }
    throw std::out_of_range("Invalid YMM register name: " + reg_name);
}