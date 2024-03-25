#include "Scroller.h"
#include <algorithm>

namespace Hexit
{
Scroller::Scroller(std::uintmax_t total_bytes, std::uintmax_t bytes_per_line)
    : m_first_line(0u)
    , m_last_line(0u)
    , m_total_lines(0u)
    , m_visible_lines(0u)
    , m_active_line(0u)
{
    if (total_bytes > 0 && bytes_per_line > 0)
    {
        m_total_lines = total_bytes / bytes_per_line;
        if ((total_bytes % bytes_per_line) > 0)
            m_total_lines++;
    }
}

void Scroller::adjust_lines(std::uintmax_t visible_lines, std::uintmax_t current_line)
{
    m_visible_lines = std::min(visible_lines, m_total_lines);

    if ((m_total_lines - current_line) < m_visible_lines)
    {
        m_last_line  = m_total_lines - 1;
        m_first_line = m_last_line - m_visible_lines + 1;
    }
    else
    {
        m_first_line = current_line / m_visible_lines * m_visible_lines;
        m_last_line  = m_first_line + m_visible_lines - 1;
    }
    m_active_line = current_line - m_first_line;
}

bool Scroller::move_down()
{
    bool scrolled = false;

    if ((m_active_line + 1) < m_visible_lines)
        m_active_line++;
    else if ((m_last_line + 1) < m_total_lines)
    {
        m_last_line++;
        m_first_line++;
        scrolled = true;
    }
    return scrolled;
}

bool Scroller::move_up()
{
    bool scrolled = false;

    if (m_active_line > 0)
        m_active_line--;
    else if (m_first_line > 0)
    {
        m_last_line--;
        m_first_line--;
        scrolled = true;
    }
    return scrolled;
}
} // namespace Hexit
