#include "memory.h"
#include <iostream>
#include <algorithm> // For std::fill and std::copy

// Constructor for the default memory layout
Memory::Memory()
  : text_segment_start(0),
    data_segment_start(0x200000),
    bss_segment_start(0x400000)
{
    // These member variables are initialized in the constructor's body for clarity.
    text_segment_size = data_segment_start - text_segment_start;
    heap_segment_start = bss_segment_start + initial_heap_size;
    total_memory_size = heap_segment_start + initial_heap_size + max_stack_size;
    
    // Allocate main memory using std::vector and smart pointer.
    main_memory = std::make_unique<std::vector<uint8_t>>(total_memory_size, 0);

    // Explicitly zero the BSS segment.
    std::fill(main_memory->begin() + bss_segment_start,
              main_memory->begin() + heap_segment_start, 0);

    // Set up stack boundaries and pointer.
    stack_bottom = total_memory_size;
    stack_pointer = stack_bottom;
    stack_segment_end = stack_bottom;
    stack_segment_start = stack_bottom - max_stack_size;
}

// Constructor for a custom memory layout
Memory::Memory(size_t text_size, size_t data_size, size_t bss_size)
  : text_segment_start(0),
    text_segment_size(text_size),
    data_segment_start(text_size),
    bss_segment_start(text_size + data_size)
{
    // Calculate memory layout based on provided sizes.
    heap_segment_start = bss_segment_start + bss_size;
    total_memory_size = heap_segment_start + initial_heap_size + max_stack_size;

    // Allocate main memory using std::vector and smart pointer.
    main_memory = std::make_unique<std::vector<uint8_t>>(total_memory_size, 0);
    
    // Explicitly zero the BSS segment.
    std::fill(main_memory->begin() + bss_segment_start,
              main_memory->begin() + heap_segment_start, 0);

    // Set up stack boundaries and pointer.
    stack_bottom = total_memory_size;
    stack_pointer = stack_bottom;
    stack_segment_end = stack_bottom;
    stack_segment_start = stack_bottom - max_stack_size;
}

// Generic bounds checking helper function
void Memory::check_bounds(address_t address, size_t size) const {
    if (address + size > main_memory->size() || address < 0) {
        throw std::out_of_range("Memory access out of bounds!");
    }
}

// Accessors with updated implementation for text segment
uint8_t Memory::read_text(address_t address) const {
    if (address < text_segment_start || address >= (text_segment_start + text_segment_size)) {
        throw std::out_of_range("Text segment read out of bounds!");
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
    if (address < text_segment_start || address + 4 > (text_segment_start + text_segment_size)) {
        throw std::out_of_range("Text segment read out of bounds!");
    }
    return *reinterpret_cast<uint32_t*>(main_memory->data() + address);
}

void Memory::write_text_dword(address_t address, uint32_t value) {
    if (address < text_segment_start || address + 4 > (text_segment_start + text_segment_size)) {
        throw std::out_of_range("Text segment write out of bounds!");
    }
    *reinterpret_cast<uint32_t*>(main_memory->data() + address) = value;
}

// Generic byte access
uint8_t Memory::read_byte(address_t address) const {
    check_bounds(address, 1);
    return main_memory->at(address);
}

void Memory::write_byte(address_t address, uint8_t value) {
    check_bounds(address, 1);
    main_memory->at(address) = value;
}

// Accessors for data segment
uint8_t Memory::read_data(address_t address) const {
    if (address < data_segment_start || address >= bss_segment_start) {
        throw std::out_of_range("Data segment read out of bounds!");
    }
    return main_memory->at(address);
}

void Memory::write_data(address_t address, uint8_t value) {
    if (address < data_segment_start || address >= bss_segment_start) {
        throw std::out_of_range("Data segment write out of bounds!");
    }
    main_memory->at(address) = value;
}

uint32_t Memory::read_data_dword(address_t address) const {
    if (address < data_segment_start || address + 4 > bss_segment_start) {
        throw std::out_of_range("Data segment read out of bounds!");
    }
    return *reinterpret_cast<uint32_t*>(main_memory->data() + address);
}

void Memory::write_data_dword(address_t address, uint32_t value) {
    if (address < data_segment_start || address + 4 > bss_segment_start) {
        throw std::out_of_range("Data segment write out of bounds!");
    }
    *reinterpret_cast<uint32_t*>(main_memory->data() + address) = value;
}

// AVX2 read/write
m256i_t Memory::read_ymm(address_t address) const {
    check_bounds(address, 32); // 32 bytes for AVX2 YMM register
    return _mm256_loadu_si256_sim(main_memory->data() + address);
}

void Memory::write_ymm(address_t address, m256i_t value) {
    check_bounds(address, 32); // 32 bytes for AVX2 YMM register
    _mm256_storeu_si256_sim(main_memory->data() + address, value);
}

// Generic 64-bit read/write using reinterpret_cast
uint64_t Memory::read64(address_t address) const {
    check_bounds(address, 8);
    return *reinterpret_cast<const uint64_t*>(main_memory->data() + address);
}

void Memory::write64(address_t address, uint64_t value) {
    check_bounds(address, 8);
    *reinterpret_cast<uint64_t*>(main_memory->data() + address) = value;
}

// 64-bit accessor aliases
uint64_t Memory::read_qword(address_t address) const {
    return read64(address);
}

void Memory::write_qword(address_t address, uint64_t value) {
    write64(address, value);
}

// Generic 32-bit read using reinterpret_cast
uint32_t Memory::read_dword(address_t address) const {
    check_bounds(address, 4);
    return *reinterpret_cast<const uint32_t*>(main_memory->data() + address);
}

// Stack accessors
uint64_t Memory::read_stack(address_t address) const {
    if (address < stack_segment_start || address >= stack_segment_end) {
        throw std::out_of_range("Stack segment read out of bounds!");
    }
    return read64(address);
}

void Memory::write_stack(address_t address, uint64_t value) {
    if (address < stack_segment_start || address + 8 > stack_segment_end) {
        throw std::out_of_range("Stack segment write out of bounds!");
    }
    *reinterpret_cast<uint64_t*>(main_memory->data() + address) = value;
}

uint32_t Memory::read_stack_dword(address_t address) const {
    if (address < stack_segment_start || address + 4 > stack_segment_end) {
        throw std::out_of_range("Stack segment dword read out of bounds!");
    }
    return *reinterpret_cast<const uint32_t*>(main_memory->data() + address);
}

void Memory::write_stack_dword(address_t address, uint32_t value) {
    if (address < stack_segment_start || address + 4 > stack_segment_end) {
        throw std::out_of_range("Stack segment dword write out of bounds!");
    }
    *reinterpret_cast<uint32_t*>(main_memory->data() + address) = value;
}

// Reset function that returns to a default state
void Memory::reset() {
    // Re-initialize members to default-constructed state, avoiding assignment.
    text_segment_start = 0;
    data_segment_start = 0x200000;
    bss_segment_start = 0x400000;

    text_segment_size = data_segment_start - text_segment_start;
    heap_segment_start = bss_segment_start + initial_heap_size;
    total_memory_size = heap_segment_start + initial_heap_size + max_stack_size;
    
    main_memory = std::make_unique<std::vector<uint8_t>>(total_memory_size, 0);

    std::fill(main_memory->begin() + bss_segment_start,
              main_memory->begin() + heap_segment_start, 0);

    stack_bottom = total_memory_size;
    stack_pointer = stack_bottom;
    stack_segment_end = stack_bottom;
    stack_segment_start = stack_bottom - max_stack_size;
}

// Getter for total memory size
size_t Memory::get_total_memory_size() const {
    return total_memory_size;
}

void Memory::set_text_segment_size(size_t size) {
    text_segment_size = size;
}
