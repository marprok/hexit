#include "ByteBuffer.h"
#include "FileHandler.h"
#include "SignatureReader.h"
#include "StdInHandler.h"
#include "TerminalWindow.h"
#include "Utilities.h"
#include <algorithm>
#include <iostream>
#include <ncurses.h>
#include <string>

using namespace Hexit;
namespace
{
inline void print_help(const char* bin)
{
    std::cerr << "\nUsage:\n";
    std::cerr << bin << " -f (--file) <file> [options]\n\n";
    std::cerr << "Display the hex dump of a file.\n\n";
    std::cerr << "If no file is given via the -f flag, then hexit will read bytes from standard input\n";
    std::cerr << "until EOF is reached. When displaying the hex dump of standard input, saving will do nothing.\n\n";
    std::cerr << "Options:\n";
    std::cerr << "-o (--offset) <offset>: Hexadecimal or decimal byte offset to seek during startup.\n";
}

inline bool init_ncurses()
{
    // Global ncurses initialization and setup
    return initscr() != nullptr
        && raw() != ERR
        && noecho() != ERR
        && curs_set(0) != ERR // invisible cursor
        && start_color() != ERR
        && use_default_colors() != ERR
        && init_pair(1, COLOR_GREEN, COLOR_BLACK) != ERR
        && keypad(stdscr, true) != ERR;
}

int get_type(IOHandler& handler, std::string& file_type)
{
    if (handler.size() == 0)
        return 1;

    std::vector<std::uint8_t> query;
    SignatureReader           reader;
    std::size_t               bytes_to_copy = std::min(handler.size(), static_cast<std::uint64_t>(32u));
    query.resize(bytes_to_copy);

    if (!handler.seek(0u) || !handler.read(query.data(), bytes_to_copy))
    {
        std::cerr << "Error reading file signature!\n";
        return 1;
    }

    file_type = reader.get_type(query);
    return 0;
}

int start_hexit(IOHandler&        handler,
                const char* const starting_offset,
                const char* const input_path,
                bool              is_read_only = false)
{
    ChunkCache cache(handler);
    if (input_path && !cache.open(input_path, is_read_only))
    {
        std::cerr << "Could not open " << input_path << '\n';
        return 1;
    }

    ByteBuffer  buffer(cache);
    std::string file_type;
    if (get_type(handler, file_type) != 0)
        return 1;

    if (!init_ncurses())
        return 1;

    TerminalWindow win(stdscr, buffer, file_type, str_to_int(starting_offset));
    win.run();
    return 0;
}
}

int main(int argc, char** argv)
{
    if (!validate_args(argc - 1, argv + 1))
    {
        std::cerr << "Invalid arguments given!\n";
        print_help(*argv);
        return 1;
    }

    auto help            = get_flag(argc - 1, argv + 1, "-h") || get_flag(argc - 1, argv + 1, "--help");
    auto input_file      = get_arg(argc - 1, argv + 1, "-f", "--file");
    auto starting_offset = get_arg(argc - 1, argv + 1, "-o", "--offset");

    if (help || (!input_file && !starting_offset && argc > 1))
    {
        print_help(*argv);
        return 1;
    }

    int ret = 0;
    if (!input_file)
    {
        StdInHandler handler;
        ret = start_hexit(handler, starting_offset, "stdin", true);
    }
    else
    {
        FileHandler handler;
        ret = start_hexit(handler, starting_offset, input_file);
    }

    return ret;
}
