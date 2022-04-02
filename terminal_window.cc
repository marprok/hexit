#include "terminal_window.h"
#include <cstdint>

namespace
{
constexpr std::uint32_t LEFT_PADDING_CHARS = sizeof(std::uint32_t) * 2;
constexpr std::uint32_t BYTES_PER_LINE     = 16;
constexpr std::uint32_t FIRST_HEX          = LEFT_PADDING_CHARS + 2 + 1;
constexpr std::uint32_t FIRST_ASCII        = FIRST_HEX + BYTES_PER_LINE * 3 + 1;
constexpr std::uint32_t CAPACITY           = 1024;
};

TerminalWindow::TerminalWindow(WINDOW* win, DataBuffer<1024>& data, std::uint32_t starting_byte_offset)
    : m_data(std::move(data)),
      m_cy(1), m_cx(FIRST_HEX),
      m_cols(COLS - 2),
      m_update(true),
      m_mode(Mode::HEX),
      m_screen(win),
      m_current_byte(0),
      m_current_byte_offset(0)
{
    m_scroller.m_total_lines = m_data.size() / BYTES_PER_LINE;
    if (m_data.size() % BYTES_PER_LINE)
        m_scroller.m_total_lines++;
    m_visible_lines = std::min(static_cast<std::uint32_t>(LINES - 2), m_scroller.m_total_lines);

    if (starting_byte_offset < m_data.size())
    {
        const std::uint32_t starting_line = starting_byte_offset / BYTES_PER_LINE;
        if (starting_line > static_cast<std::uint32_t>(m_visible_lines) && m_scroller.m_total_lines < starting_line + m_visible_lines)
        {
            m_scroller.m_first_line = starting_line - m_visible_lines + 1;
            m_scroller.m_last_line  = starting_line + 1;
        }
        else
        {
            m_scroller.m_first_line = starting_line / m_visible_lines * m_visible_lines;
            m_scroller.m_last_line  = m_scroller.m_first_line + m_visible_lines;
        }

        m_current_byte = starting_byte_offset;
        m_cy += starting_line - m_scroller.m_first_line;
        m_cx += starting_byte_offset % BYTES_PER_LINE * 3;
    }
    else
        m_scroller.m_last_line = m_visible_lines;

    std::sprintf(m_left_padding_format, "%%0%dX  ", LEFT_PADDING_CHARS);
    // TODO<Marios>: maybe move this to the ncurses initialization?
    keypad(m_screen, true);
}

TerminalWindow::~TerminalWindow()
{
    endwin();
}

void TerminalWindow::draw_line(int line)
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
            bool is_dirty = false; //m_data.m_dirty_cache.find(byte_index) != m_data.m_dirty_cache.end();
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
        const bool is_dirty = false; //m_data.m_dirty_cache.find(byte_index) != m_data.m_dirty_cache.end();
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
        for (int line = 0; line < m_visible_lines; line++)
            draw_line(line);

        const std::string filename = m_data.name().filename();
        if (true /*m_data.m_dirty_cache.empty()*/)
            mvwprintw(m_screen, 0, (COLS - filename.size()) / 2 - 1, "%s", filename.c_str());
        else
            mvwprintw(m_screen, 0, (COLS - filename.size()) / 2 - 1, "*%s", filename.c_str());

        const char mode       = m_mode == Mode::ASCII ? 'A' : 'X';
        const int  percentage = static_cast<float>(m_scroller.m_last_line) / m_scroller.m_total_lines * 100;
        mvwprintw(m_screen, LINES - 1, COLS - 7, "%c/%d%%", mode, percentage);
        m_update = false;
    }
    else
    {
        if (m_cy - 2 >= 0)
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
    if (m_scroller.m_first_line >= static_cast<std::uint32_t>(m_visible_lines - 1))
    {
        m_update = true;
        m_scroller.m_last_line -= m_visible_lines - 1;
        m_scroller.m_first_line -= m_visible_lines - 1;
        m_current_byte -= (m_visible_lines - 1) * BYTES_PER_LINE;
    }
}

void TerminalWindow::move_down()
{
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
    const auto group_id = m_current_byte / BYTES_PER_LINE;
    auto       row_size = BYTES_PER_LINE;

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

void TerminalWindow::edit_byte(int c)
{
    //    if (!m_data.m_is_editable)
    //        return;

    if (m_mode == Mode::ASCII && std::isprint(c))
        ; //m_data.m_buff[m_current_byte] = c;
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

        //m_data.m_buff[m_current_byte] &= 0xF0 >> (1 - m_current_byte_offset) * 4;
        //m_data.m_buff[m_current_byte] |= hex_digit << (1 - m_current_byte_offset) * 4;
    }

    if (false /*m_data.m_dirty_cache.empty()*/)
        m_update = true;

    //m_data.m_dirty_cache.insert(m_current_byte);
}

void TerminalWindow::TerminalWindow::save()
{
    //    if (!m_data.m_is_editable)
    //        return;

    //m_data.save();
    //m_data.m_dirty_cache.clear();
    m_update = true;
}

void TerminalWindow::toggle_ascii_mode()
{
    if (m_mode == Mode::ASCII)
        return;

    m_cx                  = FIRST_ASCII + m_current_byte % BYTES_PER_LINE;
    m_mode                = Mode::ASCII;
    m_update              = true;
    m_current_byte_offset = 0;
}

void TerminalWindow::toggle_hex_mode()
{
    if (m_mode == Mode::HEX)
        return;

    m_cx     = FIRST_HEX + (m_current_byte % BYTES_PER_LINE) * 3;
    m_mode   = Mode::HEX;
    m_update = true;
}
