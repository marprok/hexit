#include "Utilities.h"
#include <filesystem>
#include <string_view>

namespace Hexit
{
namespace fs = std::filesystem;

bool validate_args(std::uintmax_t argc, const char* const* const argv)
{
    std::uintmax_t i      = 0u;
    bool           help   = false;
    bool           file   = false;
    bool           offset = false;
    for (; i < argc && argv[i];)
    {
        std::string_view sarg(argv[i]);
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
            if (++i; !is_hex_string(argv[i]) && !is_dec_string(argv[i]))
                break;
            ++i;
            offset = true;
        }
        else if (sarg == "--file" || sarg == "-f")
        {
            if (file || ((i + 1) >= argc) || !argv[i + 1])
                break;
            if (++i; !fs::exists(argv[i]))
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

std::uintmax_t str_to_int(const char* const str)
{
    if (!str)
        return 0;

    if (is_dec_string(str))
        return std::stoull(str, nullptr);
    else if (is_hex_string(str))
        return std::stoull(str, nullptr, 16);

    return 0u;
}

Prompt::Prompt(const std::uint32_t max)
    : MAX_CHARACTERS(max)
{
    input.reserve(MAX_CHARACTERS);
    needle.reserve(MAX_CHARACTERS);
}

bool Prompt::push(std::uint8_t c, bool as_hex)
{
    if ((input.size() >= MAX_CHARACTERS)
        || c < 0x20 || c > 0x7F
        || (as_hex && !std::isxdigit(c))
        || !is_text_prompt()
        || (!as_hex && type == Prompt::GO_TO_BYTE && !std::isdigit(c)))
        return false;

    input.push_back(static_cast<char>(c));
    needle.clear();
    return true;
}

const std::vector<uint8_t>& Prompt::to_bytes(bool as_hex)
{
    if (!input.empty())
    {
        if (needle.empty())
        {
            needle.reserve(input.size());
            std::size_t ci = 0;
            if (as_hex)
            {
                if (input.size() & 0x1)
                    needle.push_back(hdtoi(static_cast<uint8_t>(input[ci++])));
                for (; ci < input.size(); ci += 2)
                    needle.push_back(static_cast<uint8_t>((hdtoi(static_cast<uint8_t>(input[ci])) << 4) | hdtoi(static_cast<uint8_t>(input[ci + 1]))));
            }
            else
                std::copy(input.begin(), input.end(), std::back_inserter(needle));
        }
    }

    return needle;
}

bool Prompt::pop()
{
    if (input.empty())
        return false;

    input.pop_back();
    needle.clear();
    return true;
}

void Prompt::reset(Type t)
{
    input.clear();
    needle.clear();
    type = t;
}
} // namespace Hexit
