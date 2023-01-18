#include "Utilities.h"
#include <gtest/gtest.h>

TEST(UtilitiesTest, HexCharToInt)
{
    // Invalid values shall return the input character to the caller unmodified.
    // Valid values shall return the integer value that corresponds to the hex digit.
    for (std::uint16_t i = 0u; i <= 0xFF; ++i)
    {
        std::uint8_t chr = static_cast<std::uint8_t>(i);
        if (chr >= '0' && chr <= '9')
            EXPECT_EQ(hex_char_to_int(chr), chr - '0');
        else if (chr == 'A' || chr == 'a')
            EXPECT_EQ(hex_char_to_int(chr), 10u);
        else if (chr == 'B' || chr == 'b')
            EXPECT_EQ(hex_char_to_int(chr), 11u);
        else if (chr == 'C' || chr == 'c')
            EXPECT_EQ(hex_char_to_int(chr), 12u);
        else if (chr == 'D' || chr == 'd')
            EXPECT_EQ(hex_char_to_int(chr), 13u);
        else if (chr == 'E' || chr == 'e')
            EXPECT_EQ(hex_char_to_int(chr), 14u);
        else if (chr == 'F' || chr == 'f')
            EXPECT_EQ(hex_char_to_int(chr), 15u);
        else
            EXPECT_EQ(hex_char_to_int(chr), chr);
    }
}

TEST(UtilitiesTest, UpdateNibble)
{
    // Nibble ids other than 0 and 1 shall allways return the input byte unchanged.
    EXPECT_EQ(update_nibble(2u, 0xF, 0x0E), 0x0E);
    EXPECT_EQ(update_nibble(0xFF, 0xF, 0x0E), 0x0E);
    for (std::uint16_t i = 0u; i <= 0xFF; ++i)
    {
        std::uint8_t chr = static_cast<std::uint8_t>(i);
        if (std::isxdigit(chr))
        {
            std::uint8_t new_value = update_nibble(0x0, chr, 0xBE);
            EXPECT_EQ(new_value >> 4, hex_char_to_int(chr));
            EXPECT_EQ(new_value & 0x0F, 0xE);

            new_value = update_nibble(0x1, chr, 0xEF);
            EXPECT_EQ(new_value >> 4, 0xE);
            EXPECT_EQ(new_value & 0x0F, hex_char_to_int(chr));
        }
        else if (chr > 0xF)
        {
            // Invalid nibble values shall always return the input byte unchanged.
            EXPECT_EQ(update_nibble(0x0, chr, 0xBE), 0xBE);
            EXPECT_EQ(update_nibble(0x1, chr, 0xA0), 0xA0);
        }
    }
}
