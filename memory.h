// Memory.h
#ifndef X86SIMULATOR_MEMORY_H
#define X86SIMULATOR_MEMORY_H

#include <cstdint>
#include <vector>
#include <cstddef>
#include <stdexcept>
#include <memory>
#include <immintrin.h>

typedef uint64_t address_t;

class Memory {
public:
  // Member variable declarations
  Memory();
  Memory(size_t text_size, size_t data_size, size_t bss_size);

  uint8_t read_text(address_t address) const;
  void write_text(address_t address, uint8_t value);

  uint32_t read_text_dword(address_t address) const;
  void write_text_dword(address_t address, uint32_t value);

  uint8_t read_data(address_t address) const;
  void write_data(address_t address, uint8_t value);

  uint32_t read_data_dword(address_t address) const;
  void write_data_dword(address_t address, uint32_t value);

  __m256i read_ymm(address_t address) const;
  void write_ymm(address_t address, __m256i value);

  uint64_t read64(address_t address) const;
  void write64(address_t address, uint64_t value);

  uint64_t read_qword(address_t address) const;
  void write_qword(address_t address, uint64_t value);

    uint32_t read_dword(address_t address) const;

  uint64_t read_stack(address_t address) const;
  void write_stack(address_t address, uint64_t value);

  void reset();
  size_t get_total_memory_size() const;

  // Getters for memory layout
  size_t get_text_segment_start() const { return text_segment_start; }
  size_t get_text_segment_size() const { return text_segment_size; }
  void set_text_segment_size(size_t size) { text_segment_size = size; }
  size_t get_data_segment_start() const { return data_segment_start; }
  size_t get_bss_segment_start() const { return bss_segment_start; }
  size_t get_heap_segment_start() const { return heap_segment_start; }
  address_t get_stack_bottom() const { return stack_bottom; }

private:
  friend class CodeGenerator; // Allow CodeGenerator to set text_segment_size
  friend class X86Simulator;  // Allow X86Simulator to access memory layout

  // Memory layout and segment definitions
  size_t text_segment_start;
  size_t text_segment_size;
  size_t data_segment_start;
  size_t bss_segment_start;
  size_t heap_segment_start;
  size_t initial_heap_size;
  size_t max_stack_size;
  size_t total_memory_size;

  // Stack boundaries and pointers
  address_t stack_segment_start;
  address_t stack_segment_end;
  address_t stack_bottom;
  size_t stack_pointer;

  // Heap pointer
  address_t next_available_heap_address;

  // The actual memory storage
  std::unique_ptr<std::vector<uint8_t>> main_memory;
};

#endif // X86SIMULATOR_MEMORY_H
