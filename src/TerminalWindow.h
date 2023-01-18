#ifndef TERMINAL_WINDOW_H
#define TERMINAL_WINDOW_H

#include "Scroller.h"
#include <cstdint>
#include <ncurses.h>
#include <string>

class DataBuffer;

class TerminalWindow
{
public:
    TerminalWindow(WINDOW* win, DataBuffer& data, const std::string& file_type, std::uint32_t go_to_byte = 0);

    ~TerminalWindow();

    void run();

private:
    void draw_line(std::uint32_t line);

    void update_screen();

    void resize();

    void reset_cursor() const;

    void move_up();

    void page_up();

    void move_down();

    void page_down();

    void move_left();

    void move_right();

    void consume_input(int key);

    void save();

    void prompt_save();

    void prompt_quit();

    void prompt_go_to_byte();

    void toggle_ascii_mode();

    void toggle_hex_mode();

    void edit_byte(std::uint8_t chr);

    void handle_prompt(int key);

    enum class Mode
    {
        HEX,
        ASCII,
    };

    enum class Prompt
    {
        NONE,
        SAVE,
        QUIT,
        GO_TO_BYTE
    };

    Scroller          m_scroller;
    DataBuffer&       m_data;
    const std::string m_type;
    bool              m_update;
    Mode              m_mode;
    Prompt            m_prompt;
    WINDOW*           m_screen;
    std::uint32_t     m_byte, m_nibble;
    char              m_line_offset_format[sizeof("%%0%dX") + 2];
    bool              m_quit;
    std::string       m_input_buffer;
};
#endif // TERMINAL_WINDOW_H
