#ifndef SCROLLER_H
#define SCROLLER_H

#include <cstdint>

namespace Hexit
{
class Scroller
{
public:
    Scroller(std::uintmax_t total_bytes, std::uintmax_t bytes_per_line);

    // No checks are performed by the method, the caller should make sure that the parameters are valid.
    void adjust_lines(std::uintmax_t visible_lines, std::uintmax_t current_line);

    bool move_down();

    bool move_up();

    inline std::uintmax_t first() const { return m_first_line; }

    inline std::uintmax_t last() const { return m_last_line; }

    inline std::uintmax_t total() const { return m_total_lines; }

    inline std::uintmax_t visible() const { return m_visible_lines; }

    inline std::uintmax_t active() const { return m_active_line; }

private:
    std::uintmax_t m_first_line;
    std::uintmax_t m_last_line;
    std::uintmax_t m_total_lines;
    std::uintmax_t m_visible_lines;
    std::uintmax_t m_active_line;
};
} // namespace Hexit
#endif // SCROLLER_H
