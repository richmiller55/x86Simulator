#include "gtest/gtest.h"
#include "../memory.h"

TEST(MemoryTest, DefaultConstructor) {
    Memory mem;
    EXPECT_EQ(mem.get_text_segment_start(), 0);
    EXPECT_EQ(mem.get_data_segment_start(), 0x200000);
    EXPECT_EQ(mem.get_bss_segment_start(), 0x400000);
    EXPECT_EQ(mem.get_heap_segment_start(), 0x500000);
    EXPECT_EQ(mem.get_total_memory_size(), 0x1600000);
    EXPECT_EQ(mem.get_stack_bottom(), 0x1600000);
    EXPECT_EQ(mem.get_text_segment_size(), 0x200000);
}

TEST(MemoryTest, ParameterizedConstructor) {
    size_t text_size = 4096; // 4KB
    size_t data_size = 8192; // 8KB
    size_t bss_size = 16384; // 16KB
    Memory mem(text_size, data_size, bss_size);

    EXPECT_EQ(mem.get_text_segment_size(), text_size);
    EXPECT_EQ(mem.get_data_segment_start(), text_size);
    EXPECT_EQ(mem.get_bss_segment_start(), text_size + data_size);
    EXPECT_EQ(mem.get_heap_segment_start(), text_size + data_size + bss_size);
    
    size_t expected_total_size = text_size + data_size + bss_size + 0x1000000 + 0x100000;
    EXPECT_EQ(mem.get_total_memory_size(), expected_total_size);
    EXPECT_EQ(mem.get_stack_bottom(), expected_total_size);
}

TEST(MemoryTest, ReadWriteText) {
    Memory mem;
    address_t addr = mem.get_text_segment_start() + 10;
    uint8_t value = 0xAB;
    mem.write_text(addr, value);
    EXPECT_EQ(mem.read_text(addr), value);
}

TEST(MemoryTest, ReadWriteTextDword) {
    Memory mem;
    address_t addr = mem.get_text_segment_start() + 20;
    uint32_t value = 0xDEADBEEF;
    mem.write_text_dword(addr, value);
    EXPECT_EQ(mem.read_text_dword(addr), value);
}

TEST(MemoryTest, ReadWrite64) {
    Memory mem;
    address_t addr = mem.get_data_segment_start() + 8;
    uint64_t value = 0xDEADBEEFCAFEBABE;
    mem.write64(addr, value);
    EXPECT_EQ(mem.read64(addr), value);
}

TEST(MemoryTest, OutOfBoundsRead) {
    Memory mem;
    address_t addr = mem.get_total_memory_size() + 1; // An address guaranteed to be out of bounds
    EXPECT_THROW(mem.read_text(addr), std::out_of_range);
}

TEST(MemoryTest, OutOfBoundsWrite) {
    Memory mem;
    address_t addr = mem.get_total_memory_size() + 1; // An address guaranteed to be out of bounds
    EXPECT_THROW(mem.write_text(addr, 0), std::out_of_range);
}
