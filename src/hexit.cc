#include "ByteBuffer.h"
#include "FileHandler.h"
#include "SignatureReader.h"
#include "StdInHandler.h"
#include "TerminalWindow.h"
#include "Utilities.h"
#include <algorithm>
#include <csignal>
#include <iostream>
#include <ncurses.h>
#include <string>

using namespace Hexit;
namespace
{
inline void print_help(const char* bin)
{
    std::cerr << "USAGE: " << bin << " -f (--file) FILE [OPTIONS]\n";
    std::cerr << "\nDisplay the hex dump of a file.\n";
    std::cerr << "If no file is given via the -f flag, then hexit will read bytes from standard input\n";
    std::cerr << "until EOF is reached. When displaying the hex dump of standard input, saving will do nothing.\n";
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
    use_default_colors();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    keypad(stdscr, true);
}

std::string get_type(ByteBuffer& byteBuffer)
{
    if (byteBuffer.size() == 0)
        return {};

    std::vector<std::uint8_t> query;
    SignatureReader           reader;
    std::size_t               bytes_to_copy = std::min(byteBuffer.size(), 32u);
    query.reserve(bytes_to_copy);

    for (std::size_t i = 0u; i < bytes_to_copy; ++i)
    {
        const auto value = byteBuffer[i];
        if (!value.has_value())
        {
            std::cerr << "Error reading file signature!\n";
            std::exit(EXIT_FAILURE);
        }
        query.push_back(*value);
    }

    return reader.get_type(query);
}

void start_hexit(IOHandler&        handler,
                 const char* const starting_offset,
                 const char* const input_path,
                 bool              is_read_only = false)
{
    ChunkCache cache(handler);
    if (input_path && !cache.open(input_path, is_read_only))
    {
        std::cerr << "Could not open " << input_path << '\n';
        std::exit(EXIT_FAILURE);
    }

    ByteBuffer buffer(cache);
    init_ncurses();
    TerminalWindow win(stdscr, buffer, get_type(buffer), str_to_int(starting_offset));
    win.run();
}
}

int main(int argc, char** argv)
{
    if (!validate_args(argc - 1, argv + 1))
    {
        std::cerr << "Invalid arguments given!\n";
        print_help(*argv);
        std::exit(EXIT_FAILURE);
    }

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

    return EXIT_SUCCESS;
}
