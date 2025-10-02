#include "formatting_utils.h"
#include <sstream>
#include <iomanip>
#include <cstdint>

// Helper to format an array of integers into a string.
template<typename T>
std::string format_integer_array(const m256i_t& value, DisplayBase display_base) {
    std::stringstream ss;
    const T* ptr = reinterpret_cast<const T*>(&value);
    size_t count = 256 / (sizeof(T) * 8);

    switch (display_base) {
        case DisplayBase::DEC:
            ss << std::dec;
            break;
        case DisplayBase::HEX:
            ss << std::hex;
            break;
        case DisplayBase::OCT:
            ss << std::oct;
            break;
    }

    for (size_t i = 0; i < count; ++i) {
        if (display_base == DisplayBase::HEX) ss << "0x";
        if (display_base == DisplayBase::OCT) ss << "0";

        // Need to cast to a wider type for printing, as int8_t and uint8_t are treated as chars.
        if constexpr (sizeof(T) == 1) {
            if (display_base == DisplayBase::DEC) {
                ss << static_cast<int>(ptr[i]);
            } else {
                ss << static_cast<unsigned int>(ptr[i]);
            }
        } else {
            ss << ptr[i];
        }
        if (i < count - 1) {
            ss << " ";
        }
    }
    return ss.str();
}


std::string format_ymm_register(
    m256i_t value,
    YmmViewMode view_mode,
    DisplayBase display_base
) {
    std::stringstream ss;

    switch (view_mode) {
        case YmmViewMode::HEX_256: {
            ss << "0x" << std::hex << std::setfill('0');
            const uint64_t* v = reinterpret_cast<const uint64_t*>(&value);
            ss << std::setw(16) << v[3]
               << std::setw(16) << v[2]
               << std::setw(16) << v[1]
               << std::setw(16) << v[0];
            return ss.str();
        }
        case YmmViewMode::INTS_32x8:
            return format_integer_array<uint8_t>(value, display_base);
        case YmmViewMode::INTS_16x16:
            return format_integer_array<uint16_t>(value, display_base);
        case YmmViewMode::INTS_8x32:
            return format_integer_array<uint32_t>(value, display_base);
        case YmmViewMode::INTS_4x64:
            return format_integer_array<uint64_t>(value, display_base);
    }

    return "Invalid YMM view mode";
}
