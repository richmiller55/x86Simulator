#include "avx_core.h"
#include <cstdint>
#include <cstring> // For memcpy
#include <cmath>   // For sqrt

m256i_t _mm256_add_epi32_sim(m256i_t a, m256i_t b) {
    m256i_t result;
    for (int i = 0; i < 8; ++i) {
        result.m256i_i32[i] = a.m256i_i32[i] + b.m256i_i32[i];
    }
    return result;
}

m256i_t _mm256_loadu_si256_sim(const void* mem_addr) {
    m256i_t result;
    memcpy(&result, mem_addr, sizeof(m256i_t));
    return result;
}

void _mm256_storeu_si256_sim(void* mem_addr, m256i_t a) {
    memcpy(mem_addr, &a, sizeof(m256i_t));
}

m256i_t _mm256_set_epi32_sim(
    int32_t i7, int32_t i6, int32_t i5, int32_t i4,
    int32_t i3, int32_t i2, int32_t i1, int32_t i0)
{
    m256i_t result;
    result.m256i_i32[0] = i0;
    result.m256i_i32[1] = i1;
    result.m256i_i32[2] = i2;
    result.m256i_i32[3] = i3;
    result.m256i_i32[4] = i4;
    result.m256i_i32[5] = i5;
    result.m256i_i32[6] = i6;
    result.m256i_i32[7] = i7;
    return result;
}

m256i_t _mm256_set_epi64x_sim(int64_t e3, int64_t e2, int64_t e1, int64_t e0) {
    m256i_t result;
    result.m256i_i64[0] = e0;
    result.m256i_i64[1] = e1;
    result.m256i_i64[2] = e2;
    result.m256i_i64[3] = e3;
    return result;
}

// Floating point
m256i_t _mm256_add_ps_sim(m256i_t a, m256i_t b) {
    m256i_t result;
    for (int i = 0; i < 8; ++i) result.m256_f32[i] = a.m256_f32[i] + b.m256_f32[i];
    return result;
}

m256i_t _mm256_sub_ps_sim(m256i_t a, m256i_t b) {
    m256i_t result;
    for (int i = 0; i < 8; ++i) result.m256_f32[i] = a.m256_f32[i] - b.m256_f32[i];
    return result;
}

m256i_t _mm256_mul_ps_sim(m256i_t a, m256i_t b) {
    m256i_t result;
    for (int i = 0; i < 8; ++i) result.m256_f32[i] = a.m256_f32[i] * b.m256_f32[i];
    return result;
}

m256i_t _mm256_div_ps_sim(m256i_t a, m256i_t b) {
    m256i_t result;
    for (int i = 0; i < 8; ++i) result.m256_f32[i] = a.m256_f32[i] / b.m256_f32[i];
    return result;
}

m256i_t _mm256_max_ps_sim(m256i_t a, m256i_t b) {
    m256i_t result;
    for (int i = 0; i < 8; ++i) result.m256_f32[i] = a.m256_f32[i] > b.m256_f32[i] ? a.m256_f32[i] : b.m256_f32[i];
    return result;
}

m256i_t _mm256_min_ps_sim(m256i_t a, m256i_t b) {
    m256i_t result;
    for (int i = 0; i < 8; ++i) result.m256_f32[i] = a.m256_f32[i] < b.m256_f32[i] ? a.m256_f32[i] : b.m256_f32[i];
    return result;
}

m256i_t _mm256_rcp_ps_sim(m256i_t a) {
    m256i_t result;
    for (int i = 0; i < 8; ++i) result.m256_f32[i] = 1.0f / a.m256_f32[i];
    return result;
}

m256i_t _mm256_sqrt_ps_sim(m256i_t a) {
    m256i_t result;
    for (int i = 0; i < 8; ++i) result.m256_f32[i] = sqrtf(a.m256_f32[i]);
    return result;
}

// 128-bit operations
m128i_t _mm_andnot_si128_sim(m128i_t a, m128i_t b) {
    m128i_t result;
    result.m128i_u64[0] = (~a.m128i_u64[0]) & b.m128i_u64[0];
    result.m128i_u64[1] = (~a.m128i_u64[1]) & b.m128i_u64[1];
    return result;
}

m128i_t _mm_and_si128_sim(m128i_t a, m128i_t b) {
    m128i_t result;
    result.m128i_u64[0] = a.m128i_u64[0] & b.m128i_u64[0];
    result.m128i_u64[1] = a.m128i_u64[1] & b.m128i_u64[1];
    return result;
}

m128i_t _mm_or_si128_sim(m128i_t a, m128i_t b) {
    m128i_t result;
    result.m128i_u64[0] = a.m128i_u64[0] | b.m128i_u64[0];
    result.m128i_u64[1] = a.m128i_u64[1] | b.m128i_u64[1];
    return result;
}

m128i_t _mm_xor_si128_sim(m128i_t a, m128i_t b) {
    m128i_t result;
    result.m128i_u64[0] = a.m128i_u64[0] ^ b.m128i_u64[0];
    result.m128i_u64[1] = a.m128i_u64[1] ^ b.m128i_u64[1];
    return result;
}

m128i_t _mm_mullo_epi16_sim(m128i_t a, m128i_t b) {
    m128i_t result;
    for (int i = 0; i < 8; ++i) {
        result.m128i_i16[i] = a.m128i_i16[i] * b.m128i_i16[i];
    }
    return result;
}

// 256-bit lane operations
m128i_t _mm256_extractf128_si256_sim(m256i_t a, int imm8) {
    return a.m128[imm8 & 1];
}

m256i_t _mm256_set_m128i_sim(m128i_t hi, m128i_t lo) {
    m256i_t result;
    result.m128[0] = lo;
    result.m128[1] = hi;
    return result;
}

m256i_t _mm256_setzero_si256_sim() {
    m256i_t result;
    for (int i = 0; i < 4; ++i) result.m256i_u64[i] = 0;
    return result;
}

m256i_t _mm256_set_ps_sim(float e7, float e6, float e5, float e4, float e3, float e2, float e1, float e0) {
    m256i_t result;
    result.m256_f32[0] = e0;
    result.m256_f32[1] = e1;
    result.m256_f32[2] = e2;
    result.m256_f32[3] = e3;
    result.m256_f32[4] = e4;
    result.m256_f32[5] = e5;
    result.m256_f32[6] = e6;
    result.m256_f32[7] = e7;
    return result;
}

m256i_t _mm256_set_epi16_sim(short e15, short e14, short e13, short e12, short e11, short e10, short e9, short e8, short e7, short e6, short e5, short e4, short e3, short e2, short e1, short e0) {
    m256i_t result;
    result.m256i_i16[0] = e0; result.m256i_i16[1] = e1; result.m256i_i16[2] = e2; result.m256i_i16[3] = e3;
    result.m256i_i16[4] = e4; result.m256i_i16[5] = e5; result.m256i_i16[6] = e6; result.m256i_i16[7] = e7;
    result.m256i_i16[8] = e8; result.m256i_i16[9] = e9; result.m256i_i16[10] = e10; result.m256i_i16[11] = e11;
    result.m256i_i16[12] = e12; result.m256i_i16[13] = e13; result.m256i_i16[14] = e14; result.m256i_i16[15] = e15;
    return result;
}

void _mm256_storeu_ps_sim(float* mem_addr, m256i_t a) {
    memcpy(mem_addr, &a, sizeof(m256i_t));
}