#ifndef I_REGISTER_MAP_H
#define I_REGISTER_MAP_H

#include <string>
#include <cstdint>

class IRegisterMap {
public:
    virtual ~IRegisterMap() = default;

    // Generic accessors for any register by name
    virtual void getValue(const std::string& reg_name, void* dest_buffer, size_t size_bytes) const = 0;
    virtual void setValue(const std::string& reg_name, const void* src_buffer, size_t size_bytes) = 0;

    // Convenience wrappers
    uint64_t get64(const std::string& reg_name) const {
        uint64_t value;
        getValue(reg_name, &value, sizeof(value));
        return value;
    }

    void set64(const std::string& reg_name, uint64_t value) {
        setValue(reg_name, &value, sizeof(value));
    }

    uint32_t get32(const std::string& reg_name) const {
        uint32_t value;
        getValue(reg_name, &value, sizeof(value));
        return value;
    }

    void set32(const std::string& reg_name, uint32_t value) {
        setValue(reg_name, &value, sizeof(value));
    }

    uint16_t get16(const std::string& reg_name) const {
        uint16_t value;
        getValue(reg_name, &value, sizeof(value));
        return value;
    }

    void set16(const std::string& reg_name, uint16_t value) {
        setValue(reg_name, &value, sizeof(value));
    }

    uint8_t get8(const std::string& reg_name) const {
        uint8_t value;
        getValue(reg_name, &value, sizeof(value));
        return value;
    }

    void set8(const std::string& reg_name, uint8_t value) {
        setValue(reg_name, &value, sizeof(value));
    }

    // Floating point accessors
    virtual float get_float(const std::string& reg_name) const = 0;
    virtual void set_float(const std::string& reg_name, float value) = 0;
    virtual double get_double(const std::string& reg_name) const = 0;
    virtual void set_double(const std::string& reg_name, double value) = 0;
};

#endif // I_REGISTER_MAP_H
