// Memory.h
#ifndef X86SIMULATOR_MEMORY_H
#define X86SIMULATOR_MEMORY_H

#include <cstdint>
#include <vector>
#include <cstddef>
#include <stdexcept>

typedef uint64_t address_t;

class Memory {
public:
  // Member variable declarations
  Memory();
  Memory(size_t text_size, size_t data_size, size_t bss_size);
  size_t text_segment_start;
  size_t text_segment_size;
  size_t data_segment_start;
  size_t bss_segment_start;
  size_t heap_segment_start;
  address_t stack_bottom;

  uint8_t read_text(address_t address) const;
  void write_text(address_t address, uint8_t value);

  uint64_t read64(address_t address) const;
  void write64(address_t address, uint64_t value);

  uint64_t read_stack(address_t address) const;
  void write_stack(address_t address, uint64_t value);

  void reset();
private:
  std::vector<uint8_t> main_memory;
  size_t total_memory_size;
  address_t stack_segment_start;
  address_t stack_segment_end;
  size_t stack_pointer;
  address_t next_available_heap_address;

  size_t initial_heap_size;
  size_t max_stack_size;
};

#endif // X86SIMULATOR_MEMORY_H