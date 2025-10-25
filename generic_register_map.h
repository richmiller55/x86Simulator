#ifndef GENERIC_REGISTER_MAP_H
#define GENERIC_REGISTER_MAP_H

#include "i_register_map.h"
#include "architecture.h"
#include <vector>
#include <map>

struct RegisterLocation {
    IRRegisterType type;
    size_t size_bytes;
    size_t offset_bytes; // Offset within the register file's byte storage
};

class GenericRegisterMap : public IRegisterMap {
public:
    explicit GenericRegisterMap(const Architecture& arch);

    void getValue(const std::string& reg_name, void* dest_buffer, size_t size_bytes) const override;
    void setValue(const std::string& reg_name, const void* src_buffer, size_t size_bytes) override;

    // Floating point accessors
    float get_float(const std::string& reg_name) const override;
    void set_float(const std::string& reg_name, float value) override;
    double get_double(const std::string& reg_name) const override;
    void set_double(const std::string& reg_name, double value) override;

private:
    std::map<std::string, RegisterLocation> register_locations_;
    std::map<IRRegisterType, std::vector<uint8_t>> storage_;
};

#endif // GENERIC_REGISTER_MAP_H