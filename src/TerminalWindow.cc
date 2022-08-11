#include "TerminalWindow.h"
#include "DataBuffer.h"
#include <cstdint>

namespace
{
constexpr std::uint32_t LEFT_PADDING_CHARS = sizeof(std::uint32_t) * 2;
constexpr std::uint32_t BYTES_PER_LINE     = 16;
constexpr std::uint32_t FIRST_HEX          = LEFT_PADDING_CHARS + 2 + 1;
constexpr std::uint32_t FIRST_ASCII        = FIRST_HEX + BYTES_PER_LINE * 3 + 1;
constexpr std::uint32_t CAPACITY           = 1024;
}

TerminalWindow::TerminalWindow(WINDOW* win, DataBuffer& data, std::uint32_t start_from_byte)
    : m_data(data)
    , m_cy(1)
    , m_cx(FIRST_HEX)
    , m_cols(COLS - 2)
    , m_update(true)
    , m_mode(Mode::HEX)
    , m_prompt(Prompt::NONE)
    , m_screen(win)
    , m_current_byte(0)
    , m_current_byte_offset(0)
    , m_quit(false)
{
    m_scroller.m_total_lines = m_data.size() / BYTES_PER_LINE;
    if (m_data.size() % BYTES_PER_LINE)
        m_scroller.m_total_lines++;
    m_visible_lines = std::min(static_cast<std::uint32_t>(LINES - 2), m_scroller.m_total_lines);

    if (start_from_byte < m_data.size())
    {
        const std::uint32_t starting_line = start_from_byte / BYTES_PER_LINE;
        if (starting_line > m_visible_lines
            && m_scroller.m_total_lines < starting_line + m_visible_lines)
        {
            m_scroller.m_first_line = starting_line - m_visible_lines + 1;
            m_scroller.m_last_line  = starting_line + 1;
        }
        else
        {
            m_scroller.m_first_line = starting_line / m_visible_lines * m_visible_lines;
            m_scroller.m_last_line  = m_scroller.m_first_line + m_visible_lines;
        }

        m_current_byte = start_from_byte;
        m_cy += starting_line - m_scroller.m_first_line;
        m_cx += start_from_byte % BYTES_PER_LINE * 3;

        m_data.load_chunk(start_from_byte / DataBuffer::capacity);
    }
    else
    {
        m_data.load_chunk(0);
        m_scroller.m_last_line = m_visible_lines;
    }

    std::sprintf(m_left_padding_format, "%%0%dX  ", LEFT_PADDING_CHARS);
    m_input_buffer.reserve(LEFT_PADDING_CHARS);
}

TerminalWindow::~TerminalWindow()
{
    endwin();
}

void TerminalWindow::draw_line(std::uint32_t line)
{
    int           col           = FIRST_HEX;
    std::uint32_t group         = m_scroller.m_first_line + line;
    std::uint32_t byte_index    = group * BYTES_PER_LINE;
    std::uint32_t bytes_to_draw = BYTES_PER_LINE;

    if (m_scroller.m_total_lines - 1 == group && (m_data.size() % BYTES_PER_LINE))
        bytes_to_draw = m_data.size() % BYTES_PER_LINE;

    mvwprintw(m_screen, line + 1, 1, m_left_padding_format, byte_index);
    for (std::uint32_t i = 0; i < BYTES_PER_LINE; ++i, col += 3, byte_index++)
    {
        if (i < bytes_to_draw)
        {
            bool is_dirty = m_data.is_dirty(byte_index);
            if (m_mode == Mode::ASCII && byte_index == m_current_byte)
                wattron(m_screen, A_REVERSE);
            else if (m_mode == Mode::ASCII && is_dirty)
                wattron(m_screen, COLOR_PAIR(1) | A_REVERSE);

            mvwprintw(m_screen, line + 1, col, "%02X", m_data[byte_index]);
            if (m_mode == Mode::ASCII && byte_index == m_current_byte)
                wattroff(m_screen, A_REVERSE);
            else if (m_mode == Mode::ASCII && is_dirty)
                wattroff(m_screen, COLOR_PAIR(1) | A_REVERSE);
        }
        else
            mvwprintw(m_screen, line + 1, col, "  ");

        mvwprintw(m_screen, line + 1, col + 2, " ");
    }

    mvwprintw(m_screen, line + 1, col++, " ");
    byte_index = group * BYTES_PER_LINE;

    for (std::uint32_t i = 0; i < bytes_to_draw; ++i, col++, byte_index++)
    {
        const char c        = m_data[byte_index];
        const bool is_dirty = m_data.is_dirty(byte_index);
        if (m_mode == Mode::HEX && byte_index == m_current_byte)
            wattron(m_screen, A_REVERSE);
        else if (m_mode == Mode::HEX && is_dirty)
            wattron(m_screen, COLOR_PAIR(1) | A_REVERSE);

        mvwprintw(m_screen, line + 1, col, "%c", std::isprint(c) ? c : '.');
        if (m_mode == Mode::HEX && byte_index == m_current_byte)
            wattroff(m_screen, A_REVERSE);
        else if (m_mode == Mode::HEX && is_dirty)
            wattroff(m_screen, COLOR_PAIR(1) | A_REVERSE);
    }
}

void TerminalWindow::update_screen()
{
    if (m_update)
    {
        erase();
        for (std::uint32_t line = 0; line < m_visible_lines; line++)
            draw_line(line);

        const std::string filename = m_data.name().filename();
        if (m_data.has_dirty())
            mvwprintw(m_screen, 0, (COLS - filename.size()) / 2 - 1, "*%s", filename.c_str());
        else
            mvwprintw(m_screen, 0, (COLS - filename.size()) / 2 - 1, "%s", filename.c_str());

        const char          mode       = m_mode == Mode::ASCII ? 'A' : 'X';
        const std::uint32_t percentage = static_cast<float>(m_scroller.m_last_line) / m_scroller.m_total_lines * 100;
        mvwprintw(m_screen, LINES - 1, COLS - 7, "%c/%d%%", mode, percentage);

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
        if (m_cy >= 2)
            draw_line(m_cy - 2);

        draw_line(m_cy - 1);

        if (m_cy < m_visible_lines)
            draw_line(m_cy);
    }
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
    m_cy            = 1;
    m_cx            = m_mode == Mode::HEX ? FIRST_HEX : FIRST_ASCII;
    m_cols          = COLS - 2;
    m_visible_lines = std::min(static_cast<std::uint32_t>(LINES - 2), m_scroller.m_total_lines);

    const std::uint32_t current_line = m_current_byte / BYTES_PER_LINE;
    if (m_scroller.m_total_lines < current_line + m_visible_lines)
    {
        m_scroller.m_first_line = current_line - m_visible_lines + 1;
        m_scroller.m_last_line  = current_line + 1;
    }
    else
    {
        m_scroller.m_first_line = current_line / m_visible_lines * m_visible_lines;
        m_scroller.m_last_line  = m_scroller.m_first_line + m_visible_lines;
    }

    m_cy += current_line - m_scroller.m_first_line;
    m_cx += m_current_byte % BYTES_PER_LINE * (m_mode == Mode::HEX ? 3 : 1);
    m_current_byte_offset = 0;

    m_update = true;
}

void TerminalWindow::reset_cursor() const
{
    wmove(m_screen, m_cy, m_cx);
}

int TerminalWindow::get_char() const
{
    return wgetch(m_screen);
}

void TerminalWindow::move_up()
{
    if (m_prompt != Prompt::NONE)
        return;

    if (m_cy - 1 > 0)
    {
        m_cy--;
        m_current_byte -= BYTES_PER_LINE;
    }
    else if (m_scroller.m_first_line > 0)
    {
        m_update = true;
        m_scroller.m_last_line--;
        m_scroller.m_first_line--;
        m_current_byte -= BYTES_PER_LINE;
    }
}

void TerminalWindow::page_up()
{
    if (m_prompt != Prompt::NONE)
        return;

    if (m_scroller.m_first_line >= m_visible_lines - 1)
    {
        m_update = true;
        m_scroller.m_last_line -= m_visible_lines - 1;
        m_scroller.m_first_line -= m_visible_lines - 1;
        m_current_byte -= (m_visible_lines - 1) * BYTES_PER_LINE;
    }
}

void TerminalWindow::move_down()
{
    if (m_prompt != Prompt::NONE)
        return;

    if (m_cy - 1 < m_visible_lines - 1)
    {
        m_cy++;
        m_current_byte += BYTES_PER_LINE;
    }
    else if (m_scroller.m_last_line < m_scroller.m_total_lines)
    {
        m_update = true;
        m_scroller.m_last_line++;
        m_scroller.m_first_line++;
        m_current_byte += BYTES_PER_LINE;
    }

    if (m_current_byte >= m_data.size())
    {
        m_cx                  = (m_mode == Mode::HEX) ? FIRST_HEX : FIRST_ASCII;
        m_update              = true;
        m_current_byte        = (m_scroller.m_total_lines - 1) * BYTES_PER_LINE;
        m_current_byte_offset = 0;
    }
}

void TerminalWindow::page_down()
{
    if (m_prompt != Prompt::NONE)
        return;

    if (m_scroller.m_last_line + m_visible_lines - 1 < m_scroller.m_total_lines)
    {
        m_update = true;
        m_scroller.m_last_line += m_visible_lines - 1;
        m_scroller.m_first_line += m_visible_lines - 1;
        m_current_byte += (m_visible_lines - 1) * BYTES_PER_LINE;
    }

    if (m_current_byte >= m_data.size())
    {
        m_cx                  = (m_mode == Mode::HEX) ? FIRST_HEX : FIRST_ASCII;
        m_update              = true;
        m_current_byte        = (m_scroller.m_total_lines - 1) * BYTES_PER_LINE;
        m_current_byte_offset = 0;
    }
}

void TerminalWindow::move_left()
{
    if (m_prompt != Prompt::NONE)
        return;

    if (m_mode == Mode::HEX)
    {
        if (m_current_byte_offset == 0)
        {
            if (m_cx < m_cols && m_current_byte % BYTES_PER_LINE > 0)
            {
                m_cx -= 2;
                m_current_byte--;
                m_current_byte_offset = 1;
            }
        }
        else
        {
            m_cx--;
            m_current_byte_offset--;
        }
    }
    else
    {
        if (m_cx < m_cols && m_current_byte % BYTES_PER_LINE > 0)
        {
            m_cx--;
            m_current_byte--;
        }
    }
}

void TerminalWindow::move_right()
{
    if (m_prompt != Prompt::NONE)
        return;

    const std::uint32_t group_id = m_current_byte / BYTES_PER_LINE;
    std::uint32_t       row_size = BYTES_PER_LINE;

    if (group_id == m_scroller.m_total_lines - 1 && (m_data.size() % BYTES_PER_LINE))
        row_size = m_data.size() % BYTES_PER_LINE;

    if (m_mode == Mode::HEX)
    {
        if (m_current_byte_offset > 0)
        {
            if (m_cx < m_cols && m_current_byte % BYTES_PER_LINE < row_size - 1)
            {
                m_cx += 2;
                m_current_byte++;
                m_current_byte_offset = 0;
            }
        }
        else if (m_cx < m_cols)
        {
            m_cx++;
            m_current_byte_offset++;
        }
    }
    else
    {
        if (m_cx < m_cols && m_current_byte % BYTES_PER_LINE < row_size - 1)
        {
            m_cx++;
            m_current_byte++;
        }
    }
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
    if (!m_data.has_dirty())
        return;

    m_data.save();
    m_update = true;
}

void TerminalWindow::prompt_save()
{
    if (!m_data.has_dirty() || m_prompt != Prompt::NONE)
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
    else if (!m_data.has_dirty())
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

    m_cx                  = FIRST_ASCII + m_current_byte % BYTES_PER_LINE;
    m_mode                = Mode::ASCII;
    m_update              = true;
    m_current_byte_offset = 0;
}

void TerminalWindow::toggle_hex_mode()
{
    if (m_mode == Mode::HEX || m_prompt != Prompt::NONE)
        return;

    m_cx     = FIRST_HEX + (m_current_byte % BYTES_PER_LINE) * 3;
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
        else if (c >= 'A' && c - 'A' <= 5)
            hex_digit = 10 + c - 'A';
        else if (c >= 'a' && c - 'a' <= 5)
            hex_digit = 10 + c - 'a';
        else
            return;

        new_byte = m_data[m_current_byte];
        new_byte &= 0xF0 >> (1 - m_current_byte_offset) * 4;
        new_byte |= hex_digit << (1 - m_current_byte_offset) * 4;
    }

    if (!m_data.has_dirty())
        m_update = true;

    m_data.set_byte(m_current_byte, new_byte);
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
                    m_current_byte = go_to_byte;
                else
                    m_current_byte = m_data.size() - 1;
                m_input_buffer.clear();
            }
            m_prompt = Prompt::NONE;
            resize();
        }
        if (c == KEY_BACKSPACE && m_input_buffer.size() > 0)
        {
            m_input_buffer.pop_back();
            m_update = true;
        }
        else if (std::isprint(c)
                 && m_input_buffer.size() < LEFT_PADDING_CHARS)
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
