#ifndef SCROLLER_H
#define SCROLLER_H

#include <cstdint>

class Scroller
{
public:
    Scroller(std::uint32_t total_bytes, std::uint32_t bytes_per_line);

    // No checks are performed by the method, the caller should make sure that the parameters are valid.
    void adjust_lines(std::uint32_t visible_lines, std::uint32_t current_byte);

    bool move_down();

    bool move_up();

    inline std::uint32_t first() const { return m_first_line; }

    inline std::uint32_t last() const { return m_last_line; }

    inline std::uint32_t total() const { return m_total_lines; }

    inline std::uint32_t visible() const { return m_visible_lines; }

    inline std::uint32_t active() const { return m_active_line; }

private:
    std::uint32_t m_first_line;
    std::uint32_t m_last_line;
    std::uint32_t m_total_lines;
    std::uint32_t m_visible_lines;
    std::uint32_t m_active_line;
    std::uint32_t m_bytes_per_line;
};

#endif // SCROLLER_H
