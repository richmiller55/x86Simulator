#include "register_map.h"
#include <iostream>
#include <stdexcept>
#include <numeric>
#include <immintrin.h> // For _mm_malloc and _mm_free

// Helper to align values up to the nearest multiple of alignment
static size_t align_up(size_t value, size_t alignment) {
    return (value + alignment - 1) & ~(alignment - 1);
}

RegisterMap::RegisterMap(const Architecture& arch) : register_buffer_(nullptr, nullptr) {
    size_t current_offset = 0;

    const std::vector<IRRegisterType> layout_order = {
        IRRegisterType::GPR,
        IRRegisterType::IP,
        IRRegisterType::FLAGS,
        IRRegisterType::SEGMENT,
        IRRegisterType::VECTOR
    };

    std::map<IRRegisterType, size_t> file_base_offsets;
    for (IRRegisterType type : layout_order) {
        if (arch.register_files.count(type) == 0) continue;

        const auto& file_def = arch.register_files.at(type);
        size_t alignment = (type == IRRegisterType::VECTOR) ? 32 : 1;
        current_offset = align_up(current_offset, alignment);
        file_base_offsets[type] = current_offset;

        size_t file_size = 0;
        for(const auto& reg_def : file_def.registers) {
            file_size += reg_def.size_bits / 8;
        }
        current_offset += file_size;
    }

    // Allocate the buffer with alignment and manage it with a unique_ptr with a custom deleter.
    void* raw_ptr = _mm_malloc(current_offset, 32);
    if (!raw_ptr) { throw std::bad_alloc(); }
    register_buffer_.reset(static_cast<uint8_t*>(raw_ptr));
    register_buffer_.get_deleter() = [](void* p){ _mm_free(p); };
    std::fill(register_buffer_.get(), register_buffer_.get() + current_offset, 0);


    for (const auto& [type, file_def] : arch.register_files) {
        size_t file_base_offset = file_base_offsets.at(type);
        size_t current_phys_reg_offset = 0;

        for (const auto& phys_reg : file_def.registers) {
            for (const auto& alias : phys_reg.aliases) {
                size_t final_offset = file_base_offset + current_phys_reg_offset + (alias.offset_bits / 8);
                switch (type) {
                    case IRRegisterType::GPR:
                        if (alias.size_bits == 64) gpr_map_64_[alias.name] = final_offset;
                        else if (alias.size_bits == 32) gpr_map_32_[alias.name] = file_base_offset + current_phys_reg_offset;
                        else if (alias.size_bits == 16) gpr_map_16_[alias.name] = final_offset;
                        else if (alias.size_bits == 8) gpr_map_8_[alias.name] = final_offset;
                        break;
                    case IRRegisterType::IP:
                        ip_map_[alias.name] = final_offset; break;
                    case IRRegisterType::FLAGS:
                        flags_map_[alias.name] = final_offset; break;
                    case IRRegisterType::VECTOR:
                        vector_map_[alias.name] = final_offset; break;
                    case IRRegisterType::SEGMENT:
                        segment_map_[alias.name] = final_offset; break;
                }
            }
            current_phys_reg_offset += phys_reg.size_bits / 8;
        }
    }
}

// --- Generic Get/Set (Interface implementation) ---
void RegisterMap::getValue(const std::string& reg_name, void* dest_buffer, size_t size_bytes) const {
    const uint8_t* src_ptr = nullptr;
    if (auto it = gpr_map_64_.find(reg_name); it != gpr_map_64_.end()) src_ptr = register_buffer_.get() + it->second;
    else if (auto it = gpr_map_32_.find(reg_name); it != gpr_map_32_.end()) src_ptr = register_buffer_.get() + it->second;
    else if (auto it = gpr_map_16_.find(reg_name); it != gpr_map_16_.end()) src_ptr = register_buffer_.get() + it->second;
    else if (auto it = gpr_map_8_.find(reg_name); it != gpr_map_8_.end()) src_ptr = register_buffer_.get() + it->second;
    else if (auto it = ip_map_.find(reg_name); it != ip_map_.end()) src_ptr = register_buffer_.get() + it->second;
    else if (auto it = flags_map_.find(reg_name); it != flags_map_.end()) src_ptr = register_buffer_.get() + it->second;
    else if (auto it = vector_map_.find(reg_name); it != vector_map_.end()) src_ptr = register_buffer_.get() + it->second;
    else if (auto it = segment_map_.find(reg_name); it != segment_map_.end()) src_ptr = register_buffer_.get() + it->second;

    if (src_ptr) {
        std::copy(src_ptr, src_ptr + size_bytes, static_cast<uint8_t*>(dest_buffer));
    } else {
        throw std::out_of_range("Register not found in getValue: " + reg_name);
    }
}

void RegisterMap::setValue(const std::string& reg_name, const void* src_buffer, size_t size_bytes) {
    uint8_t* dest_ptr = nullptr;
    if (auto it = gpr_map_64_.find(reg_name); it != gpr_map_64_.end()) dest_ptr = register_buffer_.get() + it->second;
    else if (auto it = gpr_map_32_.find(reg_name); it != gpr_map_32_.end()) dest_ptr = register_buffer_.get() + it->second;
    else if (auto it = gpr_map_16_.find(reg_name); it != gpr_map_16_.end()) dest_ptr = register_buffer_.get() + it->second;
    else if (auto it = gpr_map_8_.find(reg_name); it != gpr_map_8_.end()) dest_ptr = register_buffer_.get() + it->second;
    else if (auto it = ip_map_.find(reg_name); it != ip_map_.end()) dest_ptr = register_buffer_.get() + it->second;
    else if (auto it = flags_map_.find(reg_name); it != flags_map_.end()) dest_ptr = register_buffer_.get() + it->second;
    else if (auto it = vector_map_.find(reg_name); it != vector_map_.end()) dest_ptr = register_buffer_.get() + it->second;
    else if (auto it = segment_map_.find(reg_name); it != segment_map_.end()) dest_ptr = register_buffer_.get() + it->second;

    if (dest_ptr) {
        std::copy(static_cast<const uint8_t*>(src_buffer), static_cast<const uint8_t*>(src_buffer) + size_bytes, dest_ptr);
    } else {
        throw std::out_of_range("Register not found in setValue: " + reg_name);
    }
}

// --- Size-specific accessors ---
uint64_t RegisterMap::get64(const std::string& reg_name) const {
    if (auto it = gpr_map_64_.find(reg_name); it != gpr_map_64_.end()) return *reinterpret_cast<const uint64_t*>(register_buffer_.get() + it->second);
    if (auto it = ip_map_.find(reg_name); it != ip_map_.end()) return *reinterpret_cast<const uint64_t*>(register_buffer_.get() + it->second);
    if (auto it = flags_map_.find(reg_name); it != flags_map_.end()) return *reinterpret_cast<const uint64_t*>(register_buffer_.get() + it->second);
    if (auto it = vector_map_.find(reg_name); it != vector_map_.end()) return *reinterpret_cast<const uint64_t*>(register_buffer_.get() + it->second);
    throw std::out_of_range("Invalid 64-bit register name: " + reg_name);
}

void RegisterMap::set64(const std::string& reg_name, uint64_t value) {
    if (auto it = gpr_map_64_.find(reg_name); it != gpr_map_64_.end()) { *reinterpret_cast<uint64_t*>(register_buffer_.get() + it->second) = value; return; }
    if (auto it = ip_map_.find(reg_name); it != ip_map_.end()) { *reinterpret_cast<uint64_t*>(register_buffer_.get() + it->second) = value; return; }
    if (auto it = flags_map_.find(reg_name); it != flags_map_.end()) { *reinterpret_cast<uint64_t*>(register_buffer_.get() + it->second) = value; return; }
    if (auto it = vector_map_.find(reg_name); it != vector_map_.end()) { *reinterpret_cast<uint64_t*>(register_buffer_.get() + it->second) = value; return; }
    throw std::out_of_range("Invalid 64-bit register name: " + reg_name);
}

uint32_t RegisterMap::get32(const std::string& reg_name) const {
    if (auto it = gpr_map_32_.find(reg_name); it != gpr_map_32_.end()) return *reinterpret_cast<const uint32_t*>(register_buffer_.get() + it->second);
    if (auto it = ip_map_.find(reg_name); it != ip_map_.end()) return *reinterpret_cast<const uint32_t*>(register_buffer_.get() + it->second);
    if (auto it = flags_map_.find(reg_name); it != flags_map_.end()) return *reinterpret_cast<const uint32_t*>(register_buffer_.get() + it->second);
    if (auto it = vector_map_.find(reg_name); it != vector_map_.end()) return *reinterpret_cast<const uint32_t*>(register_buffer_.get() + it->second);
    throw std::out_of_range("Invalid 32-bit register name: " + reg_name);
}

void RegisterMap::set32(const std::string& reg_name, uint32_t value) {
    if (auto it = gpr_map_32_.find(reg_name); it != gpr_map_32_.end()) {
        *reinterpret_cast<uint64_t*>(register_buffer_.get() + it->second) = value;
        return;
    }
    if (auto it = ip_map_.find(reg_name); it != ip_map_.end()) { *reinterpret_cast<uint32_t*>(register_buffer_.get() + it->second) = value; return; }
    if (auto it = flags_map_.find(reg_name); it != flags_map_.end()) { *reinterpret_cast<uint32_t*>(register_buffer_.get() + it->second) = value; return; }
    if (auto it = vector_map_.find(reg_name); it != vector_map_.end()) { *reinterpret_cast<uint32_t*>(register_buffer_.get() + it->second) = value; return; }
    throw std::out_of_range("Invalid 32-bit register name: " + reg_name);
}

uint16_t RegisterMap::get16(const std::string& reg_name) const {
    if (auto it = gpr_map_16_.find(reg_name); it != gpr_map_16_.end()) return *reinterpret_cast<const uint16_t*>(register_buffer_.get() + it->second);
    if (auto it = ip_map_.find(reg_name); it != ip_map_.end()) return *reinterpret_cast<const uint16_t*>(register_buffer_.get() + it->second);
    if (auto it = flags_map_.find(reg_name); it != flags_map_.end()) return *reinterpret_cast<const uint16_t*>(register_buffer_.get() + it->second);
    if (auto it = segment_map_.find(reg_name); it != segment_map_.end()) return *reinterpret_cast<const uint16_t*>(register_buffer_.get() + it->second);
    throw std::out_of_range("Invalid 16-bit register name: " + reg_name);
}

void RegisterMap::set16(const std::string& reg_name, uint16_t value) {
    if (auto it = gpr_map_16_.find(reg_name); it != gpr_map_16_.end()) { *reinterpret_cast<uint16_t*>(register_buffer_.get() + it->second) = value; return; }
    if (auto it = ip_map_.find(reg_name); it != ip_map_.end()) { *reinterpret_cast<uint16_t*>(register_buffer_.get() + it->second) = value; return; }
    if (auto it = flags_map_.find(reg_name); it != flags_map_.end()) { *reinterpret_cast<uint16_t*>(register_buffer_.get() + it->second) = value; return; }
    if (auto it = segment_map_.find(reg_name); it != segment_map_.end()) { *reinterpret_cast<uint16_t*>(register_buffer_.get() + it->second) = value; return; }
    throw std::out_of_range("Invalid 16-bit register name: " + reg_name);
}

uint8_t RegisterMap::get8(const std::string& reg_name) const {
    if (auto it = gpr_map_8_.find(reg_name); it != gpr_map_8_.end()) return *reinterpret_cast<const uint8_t*>(register_buffer_.get() + it->second);
    throw std::out_of_range("Invalid 8-bit register name: " + reg_name);
}

void RegisterMap::set8(const std::string& reg_name, uint8_t value) {
    if (auto it = gpr_map_8_.find(reg_name); it != gpr_map_8_.end()) { *reinterpret_cast<uint8_t*>(register_buffer_.get() + it->second) = value; return; }
    throw std::out_of_range("Invalid 8-bit register name: " + reg_name);
}

m256i_t RegisterMap::getYmm(const std::string& reg_name) const {
    if (auto it = vector_map_.find(reg_name); it != vector_map_.end()) return *reinterpret_cast<const m256i_t*>(register_buffer_.get() + it->second);
    throw std::out_of_range("Invalid YMM register name: " + reg_name);
}

void RegisterMap::setYmm(const std::string& reg_name, m256i_t value) {
    if (auto it = vector_map_.find(reg_name); it != vector_map_.end()) { *reinterpret_cast<m256i_t*>(register_buffer_.get() + it->second) = value; return; }
    throw std::out_of_range("Invalid YMM register name: " + reg_name);
}

float RegisterMap::get_float(const std::string& reg_name) const {
    if (auto it = vector_map_.find(reg_name); it != vector_map_.end()) return *reinterpret_cast<const float*>(register_buffer_.get() + it->second);
    throw std::out_of_range("Invalid float register name: " + reg_name);
}

void RegisterMap::set_float(const std::string& reg_name, float value) {
    if (auto it = vector_map_.find(reg_name); it != vector_map_.end()) { *reinterpret_cast<float*>(register_buffer_.get() + it->second) = value; return; }
    throw std::out_of_range("Invalid float register name: " + reg_name);
}

double RegisterMap::get_double(const std::string& reg_name) const {
    if (auto it = vector_map_.find(reg_name); it != vector_map_.end()) return *reinterpret_cast<const double*>(register_buffer_.get() + it->second);
    throw std::out_of_range("Invalid double register name: " + reg_name);
}

void RegisterMap::set_double(const std::string& reg_name, double value) {
    if (auto it = vector_map_.find(reg_name); it != vector_map_.end()) { *reinterpret_cast<double*>(register_buffer_.get() + it->second) = value; return; }
    throw std::out_of_range("Invalid double register name: " + reg_name);
}