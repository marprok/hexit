#ifndef TERMINAL_WINDOW_H
#define TERMINAL_WINDOW_H

#include "Scroller.h"
#include <cinttypes>
#include <cstdint>
#include <ncurses.h>
#include <string>

namespace Hexit
{
class ByteBuffer;

class TerminalWindow
{
public:
    TerminalWindow(WINDOW* win, ByteBuffer& data, const std::string& file_type, std::uint64_t go_to_byte = 0);

    TerminalWindow(const TerminalWindow&) = delete;

    TerminalWindow& operator=(const TerminalWindow&) = delete;

    ~TerminalWindow();

    void run();

private:
    bool draw_line(std::uint32_t line);

    bool update_screen();

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

    inline void set_error_and_quit(const std::string& err)
    {
        m_error_msg.reserve(err.size());
        m_error_msg = err;
        m_quit      = true;
    }

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
    ByteBuffer&       m_data;
    const std::string m_type;
    bool              m_update;
    Mode              m_mode;
    Prompt            m_prompt;
    WINDOW*           m_screen;
    std::uint64_t     m_byte;
    std::uint8_t      m_nibble;
    char              m_offset_format[7];
    bool              m_quit;
    std::string       m_input_buffer;
    std::string       m_error_msg;
};
} // namespace Hexit
#endif // TERMINAL_WINDOW_H
