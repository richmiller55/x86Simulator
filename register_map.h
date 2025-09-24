#ifndef REGISTER_MAP_H
#define REGISTER_MAP_H

#include <map>
#include <string>
#include <vector>
#include <tuple>
#include <cstdint>
#include <immintrin.h> // For __m256i
#include "register_enums.h"

// Custom allocator for aligned memory
template <typename T, size_t Alignment>
class AlignedAllocator : public std::allocator<T> {
public:
    using pointer = T*;

    template <typename U>
    struct rebind {
        using other = AlignedAllocator<U, Alignment>;
    };

    pointer allocate(size_t n) {
        void* p = _mm_malloc(n * sizeof(T), Alignment);
        if (!p) {
            throw std::bad_alloc();
        }
        return static_cast<pointer>(p);
    }

    void deallocate(pointer p, size_t) {
        _mm_free(p);
    }
};

class RegisterMap {
private:
  std::map<std::string, Reg64> register_name_map_64_;
  std::map<std::string, Reg32> register_name_map_32_;
  std::map<std::string, RegYMM> register_name_map_ymm_;
  std::vector<uint64_t> registers64_; 
  std::vector<__m256i, AlignedAllocator<__m256i, 32>> registers_ymm_; // YMM registers
  std::vector<uint16_t> RegSeg_;
public:
  RegisterMap();
  const std::map<std::string, RegYMM>& getRegisterNameMapYmm() const;
  uint64_t get64(const std::string& reg_name) const;
  void set64(const std::string& reg_name, uint64_t value);
  uint64_t get32(const std::string& reg_name) const;
  void set32(const std::string& reg_name, uint64_t value);
  uint8_t get8(const std::string& reg_name) const;
  void set8(const std::string& reg_name, uint8_t value);
  __m256i getYmm(const std::string& reg_name) const;
  void setYmm(const std::string& reg_name, __m256i value);
  const std::map<std::string, Reg64>& getRegisterNameMap64() const;
  const std::map<std::string, Reg32>& getRegisterNameMap32() const;
};

#endif // REGISTER_MAP_H
