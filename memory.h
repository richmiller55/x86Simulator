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
  // Member variable declarations (without initializers here, unless 'static const')
  size_t text_segment_start;
  size_t data_segment_start;
  size_t bss_segment_start;
  size_t heap_segment_start;
  size_t stack_segment_start;

  size_t initial_heap_size;
  size_t max_stack_size;

  // Constructor declarations (prototypes)
  Memory();
  Memory(size_t text_size, size_t data_size, size_t bss_size);

  // Member function declarations (prototypes)
  void push(uint64_t value);
  uint64_t pop();

  uint64_t read_text(address_t address) const;
  void write_text(address_t address, uint64_t value);

  void write_data(address_t address, uint64_t value);
  uint64_t read_data(address_t address) const;

  void write_bss(address_t address, uint64_t value);
  uint64_t read_bss(address_t address) const;

  address_t allocate_heap(size_t size_in_words);

  void write_heap(address_t address, uint64_t value);
  uint64_t read_heap(address_t address) const;

private:
  std::vector<uint64_t> main_memory;
  size_t total_memory_size;

  size_t stack_pointer;
  address_t next_available_heap_address;
};

#endif // X86SIMULATOR_MEMORY_H
