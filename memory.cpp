// Memory.cpp
#include "memory.h"

Memory::Memory()
  : text_segment_start(0),
    data_segment_start(0x200000),
    bss_segment_start(0x400000),
    initial_heap_size(0x1000000),
    max_stack_size(0x100000)
{
  heap_segment_start = bss_segment_start + 0x100000;
  total_memory_size = heap_segment_start + initial_heap_size + max_stack_size;
  main_memory.resize(total_memory_size);

  for (size_t i = bss_segment_start; i < heap_segment_start; ++i) {
    main_memory[i] = 0;
  }

  next_available_heap_address = heap_segment_start;
  stack_pointer = total_memory_size;
}

Memory::Memory(size_t text_size, size_t data_size, size_t bss_size)
  : text_segment_start(0),
    initial_heap_size(0x1000000),
    max_stack_size(0x100000)
{
  data_segment_start = text_segment_start + text_size;
  bss_segment_start = data_segment_start + data_size;
  heap_segment_start = bss_segment_start + bss_size;

  total_memory_size = heap_segment_start + initial_heap_size + max_stack_size;
  main_memory.resize(total_memory_size);

  for (size_t i = bss_segment_start; i < heap_segment_start; ++i) {
    main_memory[i] = 0;
  }

  next_available_heap_address = heap_segment_start;
  stack_pointer = total_memory_size;
}

uint64_t Memory::read_text(address_t address) const {
  if (address < text_segment_start || address >= data_segment_start) {
    throw std::out_of_range("Text segment out of bounds!");
  }
  return main_memory[address];
}

void Memory::write_data(address_t address, uint64_t value) {
  if (address < data_segment_start || address >= bss_segment_start) {
    throw std::out_of_range("Data segment out of bounds!");
  }
  main_memory[address] = value;
}

uint64_t Memory::read_data(address_t address) const {
  if (address < data_segment_start || address >= bss_segment_start) {
    throw std::out_of_range("Data segment out of bounds!");
  }
  return main_memory[address];
}

void Memory::write_bss(address_t address, uint64_t value) {
  if (address < bss_segment_start || address >= heap_segment_start) {
    throw std::out_of_range("BSS segment out of bounds!");
  }
  main_memory[address] = value;
}

uint64_t Memory::read_bss(address_t address) const {
  if (address < bss_segment_start || address >= heap_segment_start) {
    throw std::out_of_range("BSS segment out of bounds!");
  }
  return main_memory[address];
}

void Memory::push(uint64_t value) {
  if (stack_pointer <= next_available_heap_address) {
    throw std::runtime_error("Stack overflow (collided with heap)!");
  }
  stack_pointer--;
  main_memory[stack_pointer] = value;
}

uint64_t Memory::pop() {
  if (stack_pointer >= total_memory_size) {
    throw std::runtime_error("Stack underflow!");
  }
  uint64_t value = main_memory[stack_pointer];
  stack_pointer++;
  return value;
}

address_t Memory::allocate_heap(size_t size_in_words) {
  if (next_available_heap_address + size_in_words >= stack_pointer) {
    throw std::runtime_error("Heap out of memory (collided with stack)!");
  }
  address_t allocated_address = next_available_heap_address;
  next_available_heap_address += size_in_words;
  return allocated_address;
}

void Memory::write_heap(address_t address, uint64_t value) {
  if (address < heap_segment_start || address >= next_available_heap_address) {
    throw std::out_of_range("Heap segment out of bounds!");
  }
  main_memory[address] = value;
}

uint64_t Memory::read_heap(address_t address) const {
  if (address < heap_segment_start || address >= next_available_heap_address) {
    throw std::out_of_range("Heap segment out of bounds!");
  }
  return main_memory[address];
}
