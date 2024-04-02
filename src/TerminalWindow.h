#ifndef TERMINAL_WINDOW_H
#define TERMINAL_WINDOW_H

#include "ByteBuffer.h"
#include "Scroller.h"
#include <cinttypes>
#include <cstdint>
#include <ncurses.h>
#include <string>

namespace Hexit
{
class TerminalWindow
{
public:
    TerminalWindow(IOHandler& handler, const std::string& file_type, std::uintmax_t go_to_byte = 0);

    TerminalWindow(const TerminalWindow&) = delete;

    TerminalWindow& operator=(const TerminalWindow&) = delete;

    ~TerminalWindow();

    void run();

private:
    void draw_line(std::uint32_t line);

    bool update_screen();

    void resize();

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

    enum class Mode : std::uint8_t
    {
        HEX,
        ASCII,
    };

    enum class Prompt : std::uint8_t
    {
        NONE,
        SAVE,
        QUIT,
        GO_TO_BYTE
    };

    Scroller          m_scroller;
    ByteBuffer        m_data;
    const std::string m_name;
    const std::string m_type;
    std::string       m_input_buffer;
    std::uintmax_t    m_byte;
    char              m_offset_format[16];
    Mode              m_mode;
    Prompt            m_prompt;
    std::uint8_t      m_nibble;
    bool              m_update;
    bool              m_quit;
};
} // namespace Hexit
#endif // TERMINAL_WINDOW_H
