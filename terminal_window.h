#ifndef TERMINAL_WINDOW_H
#define TERMINAL_WINDOW_H

#include "data_buffer.hpp"
#include <ncurses.h>

class TerminalWindow
{
private:
    enum class Mode
    {
        HEX,
        ASCII,
    };

    struct Scroller
    {
        std::uint32_t m_first_line  = { 0 };
        std::uint32_t m_last_line   = { 0 };
        std::uint32_t m_total_lines = { 0 };
    };

    DataBuffer<1024> m_data;
    std::uint32_t    m_cy, m_cx;
    std::uint32_t    m_visible_lines, m_cols;
    bool             m_update;
    Mode             m_mode;
    WINDOW*          m_screen;
    std::uint32_t    m_current_byte, m_current_byte_offset;
    Scroller         m_scroller;
    char             m_left_padding_format[sizeof("%%0%dX  ")];

public:
    TerminalWindow(WINDOW* win, DataBuffer<1024>& data, std::uint32_t starting_byte_offset = 0);

    ~TerminalWindow();

    void draw_line(std::uint32_t line);

    void update_screen();

    void erase() const;

    void refresh() const;

    void resize();

    void reset_cursor() const;

    int get_char() const;

    void move_up();

    void page_up();

    void move_down();

    void page_down();

    void move_left();

    void move_right();

    void edit_byte(int c);

    void save();

    void toggle_ascii_mode();

    void toggle_hex_mode();
};
#endif // TERMINAL_WINDOW_H
