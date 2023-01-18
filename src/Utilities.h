#ifndef UTILITIES_H
#define UTILITIES_H

#include <cctype>
#include <cstdint>

// Converts a hex character to the corresponding integer value.
inline std::uint8_t hex_char_to_int(std::uint8_t chr)
{
    if (!std::isxdigit(chr))
        return chr;

    std::uint8_t hex_byte = 0;
    if (chr - '0' <= 9)
        hex_byte = chr - '0';
    else if (chr >= 'A' && (chr - 'A') <= 5)
        hex_byte = 10 + chr - 'A';
    else if (chr >= 'a' && (chr - 'a') <= 5)
        hex_byte = 10 + chr - 'a';

    return hex_byte;
}

// Updates the nibble of a give byte.
inline std::uint8_t update_nibble(std::uint8_t nibble_id, std::uint8_t nibble_value, std::uint8_t data)
{
    if (nibble_id > 1)
        return data;

    std::uint8_t hex_digit = hex_char_to_int(nibble_value);
    if (hex_digit > 0xF)
        return data;

    data &= 0xF0 >> ((1 - nibble_id) * 4);
    data |= hex_digit << ((1 - nibble_id) * 4);

    return data;
}
#endif
