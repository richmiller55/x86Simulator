#ifndef X86SIMULATOR_MEMORY_H
#define X86SIMULATOR_MEMORY_H

#include <cstdint>
#include <vector>
#include <cstddef>
#include <stdexcept>
#include <memory>
#include "avx_core.h"

// Use a fixed-size integer type for addresses for clarity and portability.
// A 64-bit unsigned integer is appropriate for a 64-bit simulator.
using address_t = uint64_t;

class Memory {
public:
  // Public interface and constructors
  Memory();
  Memory(size_t text_size, size_t data_size, size_t bss_size);

  // Accessors for different memory segments
  uint8_t read_text(address_t address) const;
  void write_text(address_t address, uint8_t value);
  uint32_t read_text_dword(address_t address) const;
  void write_text_dword(address_t address, uint32_t value);
  uint8_t read_byte(address_t address) const;
  void write_byte(address_t address, uint8_t value);
  uint64_t read64(address_t address) const;
  void write64(address_t address, uint64_t value);
  uint8_t read_data(address_t address) const;
  void write_data(address_t address, uint8_t value);
  uint32_t read_data_dword(address_t address) const;
  void write_data_dword(address_t address, uint32_t value);

  uint64_t read_stack(address_t address) const;
  void write_stack(address_t address, uint64_t value);
  uint32_t read_stack_dword(address_t address) const;
  void write_stack_dword(address_t address, uint32_t value);

  // AVX2 read/write
  m256i_t read_ymm(address_t address) const;
  void write_ymm(address_t address, m256i_t value);

  // Helper functions for various data sizes
  uint64_t read_qword(address_t address) const;
  void write_qword(address_t address, uint64_t value);
  uint32_t read_dword(address_t address) const;
  void write_dword(address_t address, uint32_t value);
  uint16_t read_word(address_t address) const;
  void write_word(address_t address, uint16_t value);

  // Management functions
  void reset();
  size_t get_total_memory_size() const;
  void set_text_segment_size(size_t size);

  // Getters for memory layout
  size_t get_text_segment_start() const { return text_segment_start; }
  size_t get_text_segment_size() const { return text_segment_size; }
  size_t get_data_segment_start() const { return data_segment_start; }
  size_t get_bss_segment_start() const { return bss_segment_start; }
  size_t get_heap_segment_start() const { return heap_segment_start; }
  address_t get_stack_bottom() const { return stack_bottom; }
  address_t get_stack_segment_start() const { return stack_segment_start; }

private:
  // Helper for bounds checking
  void check_bounds(address_t address, size_t size) const;

  // Main memory and layout details
  std::unique_ptr<std::vector<uint8_t>> main_memory;

  // Memory segment boundaries
  address_t text_segment_start;
  size_t text_segment_size;
  address_t data_segment_start;
  address_t bss_segment_start;
  address_t heap_segment_start;
  size_t total_memory_size;
  
  // Stack management
  address_t stack_segment_start;
  address_t stack_segment_end;
  address_t stack_bottom;
  address_t stack_pointer;
  
  // Initial size constants
  const size_t initial_heap_size = 0x1000000;
  const size_t max_stack_size = 0x100000;

};

#endif // X86SIMULATOR_MEMORY_H
