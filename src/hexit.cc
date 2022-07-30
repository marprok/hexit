#include "DataBuffer.h"
#include "TerminalWindow.h"
#include <csignal>
#include <iostream>
#include <ncurses.h>
#include <string>

namespace
{
constexpr int CTRL_Q = 'q' & 0x1F; // Quit
constexpr int CTRL_S = 's' & 0x1F; // Save
constexpr int CTRL_X = 'x' & 0x1F; // HEX mode
constexpr int CTRL_A = 'a' & 0x1F; // ASCII mode
constexpr int CTRL_Z = 'z' & 0x1F; // Suspend
constexpr int CTRL_G = 'g' & 0x1F; // Go to byte

inline void print_help(const char* bin)
{
    std::cerr << "USAGE: " << bin << " -f FILE [OPTIONS]\n";
    std::cerr << "\nOPTIONS:\n";
    std::cerr << "-o OFFSET: Hexadecimal or decimal byte offset to seek during startup\n";
}

inline void init_ncurses()
{
    // Global ncurses initialization and setup
    initscr();
    raw();
    noecho();
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    keypad(stdscr, true);
}

char* get_arg(int argc, char** argv, const std::string& arg)
{
    auto res = std::find(argv, argv + argc, arg);
    if (res != argv + argc && ++res != argv + argc)
        return *res;

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
}

int main(int argc, char** argv)
{
    auto help            = get_flag(argc - 1, argv + 1, "-h");
    auto input_file      = get_arg(argc - 1, argv + 1, "-f");
    auto starting_offset = get_arg(argc - 1, argv + 1, "-o");
    if (help || !input_file)
    {
        print_help(*argv);
        std::exit(EXIT_FAILURE);
    }

    DataBuffer data;
    if (input_file && !data.open_file(input_file))
    {
        std::cerr << "Could not read from " << input_file << '\n';
        std::exit(EXIT_FAILURE);
    }

    init_ncurses();
    TerminalWindow win(stdscr, data, get_starting_offset(starting_offset));

    while (!win.quit())
    {
        win.update_screen();
        win.reset_cursor();
        win.refresh();
        auto c = win.get_char();
        switch (c)
        {
        case KEY_UP:
            win.move_up();
            break;
        case KEY_DOWN:
            win.move_down();
            break;
        case KEY_RIGHT:
            win.move_right();
            break;
        case KEY_LEFT:
            win.move_left();
            break;
        case KEY_PPAGE:
            win.page_up();
            break;
        case KEY_NPAGE:
            win.page_down();
            break;
        case KEY_RESIZE:
            win.resize();
            break;
        case CTRL_S:
            win.prompt_save();
            break;
        case CTRL_Q:
            win.prompt_quit();
            break;
        case CTRL_X:
            win.toggle_hex_mode();
            break;
        case CTRL_A:
            win.toggle_ascii_mode();
            break;
        case CTRL_Z:
            endwin();
            raise(SIGSTOP);
            break;
        case CTRL_G:
            win.prompt_go_to_byte();
            break;
        default:
            win.consume_input(c);
            break;
        }
    }

    return 0;
}
