#ifndef REGISTER_MAP_H
#define REGISTER_MAP_H

#include <map>
#include <string>
#include <vector>
#include <tuple>
#include <cstdint>
#include "avx_core.h" // For m256i_t
#include "register_enums.h"

class RegisterMap {
private:
  std::map<std::string, Reg64> register_name_map_64_;
  std::map<std::string, Reg32> register_name_map_32_;
  std::map<std::string, Reg64> register_name_map_16_;
  std::map<std::string, Reg64> register_name_map_8_;
  std::map<std::string, RegYMM> register_name_map_ymm_;
  std::vector<uint64_t> registers64_; 
  std::vector<m256i_t> registers_ymm_; // YMM registers
  std::vector<uint16_t> RegSeg_;
public:
  explicit RegisterMap();
  const std::map<std::string, RegYMM>& getRegisterNameMapYmm() const;
  uint64_t get64(const std::string& reg_name) const;
  void set64(const std::string& reg_name, uint64_t value);
  uint64_t get32(const std::string& reg_name) const;
  void set32(const std::string& reg_name, uint64_t value);
  uint16_t get16(const std::string& reg_name) const;
  void set16(const std::string& reg_name, uint16_t value);
  uint8_t get8(const std::string& reg_name) const;
  void set8(const std::string& reg_name, uint8_t value);
  m256i_t getYmm(const std::string& reg_name) const;
  void setYmm(const std::string& reg_name, m256i_t value);
  const std::map<std::string, Reg64>& getRegisterNameMap64() const;
  const std::map<std::string, Reg32>& getRegisterNameMap32() const;
};

#endif // REGISTER_MAP_H
