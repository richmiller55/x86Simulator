#ifndef REGISTER_MAP_H
#define REGISTER_MAP_H

#include <map>
#include <string>
#include <vector>
#include <tuple>
#include <cstdint>
#include "register_enums.h"

class RegisterMap {
private:
  std::map<std::string, Reg64> register_name_map_64_;
  std::map<std::string, Reg32> register_name_map_32_;
  std::vector<uint64_t> registers64_;
  std::vector<uint64_t> registers32_; 
  std::vector<uint16_t> RegSeg_;
public:
  RegisterMap();
  bool init();
  uint64_t get64(const std::string& reg_name) const;
  void set64(const std::string& reg_name, uint64_t value);
  uint64_t get32(const std::string& reg_name) const;
  void set32(const std::string& reg_name, uint64_t value);
  const std::map<std::string, Reg64>& getRegisterNameMap64() const;
  const  std::map<std::string, Reg32>& getRegisterNameMap32() const;
};

#endif // REGISTER_MAP_H
