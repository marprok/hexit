#include "TerminalWindow.h"
#include "DataBuffer.h"
#include <cstdint>
#include <cstdio>

namespace
{
constexpr std::uint32_t BYTES_PER_LINE  = 16;
constexpr std::uint32_t ASCII_PADDING   = 2;                         // The distance between hex and ASCII digits.
constexpr std::uint32_t HEX_PADDING     = 2;                         // The distatnce between the line byte offset and the ASCII digits.
constexpr std::uint32_t LINE_OFFSET_LEN = sizeof(std::uint32_t) * 2; // The number of hex digits used for the line byte offset.
constexpr std::uint32_t FIRST_HEX       = LINE_OFFSET_LEN + 1 + HEX_PADDING;
constexpr std::uint32_t FIRST_ASCII     = FIRST_HEX + BYTES_PER_LINE * 3 - 1 + ASCII_PADDING;
}

TerminalWindow::TerminalWindow(WINDOW* win, DataBuffer& data, const std::string& file_type, std::uint32_t start_from_byte)
    : m_scroller(data.size(), BYTES_PER_LINE)
    , m_data(data)
    , m_type(file_type)
    , m_update(true)
    , m_mode(Mode::HEX)
    , m_prompt(Prompt::NONE)
    , m_screen(win)
    , m_byte(0)
    , m_nibble(0)
    , m_quit(false)
{

    if (start_from_byte < m_data.size())
        m_byte = start_from_byte;
    else
        m_byte = m_data.size() - 1;

    std::sprintf(m_line_offset_format, "%%0%dX", LINE_OFFSET_LEN);
    m_input_buffer.reserve(LINE_OFFSET_LEN);
    m_data.load_chunk(m_byte / DataBuffer::capacity);
    resize();
}

TerminalWindow::~TerminalWindow()
{
    endwin();
}

void TerminalWindow::draw_line(std::uint32_t line)
{
    std::uint32_t col           = FIRST_HEX;
    std::uint32_t line_abs      = m_scroller.first() + line;
    std::uint32_t line_byte     = line_abs * BYTES_PER_LINE;
    std::uint32_t bytes_to_draw = BYTES_PER_LINE;

    if (((m_scroller.total() - 1) == line_abs) && (m_data.size() % BYTES_PER_LINE) > 0)
        bytes_to_draw = m_data.size() % BYTES_PER_LINE;
    // Skip the first box border line.
    line++;
    // Draw the line byte offset.
    mvwprintw(m_screen, line, 1, m_line_offset_format, line_byte);
    // Draw the HEX part.
    for (std::uint32_t i = 0; i < BYTES_PER_LINE; ++i, col += 3, line_byte++)
    {
        if (i < bytes_to_draw)
        {
            bool is_dirty     = m_data.is_dirty(line_byte);
            char hexDigits[3] = { 0 };
            std::sprintf(hexDigits, "%02X", m_data[line_byte]);

            if (line_byte == m_byte)
            {
                if (m_mode == Mode::HEX)
                {
                    wattron(m_screen, A_REVERSE);
                    mvwprintw(m_screen, line, col + m_nibble, "%c", hexDigits[m_nibble]);
                    wattroff(m_screen, A_REVERSE);
                    mvwprintw(m_screen, line, col + 1 - m_nibble, "%c", hexDigits[1 - m_nibble]);
                }
                else
                {
                    wattron(m_screen, A_REVERSE);
                    mvwprintw(m_screen, line, col, hexDigits);
                    wattroff(m_screen, A_REVERSE);
                }
            }
            else
            {
                if (is_dirty)
                    wattron(m_screen, COLOR_PAIR(1) | A_REVERSE);
                mvwprintw(m_screen, line, col, hexDigits);
                if (is_dirty)
                    wattroff(m_screen, COLOR_PAIR(1) | A_REVERSE);
            }
        }
    }

    line_byte = line_abs * BYTES_PER_LINE;
    col       = FIRST_ASCII;
    // Draw the ASCII part.
    for (std::uint32_t i = 0; i < bytes_to_draw; ++i, col++, line_byte++)
    {
        const char c        = m_data[line_byte];
        const bool is_dirty = m_data.is_dirty(line_byte);
        if (line_byte == m_byte)
        {
            wattron(m_screen, A_REVERSE);
            mvwprintw(m_screen, line, col, "%c", std::isprint(c) ? c : '.');
            wattroff(m_screen, A_REVERSE);
        }
        else
        {
            if (is_dirty)
                wattron(m_screen, COLOR_PAIR(1) | A_REVERSE);
            mvwprintw(m_screen, line, col, "%c", std::isprint(c) ? c : '.');
            if (is_dirty)
                wattroff(m_screen, COLOR_PAIR(1) | A_REVERSE);
        }
    }
}

void TerminalWindow::update_screen()
{
    if (m_update)
    {
        erase();
        for (std::uint32_t line = 0; line < m_scroller.visible(); line++)
            draw_line(line);

        const std::string file_name = m_data.name().filename();
        if (m_data.has_dirty())
            mvwprintw(m_screen, 0, (COLS - file_name.size()) / 2 - 1, "*%s", file_name.c_str());
        else
            mvwprintw(m_screen, 0, (COLS - file_name.size()) / 2 - 1, "%s", file_name.c_str());

        const char          mode        = m_mode == Mode::ASCII ? 'A' : 'X';
        const std::uint32_t percentage  = static_cast<float>(m_scroller.last() + 1) / m_scroller.total() * 100;
        const int           info_column = COLS - 8 - m_type.size();
        mvwprintw(m_screen, LINES - 1, info_column, "%s/%c/%d%%", m_type.data(), mode, percentage);

        if (m_prompt == Prompt::SAVE)
            mvwprintw(m_screen, LINES - 1, 1, "Modified buffer, save?(y/n)");
        else if (m_prompt == Prompt::QUIT)
            mvwprintw(m_screen, LINES - 1, 1, "Modified buffer, quit?(y,n)");
        else if (m_prompt == Prompt::GO_TO_BYTE)
            mvwprintw(m_screen, LINES - 1, 1, "Goto byte: %s", m_input_buffer.c_str());
        m_update = false;
    }
    else
    {
        // We have to update the lines above and below in case we have modified a byte contained in them.
        if (m_scroller.active() > 0)
            draw_line(m_scroller.active() - 1);
        draw_line(m_scroller.active());
        if ((m_scroller.active() + 1) < m_scroller.visible())
            draw_line(m_scroller.active() + 1);
    }

    // Draw the current byte offset.
    if (m_prompt == Prompt::NONE)
        mvwprintw(m_screen, LINES - 1, 1, m_line_offset_format, m_byte);
}

void TerminalWindow::erase() const
{
    werase(m_screen);
    box(m_screen, 0, 0);
}

void TerminalWindow::refresh() const
{
    wrefresh(m_screen);
}

void TerminalWindow::resize()
{
    // If the number of terminal lines is less than or equal to 2, we cannot display much :(
    if (LINES <= 2)
        return;

    m_scroller.adjust_lines(static_cast<std::uint32_t>(LINES - 2), m_byte);
    m_update = true;
}

void TerminalWindow::reset_cursor() const
{
    std::uint32_t col = 0u;
    if (m_mode == Mode::HEX)
        col += FIRST_HEX + (m_byte % BYTES_PER_LINE) * 3;
    else
        col += FIRST_ASCII + m_byte % BYTES_PER_LINE;

    wmove(m_screen, m_scroller.active() + 1, col);
}

int TerminalWindow::get_char() const
{
    return wgetch(m_screen);
}

void TerminalWindow::move_up()
{
    if (m_prompt != Prompt::NONE)
        return;

    m_update = m_scroller.move_up();
    if (m_byte >= BYTES_PER_LINE)
        m_byte -= BYTES_PER_LINE;
}

void TerminalWindow::page_up()
{
    if (m_prompt != Prompt::NONE)
        return;

    if (m_byte >= (BYTES_PER_LINE * m_scroller.visible()))
        m_byte -= BYTES_PER_LINE * m_scroller.visible();
    else
        m_byte = 0;

    resize();
}

void TerminalWindow::move_down()
{
    if (m_prompt != Prompt::NONE)
        return;

    const std::uint32_t distance = m_data.size() - m_byte;
    m_update                     = m_scroller.move_down();
    if (distance > BYTES_PER_LINE)
        m_byte += BYTES_PER_LINE;
    else if (distance > (m_data.size() % BYTES_PER_LINE))
    {
        m_byte   = m_data.size() - 1;
        m_nibble = 0;
    }
}

void TerminalWindow::page_down()
{
    if (m_prompt != Prompt::NONE)
        return;

    if ((m_data.size() - m_byte) > (BYTES_PER_LINE * m_scroller.visible()))
        m_byte += BYTES_PER_LINE * m_scroller.visible();
    else
        m_byte = m_data.size() - 1;

    resize();
}

void TerminalWindow::move_left()
{
    if (m_prompt != Prompt::NONE)
        return;

    if (m_mode == Mode::HEX)
    {
        if (m_nibble == 0)
        {
            if ((m_byte % BYTES_PER_LINE) > 0)
            {
                m_byte--;
                m_nibble = 1;
            }
        }
        else
            m_nibble--;
    }
    else if ((m_byte % BYTES_PER_LINE) > 0)
        m_byte--;
}

void TerminalWindow::move_right()
{
    if (m_prompt != Prompt::NONE)
        return;

    const std::uint32_t line_abs   = m_byte / BYTES_PER_LINE;
    std::uint32_t       line_bytes = BYTES_PER_LINE;

    if ((line_abs == m_scroller.total() - 1) && (m_data.size() % BYTES_PER_LINE) > 0)
        line_bytes = m_data.size() % BYTES_PER_LINE;

    if (m_mode == Mode::HEX)
    {
        if (m_nibble > 0)
        {
            if ((m_byte % BYTES_PER_LINE) < (line_bytes - 1))
            {
                m_byte++;
                m_nibble = 0;
            }
        }
        else
            m_nibble++;
    }
    else if ((m_byte % BYTES_PER_LINE) < (line_bytes - 1))
        m_byte++;
}

void TerminalWindow::consume_input(int c)
{
    if (m_prompt != Prompt::NONE)
        handle_prompt(c);
    else
        edit_byte(c);
}

void TerminalWindow::TerminalWindow::save()
{
    if (!m_data.has_dirty() || m_data.immutable())
        return;

    m_data.save();
    m_update = true;
}

void TerminalWindow::prompt_save()
{
    if (!m_data.has_dirty()
        || m_prompt != Prompt::NONE
        || m_data.immutable())
        return;

    m_prompt = Prompt::SAVE;
    m_update = true;
}

void TerminalWindow::prompt_quit()
{
    if (m_prompt != Prompt::NONE)
    {
        // Just remove any active prompt
        m_prompt = Prompt::NONE;
        m_update = true;
        m_input_buffer.clear();
    }
    else if (!m_data.has_dirty() || m_data.immutable())
        m_quit = true;
    else
    {
        m_prompt = Prompt::QUIT;
        m_update = true;
    }
}

void TerminalWindow::prompt_go_to_byte()
{
    if (m_prompt != Prompt::NONE)
        return;

    m_prompt = Prompt::GO_TO_BYTE;
    m_input_buffer.clear();
    m_update = true;
}

void TerminalWindow::toggle_ascii_mode()
{
    if (m_mode == Mode::ASCII || m_prompt != Prompt::NONE)
        return;

    m_mode   = Mode::ASCII;
    m_update = true;
    m_nibble = 0;
}

void TerminalWindow::toggle_hex_mode()
{
    if (m_mode == Mode::HEX || m_prompt != Prompt::NONE)
        return;

    m_mode   = Mode::HEX;
    m_update = true;
}

bool TerminalWindow::quit() const
{
    return m_quit;
}

void TerminalWindow::edit_byte(int c)
{
    std::uint8_t new_byte;
    if (m_mode == Mode::ASCII && std::isprint(c))
        new_byte = static_cast<std::uint8_t>(c);
    else
    {
        if (c < '0' || c > 'f')
            return;

        std::uint8_t hex_digit = 0;
        if (c - '0' <= 9)
            hex_digit = c - '0';
        else if (c >= 'A' && (c - 'A') <= 5)
            hex_digit = 10 + c - 'A';
        else if (c >= 'a' && (c - 'a') <= 5)
            hex_digit = 10 + c - 'a';
        else
            return;

        new_byte = m_data[m_byte];
        new_byte &= 0xF0 >> ((1 - m_nibble) * 4);
        new_byte |= hex_digit << ((1 - m_nibble) * 4);
    }

    if (!m_data.has_dirty())
        m_update = true;

    m_data.set_byte(m_byte, new_byte);
}

void TerminalWindow::handle_prompt(int c)
{

    if (m_prompt == Prompt::GO_TO_BYTE)
    {
        if (c == '\n')
        {
            if (m_input_buffer.size() > 0)
            {
                std::uint32_t go_to_byte = 0;
                if (m_mode == Mode::ASCII)
                    go_to_byte = std::stoll(m_input_buffer, nullptr);
                else if (m_mode == Mode::HEX)
                    go_to_byte = std::stoll(m_input_buffer, nullptr, 16);

                if (go_to_byte < m_data.size())
                    m_byte = go_to_byte;
                else
                    m_byte = m_data.size() - 1;
                m_input_buffer.clear();
            }
            m_prompt = Prompt::NONE;
            resize();
        }
        else if (c == KEY_BACKSPACE && m_input_buffer.size() > 0)
        {
            m_input_buffer.pop_back();
            m_update = true;
        }
        else if (std::isprint(c)
                 && m_input_buffer.size() < LINE_OFFSET_LEN)
        {
            if ((m_mode == Mode::ASCII && isdigit(c))
                || (m_mode == Mode::HEX && isxdigit(c)))
            {
                m_input_buffer.push_back(static_cast<char>(c));
                m_update = true;
            }
        }
    }
    else if (std::isprint(c))
    {
        char chr = static_cast<char>(c);
        switch (chr)
        {
        case 'y':
        case 'Y':
        {
            if (m_prompt == Prompt::SAVE)
            {
                save();
                m_prompt = Prompt::NONE;
            }
            else if (m_prompt == Prompt::QUIT)
                m_quit = true;
            break;
        }
        case 'n':
        case 'N':
        {
            m_prompt = Prompt::NONE;
            m_update = true;
            break;
        }
        default:
            break;
        }
    }
}
