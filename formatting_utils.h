#ifndef FORMATTING_UTILS_H
#define FORMATTING_UTILS_H

#include <string>
#include "avx_core.h"

// Defines the different ways we can view a YMM register's data.
enum class YmmViewMode {
    HEX_256,   // Default: one 256-bit hex number
    INTS_8x32, // 8 x 32-bit integers
    INTS_4x64, // 4 x 64-bit integers
    INTS_16x16,// 16 x 16-bit integers
    INTS_32x8  // 32 x 8-bit integers
};

// Defines the display base for integer views.
enum class DisplayBase {
    DEC, // Decimal
    HEX, // Hexadecimal
    OCT  // Octal
};

// Formats the content of a YMM register into a string based on the desired view and base.
std::string format_ymm_register(
    m256i_t value,
    YmmViewMode view_mode,
    DisplayBase display_base
);

#endif // FORMATTING_UTILS_H
