#include "Utilities.h"
#include <cstring>
#include <filesystem>
#include <gtest/gtest.h>

namespace fs = std::filesystem;

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

TEST(UtilitiesTest, StringChecks)
{
    EXPECT_TRUE(is_hex_string("0123456789"));
    EXPECT_TRUE(is_hex_string("0123456789aAbBcCdDeEfF"));
    EXPECT_TRUE(is_hex_string("0x0123456789aAbBcCdDeEfF"));
    EXPECT_TRUE(is_hex_string("0X0123456789aAbBcCdDeEfF"));
    EXPECT_FALSE(is_hex_string("0xbeefqwerty"));
    EXPECT_FALSE(is_hex_string(""));

    EXPECT_TRUE(is_dec_string("0123456789"));
    EXPECT_FALSE(is_dec_string("0x0123456789"));
    EXPECT_FALSE(is_dec_string("0X0123456789"));
    EXPECT_FALSE(is_dec_string("0123456789aAbBcCdDeEfF"));
    EXPECT_FALSE(is_dec_string("0x0123456789aAbBcCdDeEfF"));
    EXPECT_FALSE(is_dec_string("0X0123456789aAbBcCdDeEfF"));
    EXPECT_FALSE(is_dec_string(""));
}

TEST(UtilitiesTest, StrToInt)
{
    EXPECT_EQ(str_to_int("1234"), 1234u);
    EXPECT_EQ(str_to_int("0x1234"), 0x1234u);
    EXPECT_EQ(str_to_int("0X1234"), 0x1234u);
    EXPECT_EQ(str_to_int("beef"), 0XBEEFu);

    EXPECT_EQ(str_to_int("this is not a number"), 0u);
    EXPECT_EQ(str_to_int(""), 0u);
}

TEST(UtilitiesTest, GetArgFlags)
{
    EXPECT_EQ(get_arg(0, nullptr, "--nothing"), nullptr);
    {
        const char* argv[] = { "--nothing", nullptr };
        EXPECT_EQ(get_arg(1, argv, "--nothing"), nullptr);
    }
    {
        const char* argv[] = { "--arg1", "arg1_value", nullptr };
        EXPECT_EQ(get_arg(1, argv, "--arg1"), nullptr);
        EXPECT_EQ(std::strcmp(get_arg(2, argv, "--arg1"), "arg1_value"), 0);
    }
    {
        const char* argv[] = { "--arg1", "arg1_value", "-a", nullptr };
        EXPECT_EQ(get_arg(1, argv, "-a"), nullptr);
        EXPECT_EQ(get_arg(3, argv, "-a"), nullptr);
    }
    {
        const char* argv[] = { "--arg1", "arg1_value", "--alt_arg", "alt_arg_value", nullptr };
        EXPECT_EQ(get_arg(4, argv, "-a"), nullptr);
        EXPECT_EQ(std::strcmp(get_arg(4, argv, "--alt_arg"), "alt_arg_value"), 0);
        EXPECT_EQ(std::strcmp(get_arg(4, argv, "-atl", "--alt_arg"), "alt_arg_value"), 0);
    }
    {
        const char* argv[] = { "--arg1", "arg1_value", "-a", "a_value", "-f", nullptr };
        EXPECT_FALSE(get_flag(3, argv, "-f"));
        EXPECT_TRUE(get_flag(5, argv, "-f"));
        EXPECT_TRUE(get_flag(5, argv, "-a"));
        EXPECT_TRUE(get_flag(5, argv, "--arg1"));
        EXPECT_FALSE(get_flag(5, argv, "-z"));
        EXPECT_FALSE(get_flag(5, argv, "-ar"));
    }
}

TEST(UtilitiesTest, ValidateArgs)
{
    const std::string current_path = fs::current_path().string();
    {
        const char* argv[] = { nullptr };
        EXPECT_TRUE(validate_args(0, argv));
    }
    {
        const char* argv[] = { "-f", nullptr };
        EXPECT_FALSE(validate_args(1, argv));
    }
    {
        const char* argv[] = { "--file", nullptr };
        EXPECT_FALSE(validate_args(1, argv));
    }
    {
        const char* argv[] = { "-f", current_path.c_str(), nullptr };
        EXPECT_TRUE(validate_args(2, argv));
    }
    {
        const char* argv[] = { "--file", current_path.c_str(), nullptr };
        EXPECT_TRUE(validate_args(2, argv));
    }
    {
        const char* argv[] = { "-f", "this file does not exist", nullptr };
        EXPECT_FALSE(validate_args(2, argv));
    }
    {
        const char* argv[] = { "--file", "this file does not exist", nullptr };
        EXPECT_FALSE(validate_args(2, argv));
    }
    {
        const char* argv[] = { current_path.c_str(), "--file", nullptr };
        EXPECT_FALSE(validate_args(2, argv));
    }
    {
        const char* argv[] = { "-o", nullptr };
        EXPECT_FALSE(validate_args(1, argv));
    }
    {
        const char* argv[] = { "--offset", nullptr };
        EXPECT_FALSE(validate_args(1, argv));
    }
    {
        const char* argv[] = { "-o", "abcdef", nullptr };
        EXPECT_TRUE(validate_args(2, argv));
    }
    {
        const char* argv[] = { "--offset", "0xabcdef", nullptr };
        EXPECT_TRUE(validate_args(2, argv));
    }
    {
        const char* argv[] = { "-o", "1234", nullptr };
        EXPECT_TRUE(validate_args(2, argv));
    }
    {
        const char* argv[] = { "--offset", "1234", nullptr };
        EXPECT_TRUE(validate_args(2, argv));
    }
    {
        const char* argv[] = { "1234", "--offset", nullptr };
        EXPECT_FALSE(validate_args(2, argv));
    }
    {
        const char* argv[] = { "-f", "this file does not exist", "-o", "1234", nullptr };
        EXPECT_FALSE(validate_args(4, argv));
    }
    {
        const char* argv[] = { "-f", current_path.c_str(), "-o", "1234", nullptr };
        EXPECT_TRUE(validate_args(4, argv));
    }
    {
        const char* argv[] = { "-o", "1234", "-f", current_path.c_str(), nullptr };
        EXPECT_TRUE(validate_args(4, argv));
    }
    {
        const char* argv[] = { "--offset", "1234", "--file", current_path.c_str(), "--help", nullptr };
        EXPECT_TRUE(validate_args(5, argv));
    }
    {
        const char* argv[] = { "--offset", "1234", "--file", current_path.c_str(), "--help", "-h", nullptr };
        EXPECT_FALSE(validate_args(6, argv));
    }
    {
        const char* argv[] = { "--offset", "abcdef", "--file", current_path.c_str(), "-f", current_path.c_str(), nullptr };
        EXPECT_FALSE(validate_args(6, argv));
    }
    {
        const char* argv[] = { "--offset", "1234", "-o", "0xabcdef", "-f", current_path.c_str(), nullptr };
        EXPECT_FALSE(validate_args(6, argv));
    }
    {
        const char* argv[] = { "0xabcdef", "-f", current_path.c_str(), nullptr };
        EXPECT_FALSE(validate_args(3, argv));
    }
    {
        const char* argv[] = { "-d", "--some-random-flag", current_path.c_str(), nullptr };
        EXPECT_FALSE(validate_args(3, argv));
    }
}
