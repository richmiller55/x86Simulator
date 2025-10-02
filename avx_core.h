#ifndef AVX_CORE_H
#define AVX_CORE_H

#include <cstdint>

// Align the structure to a 16-byte boundary
struct alignas(16) m128i_t {
    union {
        int8_t    m128i_i8[16];   // Access as 16 packed 8-bit integers
        int16_t   m128i_i16[8];   // Access as 8 packed 16-bit integers
        int32_t   m128i_i32[4];   // Access as 4 packed 32-bit integers
        int64_t   m128i_i64[2];   // Access as 2 packed 64-bit integers
        uint8_t   m128i_u8[16];   // Access as 16 packed unsigned 8-bit integers
        uint16_t  m128i_u16[8];   // Access as 8 packed unsigned 16-bit integers
        uint32_t  m128i_u32[4];   // Access as 4 packed unsigned 32-bit integers
        uint64_t  m128i_u64[2];   // Access as 2 packed unsigned 64-bit integers
        float     m128_f32[4];    // Access as 4 packed single-precision floats
    };
};

// Align the structure to a 32-byte boundary
struct alignas(32) m256i_t {
    union {
        int8_t    m256i_i8[32];   // Access as 32 packed 8-bit integers
        int16_t   m256i_i16[16];  // Access as 16 packed 16-bit integers
        int32_t   m256i_i32[8];   // Access as 8 packed 32-bit integers
        int64_t   m256i_i64[4];   // Access as 4 packed 64-bit integers
        uint8_t   m256i_u8[32];   // Access as 32 packed unsigned 8-bit integers
        uint16_t  m256i_u16[16];  // Access as 16 packed unsigned 16-bit integers
        uint32_t  m256i_u32[8];   // Access as 8 packed unsigned 32-bit integers
        uint64_t  m256i_u64[4];   // Access as 4 packed unsigned 64-bit integers
        float     m256_f32[8];    // Access as 8 packed single-precision floats
        double    m256_f64[4];    // Access as 4 packed double-precision floats
        m128i_t   m128[2];        // Access as 2 128-bit lanes
    };
};

m256i_t _mm256_add_epi32_sim(m256i_t a, m256i_t b);
m256i_t _mm256_loadu_si256_sim(const void* mem_addr);
void _mm256_storeu_si256_sim(void* mem_addr, m256i_t a);
m256i_t _mm256_set_epi32_sim(
    int32_t i7, int32_t i6, int32_t i5, int32_t i4,
    int32_t i3, int32_t i2, int32_t i1, int32_t i0);
m256i_t _mm256_set_epi64x_sim(int64_t e3, int64_t e2, int64_t e1, int64_t e0);

// Floating point
m256i_t _mm256_add_ps_sim(m256i_t a, m256i_t b);
m256i_t _mm256_sub_ps_sim(m256i_t a, m256i_t b);
m256i_t _mm256_mul_ps_sim(m256i_t a, m256i_t b);
m256i_t _mm256_div_ps_sim(m256i_t a, m256i_t b);
m256i_t _mm256_max_ps_sim(m256i_t a, m256i_t b);
m256i_t _mm256_min_ps_sim(m256i_t a, m256i_t b);
m256i_t _mm256_rcp_ps_sim(m256i_t a);
m256i_t _mm256_sqrt_ps_sim(m256i_t a);

// 128-bit operations
m128i_t _mm_andnot_si128_sim(m128i_t a, m128i_t b);
m128i_t _mm_and_si128_sim(m128i_t a, m128i_t b);
m128i_t _mm_or_si128_sim(m128i_t a, m128i_t b);
m128i_t _mm_xor_si128_sim(m128i_t a, m128i_t b);
m128i_t _mm_mullo_epi16_sim(m128i_t a, m128i_t b);

// 256-bit lane operations
m128i_t _mm256_extractf128_si256_sim(m256i_t a, int imm8);
m256i_t _mm256_set_m128i_sim(m128i_t hi, m128i_t lo);

m256i_t _mm256_setzero_si256_sim();
m256i_t _mm256_set_ps_sim(float e7, float e6, float e5, float e4, float e3, float e2, float e1, float e0);
m256i_t _mm256_set_epi16_sim(short e15, short e14, short e13, short e12, short e11, short e10, short e9, short e8, short e7, short e6, short e5, short e4, short e3, short e2, short e1, short e0);
void _mm256_storeu_ps_sim(float* mem_addr, m256i_t a);

#endif // AVX_CORE_H