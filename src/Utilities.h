#ifndef UTILITIES_H
#define UTILITIES_H

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>

namespace Hexit
{
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

inline bool is_hex_string(const std::string& str)
{
    bool skip = (str.compare(0, 2, "0x") == 0) || (str.compare(0, 2, "0X") == 0);
    return !str.empty() && std::all_of(str.begin() + (skip ? 2 : 0), str.end(), [](unsigned char uc)
                                       { return std::isxdigit(uc); });
}

inline bool is_dec_string(const std::string& str)
{
    return !str.empty() && std::all_of(str.begin(), str.end(), [](unsigned char uc)
                                       { return std::isdigit(uc); });
}

bool validate_args(std::size_t argc, const char* const* const argv);

const char* get_arg(int argc, const char* const* const argv, const std::string& arg, const std::string& alt_arg = "");

bool get_flag(int argc, const char* const* const argv, const std::string& flag);

std::uint32_t str_to_int(const char* const str);
} // namespace Hexit
#endif // UTILITIES_H
