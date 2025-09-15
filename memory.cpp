// Memory.cpp
#include "memory.h"

Memory::Memory()
  : text_segment_start(0),
    data_segment_start(0x200000),
    bss_segment_start(0x400000),
    initial_heap_size(0x1000000),
    max_stack_size(0x100000)
{
  text_segment_size = data_segment_start - text_segment_start;
  heap_segment_start = bss_segment_start + 0x100000;
  total_memory_size = heap_segment_start + initial_heap_size + max_stack_size;
  main_memory.resize(total_memory_size);

  for (size_t i = bss_segment_start; i < heap_segment_start; ++i) {
    main_memory[i] = 0;
  }

  next_available_heap_address = heap_segment_start;
  stack_bottom = total_memory_size;
  stack_pointer = stack_bottom;
  stack_segment_end = stack_bottom;
  stack_segment_start = stack_bottom - max_stack_size;
}

Memory::Memory(size_t text_size, size_t data_size, size_t bss_size)
  : text_segment_start(0),
    text_segment_size(text_size),
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
  stack_bottom = total_memory_size;
  stack_pointer = stack_bottom;
  stack_segment_end = stack_bottom;
  stack_segment_start = stack_bottom - max_stack_size;
}

uint8_t Memory::read_text(address_t address) const {
  if (address < text_segment_start || address >= data_segment_start) {
    throw std::out_of_range("Text segment out of bounds!");
  }
  return main_memory[address];
}
void Memory::write_text(address_t address, uint8_t value) {
  if (address < text_segment_start || address >= data_segment_start) {
    throw std::out_of_range("Text segment write out of bounds!");
  }
  main_memory[address] = value;
}

uint32_t Memory::read_text_dword(address_t address) const {
    uint32_t value = 0;
    for (int i = 0; i < 4; ++i) {
        value |= static_cast<uint32_t>(read_text(address + i)) << (i * 8);
    }
    return value;
}

void Memory::write_text_dword(address_t address, uint32_t value) {
    for (int i = 0; i < 4; ++i) {
        write_text(address + i, (value >> (i * 8)) & 0xFF);
    }
}

uint64_t Memory::read64(address_t address) const {
    uint64_t value = 0;
    for (int i = 0; i < 8; ++i) {
        // Assuming little-endian byte order
        value |= static_cast<uint64_t>(main_memory[address + i]) << (i * 8);
    }
    return value;
}

void Memory::write64(address_t address, uint64_t value) {
    for (int i = 0; i < 8; ++i) {
        // Assuming little-endian byte order
        main_memory[address + i] = (value >> (i * 8)) & 0xFF;
    }
}

void Memory::reset() {
  main_memory.clear();
  main_memory.resize(total_memory_size);

  for (size_t i = text_segment_start; i < text_segment_start + text_segment_size; ++i) {
    main_memory[i] = 0;
  }

  for (size_t i = bss_segment_start; i < heap_segment_start; ++i) {
    main_memory[i] = 0;
  }
  next_available_heap_address = heap_segment_start;
  stack_pointer = stack_bottom;
}

uint64_t Memory::read_stack(address_t address) const {
  if (address < stack_segment_start || address >= stack_segment_end) {
    throw std::out_of_range("Stack segment read out of bounds!");
  }
  return read64(address);
}

void Memory::write_stack(address_t address, uint64_t value) {
  if (address < stack_segment_start || address >= stack_segment_end) {
    throw std::out_of_range("Stack segment write out of bounds!");
  }
  write64(address, value);
}
