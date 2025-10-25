#ifndef REGISTER_MAP_H
#define REGISTER_MAP_H

#include <map>
#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include "architecture.h"
#include "avx_core.h" // For m256i_t
#include "i_register_map.h"

class RegisterMap : public IRegisterMap {
public:
  explicit RegisterMap(const Architecture& arch);
  RegisterMap(const RegisterMap&) = delete;
  RegisterMap& operator=(const RegisterMap&) = delete;

  void getValue(const std::string& reg_name, void* dest_buffer, size_t size_bytes) const override;
  void setValue(const std::string& reg_name, const void* src_buffer, size_t size_bytes) override;

  uint64_t get64(const std::string& reg_name) const;
  void set64(const std::string& reg_name, uint64_t value);
  uint32_t get32(const std::string& reg_name) const;
  void set32(const std::string& reg_name, uint32_t value);
  uint16_t get16(const std::string& reg_name) const;
  void set16(const std::string& reg_name, uint16_t value);
  uint8_t get8(const std::string& reg_name) const;
  void set8(const std::string& reg_name, uint8_t value);
  m256i_t getYmm(const std::string& reg_name) const;
  void setYmm(const std::string& reg_name, m256i_t value);

  float get_float(const std::string& reg_name) const override;
  void set_float(const std::string& reg_name, float value) override;
  double get_double(const std::string& reg_name) const override;
  void set_double(const std::string& reg_name, double value) override;

  const std::map<std::string, size_t>& getGprMap64() const { return gpr_map_64_; }
  const std::map<std::string, size_t>& getGprMap32() const { return gpr_map_32_; }
  const std::map<std::string, size_t>& getGprMap16() const { return gpr_map_16_; }
  const std::map<std::string, size_t>& getGprMap8() const { return gpr_map_8_; }
  const std::map<std::string, size_t>& getIpMap() const { return ip_map_; }
  const std::map<std::string, size_t>& getFlagsMap() const { return flags_map_; }
  const std::map<std::string, size_t>& getVectorMap() const { return vector_map_; }
  const std::map<std::string, size_t>& getSegmentMap() const { return segment_map_; }

private:
    // Maps for different register sizes/types to a byte offset in the register_buffer_
    std::map<std::string, size_t> gpr_map_64_;
    std::map<std::string, size_t> gpr_map_32_;
    std::map<std::string, size_t> gpr_map_16_;
    std::map<std::string, size_t> gpr_map_8_;
    std::map<std::string, size_t> ip_map_;
    std::map<std::string, size_t> flags_map_;
    std::map<std::string, size_t> vector_map_;
    std::map<std::string, size_t> segment_map_;

    // A single buffer to hold all register data, managed by a unique_ptr with a custom deleter for aligned memory.
    std::unique_ptr<uint8_t[], void(*)(void*)> register_buffer_;
};

#endif // REGISTER_MAP_H
