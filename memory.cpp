// Memory.cpp
#include "memory.h"
#include <iostream>

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
  main_memory = std::make_unique<std::vector<uint8_t>>(total_memory_size);

  for (size_t i = bss_segment_start; i < heap_segment_start; ++i) {
    main_memory->at(i) = 0;
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
  main_memory = std::make_unique<std::vector<uint8_t>>(total_memory_size);

  for (size_t i = bss_segment_start; i < heap_segment_start; ++i) {
    main_memory->at(i) = 0;
  }

  next_available_heap_address = heap_segment_start;
  stack_bottom = total_memory_size;
  stack_pointer = stack_bottom;
  stack_segment_end = stack_bottom;
  stack_segment_start = stack_bottom - max_stack_size;
}

uint8_t Memory::read_text(address_t address) const {
  if (address < text_segment_start || address >= (text_segment_start + text_segment_size)) {
    throw std::out_of_range("Text segment out of bounds!");
  }
  return main_memory->at(address);
}
void Memory::write_text(address_t address, uint8_t value) {
  if (address < text_segment_start || address >= (text_segment_start + text_segment_size)) {
    throw std::out_of_range("Text segment write out of bounds!");
  }
  main_memory->at(address) = value;
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
        value |= static_cast<uint64_t>(main_memory->at(address + i)) << (i * 8);
    }
    return value;
}

void Memory::write64(address_t address, uint64_t value) {
    for (int i = 0; i < 8; ++i) {
        // Assuming little-endian byte order
        main_memory->at(address + i) = (value >> (i * 8)) & 0xFF;
    }
}

void Memory::reset() {
    // Bulletproof reset: Recalculate the entire memory layout from constructor constants
    // to defend against memory corruption that was invalidating size-related members.
    const size_t const_bss_segment_start = 0x400000;
    const size_t const_initial_heap_size = 0x1000000;
    const size_t const_max_stack_size = 0x100000;
    const size_t const_data_segment_start = 0x200000;
    const size_t const_text_segment_start = 0;

    // Re-calculate and re-assign member variables to restore a valid state
    heap_segment_start = const_bss_segment_start + 0x100000;
    total_memory_size = heap_segment_start + const_initial_heap_size + const_max_stack_size;

    // Re-allocate the main memory vector with the corrected size
    main_memory = std::make_unique<std::vector<uint8_t>>(total_memory_size, 0);

    // Reset all pointers and sizes to their initial state
    text_segment_size = const_data_segment_start - const_text_segment_start;
    next_available_heap_address = heap_segment_start;
    
    // Reset stack pointers and boundaries
    stack_bottom = total_memory_size;
    stack_pointer = stack_bottom;
    stack_segment_end = stack_bottom;
    stack_segment_start = stack_bottom - const_max_stack_size;
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

size_t Memory::get_total_memory_size() const {
    return total_memory_size;
}
