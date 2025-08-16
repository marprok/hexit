#include "TerminalWindow.h"
#include "ByteBuffer.h"
#include "Utilities.h"
#include <cinttypes>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <iostream>

namespace Hexit
{
TerminalWindow::TerminalWindow(IOHandler& handler, const std::string& file_type, std::uintmax_t start_from_byte)
    : m_data(handler)
    , m_scroller(handler.size(), BYTES_PER_LINE)
    , m_prompt(std::bind(&TerminalWindow::handle_prompt, this, std::placeholders::_1))
    , m_name(handler.name().filename())
    , m_type(file_type)
    , m_byte(start_from_byte < handler.size() ? start_from_byte : handler.size() - 1)
    , m_mode(Mode::HEX)
    , m_nibble(0u)
    , m_update(true)
    , m_quit(false)
{
    std::snprintf(m_offset_format, sizeof(m_offset_format), "%%0%" PRIu32 PRIX64, LINE_OFFSET_LEN);
    resize();
}

TerminalWindow::~TerminalWindow()
{
    if (endwin(); !m_data.is_ok())
        std::cerr << m_data.error_msg() << std::endl;
}

void TerminalWindow::run()
{
    erase();
    box(stdscr, 0, 0);
    while (!m_quit && update_screen())
    {
        refresh();
        auto c = getch();
        switch (c)
        {
        case KEY_UP:
            move_up();
            break;
        case KEY_DOWN:
            move_down();
            break;
        case KEY_RIGHT:
            move_right();
            break;
        case KEY_LEFT:
            move_left();
            break;
        case KEY_PPAGE:
            page_up();
            break;
        case KEY_NPAGE:
            page_down();
            break;
        case KEY_RESIZE:
            resize();
            erase();
            break;
        case K_SAVE:
            prompt_save();
            break;
        case K_QUIT:
            prompt_quit();
            break;
        case K_GO_TO:
            prompt_go_to_byte();
            break;
        case K_SEARCH:
            prompt_search();
            break;
        case K_HEX:
            toggle_hex_mode();
            break;
        case K_ASCII:
            toggle_ascii_mode();
            break;
        case K_SUSP:
            endwin();
            raise(SIGSTOP);
            break;
        default:
            consume_input(c);
            break;
        }
    }
}

void TerminalWindow::draw_line(std::uint32_t line)
{
    const std::uintmax_t line_abs      = m_scroller.first() + line;
    std::uintmax_t       line_byte     = line_abs * BYTES_PER_LINE;
    std::uint32_t        bytes_to_draw = BYTES_PER_LINE;

    if (((m_scroller.total() - 1) == line_abs) && (m_data.size % BYTES_PER_LINE) > 0)
        bytes_to_draw = m_data.size % BYTES_PER_LINE;
    // Skip the first box border line.
    line++;
    // Draw the line byte offset.
    mvprintw(static_cast<int>(line), 1, m_offset_format, line_byte);
    for (std::uint32_t i = 0; i < BYTES_PER_LINE; ++i, ++line_byte)
    {
        const std::uint8_t bt           = m_data[line_byte];
        const bool         is_dirty     = m_data.is_dirty(line_byte);
        char               hexDigits[3] = { 0 };
        std::sprintf(hexDigits, "%02X", bt);
        if (line_byte == m_byte)
        {
            attron(A_REVERSE);
            mvaddch(line, FIRST_ASCII + i, std::isprint(bt) ? bt : '.');
            if (m_mode == Mode::HEX)
            {
                mvaddch(line, FIRST_HEX + i * 3 + m_nibble, hexDigits[m_nibble]);
                attroff(A_REVERSE);
                mvaddch(line, FIRST_HEX + i * 3 + 1 - m_nibble, hexDigits[1 - m_nibble]);
            }
            else
            {
                mvaddstr(line, FIRST_HEX + i * 3, hexDigits);
                attroff(A_REVERSE);
            }
        }
        else if (i < bytes_to_draw)
        {
            if (is_dirty)
                attron(COLOR_PAIR(1) | A_REVERSE);
            mvaddch(line, FIRST_ASCII + i, std::isprint(bt) ? bt : '.');
            mvaddstr(line, FIRST_HEX + i * 3, hexDigits);
            if (is_dirty)
                attroff(COLOR_PAIR(1) | A_REVERSE);
        }
        else
        {
            // padding for the very last line.
            mvaddch(line, FIRST_ASCII + i, ' ');
            mvaddstr(line, FIRST_HEX + i * 3, "  ");
        }
    }
}

bool TerminalWindow::update_screen()
{
    if (m_update)
    {
        box(stdscr, 0, 0);
        for (std::uint32_t line = 0; line < m_scroller.visible(); ++line)
            draw_line(line);

        if (m_data.has_dirty())
            mvprintw(0, (COLS - static_cast<int>(m_name.size())) / 2 - 1, "*%s", m_name.c_str());
        else
            mvaddstr(0, (COLS - static_cast<int>(m_name.size())) / 2 - 1, m_name.c_str());

        const char          mode        = m_mode == Mode::ASCII ? 'A' : 'X';
        const std::uint32_t percentage  = static_cast<std::uint32_t>(static_cast<long double>(m_scroller.last() + 1) / m_scroller.total() * 100);
        const int           info_column = COLS - 8 - static_cast<int>(m_type.size());
        mvprintw(LINES - 1, info_column, "%s/%c/%d%%", m_type.data(), mode, percentage);

        switch (m_prompt.type)
        {
        case Prompt::SAVE:
            mvaddstr(LINES - 1, 1, "Modified buffer, save?(y/n)");
            break;
        case Prompt::QUIT:
            mvaddstr(LINES - 1, 1, "Modified buffer, quit?(y,n)");
            break;
        case Prompt::GO_TO_BYTE:
            mvprintw(LINES - 1, 1, "Goto byte: %s", m_prompt.input.c_str());
            break;
        case Prompt::SEARCH:
            mvprintw(LINES - 1, 1, "Search: %s", m_prompt.input.c_str());
            break;
        default:
            break;
        }
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
    if (m_prompt.type == Prompt::NONE)
        mvprintw(LINES - 1, 1, m_offset_format, m_byte);

    return m_data.is_ok();
}

void TerminalWindow::resize()
{
    // If the number of terminal lines is less than or equal to 2, we cannot display much :(
    if (LINES <= 2)
        return;

    m_scroller.adjust_lines(static_cast<std::uintmax_t>(LINES - 2), m_byte / BYTES_PER_LINE);
    m_update = true;
}

void TerminalWindow::move_up()
{
    if (m_prompt.type != Prompt::NONE)
        return;

    m_update = m_scroller.move_up();
    if (m_byte >= BYTES_PER_LINE)
        m_byte -= BYTES_PER_LINE;
}

void TerminalWindow::page_up()
{
    if (m_prompt.type != Prompt::NONE)
        return;

    if (m_byte >= (BYTES_PER_LINE * m_scroller.visible()))
        m_byte -= BYTES_PER_LINE * m_scroller.visible();
    else
        m_byte = 0;

    resize();
}

void TerminalWindow::move_down()
{
    if (m_prompt.type != Prompt::NONE)
        return;

    const std::uintmax_t distance = m_data.size - m_byte;
    m_update                      = m_scroller.move_down();
    if (distance > BYTES_PER_LINE)
        m_byte += BYTES_PER_LINE;
    else if (distance > (m_data.size % BYTES_PER_LINE))
    {
        m_byte   = m_data.size - 1;
        m_nibble = 0;
    }
}

void TerminalWindow::page_down()
{
    if (m_prompt.type != Prompt::NONE)
        return;

    if ((m_data.size - m_byte) > (BYTES_PER_LINE * m_scroller.visible()))
        m_byte += BYTES_PER_LINE * m_scroller.visible();
    else
        m_byte = m_data.size - 1;

    resize();
}

void TerminalWindow::move_left()
{
    if (m_prompt.type != Prompt::NONE)
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
    if (m_prompt.type != Prompt::NONE)
        return;

    const std::uintmax_t line_abs   = m_byte / BYTES_PER_LINE;
    std::uint32_t        line_bytes = BYTES_PER_LINE;

    if ((line_abs == m_scroller.total() - 1) && (m_data.size % BYTES_PER_LINE) > 0)
        line_bytes = m_data.size % BYTES_PER_LINE;

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

void TerminalWindow::consume_input(int key)
{
    if (m_prompt.type != Prompt::NONE)
        m_prompt.handle_key(key);
    else if (key >= 0 && key <= 0xFF)
        edit_byte(static_cast<std::uint8_t>(key));
}

void TerminalWindow::TerminalWindow::save()
{
    if (!m_data.has_dirty() || m_data.is_read_only())
        return;

    m_data.save();
    m_update = true;
}

void TerminalWindow::prompt_save()
{
    if (!m_data.has_dirty()
        || m_prompt.type != Prompt::NONE
        || m_data.is_read_only())
        return;

    m_prompt.type = Prompt::SAVE;
    m_update      = true;
}

void TerminalWindow::prompt_quit()
{
    if (m_prompt.type != Prompt::NONE)
    {
        // Just remove any active prompt
        m_prompt.type = Prompt::NONE;
        m_update      = true;
        m_prompt.input.clear();
        m_prompt.needle.clear();
    }
    else if (!m_data.has_dirty() || m_data.is_read_only())
        m_quit = true;
    else
    {
        m_prompt.type = Prompt::QUIT;
        m_update      = true;
    }
}

void TerminalWindow::prompt_go_to_byte()
{
    if (m_prompt.type != Prompt::NONE)
        return;

    m_prompt.type = Prompt::GO_TO_BYTE;
    m_prompt.input.clear();
    m_update = true;
}

void TerminalWindow::prompt_search()
{
    if (m_prompt.type == Prompt::SEARCH)
        return;

    m_prompt.type = Prompt::SEARCH;
    m_prompt.input.clear();
    m_prompt.needle.clear();
    m_update = true;
}

void TerminalWindow::toggle_ascii_mode()
{
    if (m_mode == Mode::ASCII || m_prompt.type != Prompt::NONE)
        return;

    m_mode   = Mode::ASCII;
    m_update = true;
    m_nibble = 0;
}

void TerminalWindow::toggle_hex_mode()
{
    if (m_mode == Mode::HEX || m_prompt.type != Prompt::NONE)
        return;

    m_mode   = Mode::HEX;
    m_update = true;
}

void TerminalWindow::edit_byte(std::uint8_t chr)
{
    std::uint8_t new_value;
    if (m_mode == Mode::ASCII && std::isprint(chr))
        new_value = chr;
    else if (m_mode == Mode::HEX && std::isxdigit(chr))
    {
        new_value = update_nibble(m_nibble, chr, m_data[m_byte]);
    }
    else
        return;

    if (!m_data.has_dirty())
        m_update = true;

    m_data.set_byte(m_byte, new_value);
}

void TerminalWindow::handle_prompt(int key)
{
    if (m_prompt.type == Prompt::GO_TO_BYTE || m_prompt.type == Prompt::SEARCH)
    {
        if (key == '\n' && !m_prompt.input.empty())
        {
            if (m_prompt.type == Prompt::GO_TO_BYTE)
            {
                std::uintmax_t go_to_byte = 0;
                if (m_mode == Mode::ASCII)
                    go_to_byte = std::stoull(m_prompt.input, nullptr);
                else if (m_mode == Mode::HEX)
                    go_to_byte = std::stoull(m_prompt.input, nullptr, 16);

                if (go_to_byte < m_data.size)
                    m_byte = go_to_byte;
                else
                    m_byte = m_data.size - 1;
                m_prompt.type = Prompt::NONE;
                m_prompt.input.clear();
            }
            else if (m_prompt.type == Prompt::SEARCH)
            {
                if (m_prompt.needle.empty())
                {
                    m_prompt.needle.reserve(m_prompt.input.size());
                    std::size_t ci = 0;
                    if (m_mode == Mode::HEX)
                    {
                        if (m_prompt.input.size() & 0x1)
                            m_prompt.needle.push_back(hdtoi(static_cast<uint8_t>(m_prompt.input[ci++])));
                        for (; ci < m_prompt.input.size(); ci += 2)
                            m_prompt.needle.push_back(static_cast<uint8_t>((hdtoi(static_cast<uint8_t>(m_prompt.input[ci])) << 4) | hdtoi(static_cast<uint8_t>(m_prompt.input[ci + 1]))));
                    }
                    else
                        std::copy(m_prompt.input.begin(), m_prompt.input.end(), std::back_inserter(m_prompt.needle));
                    m_byte = m_data.find(m_prompt.needle, m_byte).value_or(m_byte);
                }
                else
                    m_byte = m_data.find(m_prompt.needle, m_byte + m_prompt.needle.size()).value_or(m_byte);
            }
            resize();
        }
        else if ((key == KEY_BACKSPACE) && !m_prompt.input.empty())
        {
            m_prompt.input.pop_back();
            m_prompt.needle.clear();
            m_update = true;
        }
        else if ((m_prompt.input.size() < LINE_OFFSET_LEN)
                 && (key >= 0x20 && key <= 0x7F))
        {

            if ((m_mode == Mode::HEX && !std::isxdigit(key))
                || ((m_mode == Mode::ASCII && m_prompt.type == Prompt::GO_TO_BYTE && !std::isdigit(key))))
            {
                return;
            }

            m_prompt.input.push_back(static_cast<char>(key));
            m_prompt.needle.clear();
            m_update = true;
        }
    }
    else
    {
        switch (key)
        {
        case 'y':
        case 'Y':
        {
            if (m_prompt.type == Prompt::SAVE)
            {
                save();
                m_prompt.type = Prompt::NONE;
            }
            else if (m_prompt.type == Prompt::QUIT)
                m_quit = true;
            break;
        }
        case 'n':
        case 'N':
        {
            m_prompt.type = Prompt::NONE;
            m_update      = true;
            break;
        }
        default:
            break;
        }
    }
}
} // namespace Hexit
