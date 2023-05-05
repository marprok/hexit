#include "Utilities.h"
#include <filesystem>
#include <iostream>

namespace Hexit
{
namespace fs = std::filesystem;

bool validate_args(std::size_t argc, const char* const* const argv)
{
    std::size_t i      = 0u;
    bool        help   = false;
    bool        file   = false;
    bool        offset = false;
    for (; i < argc && argv[i];)
    {
        std::string sarg(argv[i]);
        if (sarg == "--help" || sarg == "-h")
        {
            if (help)
                break;
            ++i;
            help = true;
        }
        else if (sarg == "--offset" || sarg == "-o")
        {
            if (offset || ((i + 1) >= argc) || !argv[i + 1])
                break;
            ++i;
            if (!is_hex_string(argv[i]) && !is_dec_string(argv[i]))
                break;
            ++i;
            offset = true;
        }
        else if (sarg == "--file" || sarg == "-f")
        {
            if (file || ((i + 1) >= argc) || !argv[i + 1])
                break;
            ++i;
            if (!fs::exists(argv[i]))
                break;
            ++i;
            file = true;
        }
        else
            break;
    }

    return i == argc;
}

const char* get_arg(int argc, const char* const* const argv, const std::string& arg, const std::string& alt_arg)
{
    auto res_arg = std::find(argv, argv + argc, arg);
    if (res_arg != argv + argc && ++res_arg != argv + argc)
        return *res_arg;
    if (!alt_arg.empty())
    {
        auto res_alt = std::find(argv, argv + argc, alt_arg);
        if (res_alt != argv + argc && ++res_alt != argv + argc)
            return *res_alt;
    }

    return nullptr;
}

bool get_flag(int argc, const char* const* const argv, const std::string& flag)
{
    auto res = std::find(argv, argv + argc, flag);
    return res != (argv + argc);
}

std::uint32_t str_to_int(const char* const str)
{
    if (!str)
        return 0;

    if (is_dec_string(str))
        return std::stoll(str, nullptr);
    else if (is_hex_string(str))
        return std::stoll(str, nullptr, 16);

    return 0u;
}
} // namespace Hexit
