#include "terminal_window.h"
#include <csignal>
#include <iostream>
#include <ncurses.h>
#include <string>

namespace
{
constexpr int CTRL_Q = 'q' & 0x1F;
constexpr int CTRL_S = 's' & 0x1F;
constexpr int CTRL_X = 'x' & 0x1F;
constexpr int CTRL_A = 'a' & 0x1F;
constexpr int CTRL_Z = 'z' & 0x1F;
}

static inline void init_ncurses()
{
    // Global ncurses initialization and setup
    initscr();
    raw();
    noecho();
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    keypad(stdscr, true);
}

static void print_help_and_exit(char* bin)
{
    std::cerr << "USAGE: " << bin << " [-f file] [-o offset]\n";
    std::exit(EXIT_SUCCESS);
}

static char* get_arg(int argc, char** argv, const std::string& arg)
{
    auto res = std::find(argv, argv + argc, arg);
    if (res != argv + argc && ++res != argv + argc)
        return *res;

    return nullptr;
}

static bool get_flag(int argc, char** argv, const std::string& flag)
{
    auto res = std::find(argv, argv + argc, flag);
    return res != (argv + argc);
}

static std::uint32_t get_starting_offset(const char* offset)
{
    if (!offset)
        return 0;

    std::string starting_offset(offset);
    if (starting_offset.size() > 2 && starting_offset[0] == '0' && (starting_offset[1] == 'x' || starting_offset[1] == 'X'))
        return std::stoll(starting_offset, nullptr, 16);

    return std::stoll(starting_offset, nullptr);
}

int main(int argc, char** argv)
{
    auto help = get_flag(argc - 1, argv + 1, "-h");

    auto input_file      = get_arg(argc - 1, argv + 1, "-f");
    auto starting_offset = get_arg(argc - 1, argv + 1, "-o");
    if (help || !input_file)
        print_help_and_exit(*argv);

    DataBuffer data;
    if (input_file && !data.open_file(input_file))
    {
        std::cerr << "Could not read from " << input_file << '\n';
        std::exit(EXIT_FAILURE);
    }

    init_ncurses();
    TerminalWindow win(stdscr, data, get_starting_offset(starting_offset));

    bool end = false;
    while (!end)
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
            win.save();
            break;
        case CTRL_Q:
            end = true;
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
        default:
            win.edit_byte(c);
            break;
        }
    }

    return 0;
}
