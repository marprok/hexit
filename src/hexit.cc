#include "DataBuffer.h"
#include "FileHandler.h"
#include "SignatureReader.h"
#include "StdInHandler.h"
#include "TerminalWindow.h"
#include <algorithm>
#include <csignal>
#include <iostream>
#include <ncurses.h>
#include <string>

namespace
{
inline void print_help(const char* bin)
{
    std::cerr << "USAGE: " << bin << " -f (--file) FILE [OPTIONS]\n";
    std::cerr << "\nOPTIONS:\n";
    std::cerr << "-o (--offset) OFFSET: Hexadecimal or decimal byte offset to seek during startup\n";
}

inline void init_ncurses()
{
    // Global ncurses initialization and setup
    initscr();
    raw();
    noecho();
    curs_set(0); // invisible cursor
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    keypad(stdscr, true);
}

char* get_arg(int argc, char** argv, const std::string& arg, const std::string& alt_arg = "")
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

bool get_flag(int argc, char** argv, const std::string& flag)
{
    auto res = std::find(argv, argv + argc, flag);
    return res != (argv + argc);
}

std::uint32_t get_starting_offset(const char* offset)
{
    if (!offset)
        return 0;

    std::string starting_offset(offset);
    if (starting_offset.size() > 2
        && starting_offset[0] == '0'
        && (starting_offset[1] == 'x' || starting_offset[1] == 'X'))
        return std::stoll(starting_offset, nullptr, 16);

    return std::stoll(starting_offset, nullptr);
}

std::string get_type(DataBuffer& dataBuffer)
{
    if (dataBuffer.size() == 0)
        return {};

    std::vector<std::uint8_t> query;
    SignatureReader           reader;
    std::size_t               bytes_to_copy = std::min(dataBuffer.size(), 32u);
    query.reserve(bytes_to_copy);

    for (std::size_t i = 0u; i < bytes_to_copy; ++i)
        query.push_back(dataBuffer[i]);

    return reader.get_type(query);
}

void start_hexit(IOHandler&  handler,
                 const char* starting_offset,
                 const char* input_path,
                 bool        is_read_only = false)
{
    DataBuffer data(handler);
    if (input_path && !data.open(input_path, is_read_only))
    {
        std::cerr << "Could not open " << input_path << '\n';
        std::exit(EXIT_FAILURE);
    }

    init_ncurses();
    TerminalWindow win(stdscr, data, get_type(data), get_starting_offset(starting_offset));
    win.run();
}
}

int main(int argc, char** argv)
{
    auto help            = get_flag(argc - 1, argv + 1, "-h") || get_flag(argc - 1, argv + 1, "--help");
    auto input_file      = get_arg(argc - 1, argv + 1, "-f", "--file");
    auto starting_offset = get_arg(argc - 1, argv + 1, "-o", "--offset");

    if (help || (!input_file && !starting_offset && argc > 1))
    {
        print_help(*argv);
        std::exit(EXIT_FAILURE);
    }

    if (!input_file)
    {
        StdInHandler handler;
        start_hexit(handler, starting_offset, "stdin", true);
    }
    else
    {
        FileHandler handler;
        start_hexit(handler, starting_offset, input_file);
    }

    return 0;
}
