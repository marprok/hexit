#include "Utilities.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

bool validate_args(std::size_t argc, const char* const* const argv)
{
    std::size_t i = 0u;
    for (; i < argc && argv[i];)
    {
        std::string sarg(argv[i]);
        if (sarg == "--help" || sarg == "-h")
            ++i;
        else if (sarg == "--offset" || sarg == "-o")
        {
            if (!argv[i + 1])
                break;
            ++i;
            if (!is_hex_string(argv[i]) && !is_dec_string(argv[i]))
            {
                std::cerr << "Invalid starting offset " << argv[i] << "\n";
                break;
            }
            ++i;
        }
        else if (sarg == "-f" || sarg == "--file")
        {
            if (!argv[i + 1])
                break;
            ++i;
            if (!fs::exists(argv[i]))
            {
                std::cerr << "File " << argv[i] << " does not exist!\n";
                break;
            }
            ++i;
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

    if (is_hex_string(str))
        return std::stoll(str, nullptr, 16);
    else if (is_dec_string(str))
        return std::stoll(str, nullptr);

    return 0u;
}
