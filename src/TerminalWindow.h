#ifndef TERMINAL_WINDOW_H
#define TERMINAL_WINDOW_H

#include "ByteBuffer.h"
#include "Scroller.h"
#include "config.h"
#include <cstdint>
#include <functional>
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

    void prompt_search();

    void toggle_ascii_mode();

    void toggle_hex_mode();

    void edit_byte(std::uint8_t chr);

    void handle_prompt(int key);

    enum class Mode : std::uint8_t
    {
        HEX,
        ASCII,
    };

    struct Prompt
    {

        Prompt(std::function<void(int)> callback)
            : handle_key(callback)
        {
            input.reserve(LINE_OFFSET_LEN);
            needle.reserve(LINE_OFFSET_LEN);
        }
        std::string              input;
        std::vector<uint8_t>     needle;
        std::function<void(int)> handle_key;

        enum
        {
            NONE,
            SAVE,
            QUIT,
            GO_TO_BYTE,
            SEARCH,
        } type { NONE };
    };

    ByteBuffer        m_data;
    Scroller          m_scroller;
    Prompt            m_prompt;
    const std::string m_name;
    const std::string m_type;
    std::uintmax_t    m_byte;
    char              m_offset_format[16];
    Mode              m_mode;
    std::uint8_t      m_nibble;
    bool              m_update;
    bool              m_quit;
};
} // namespace Hexit
#endif // TERMINAL_WINDOW_H
