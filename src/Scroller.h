#ifndef SCROLLER_H
#define SCROLLER_H

#include <cstdint>

namespace Hexit
{
class Scroller
{
public:
    Scroller(std::uint64_t total_bytes, std::uint64_t bytes_per_line);

    // No checks are performed by the method, the caller should make sure that the parameters are valid.
    void adjust_lines(std::uint64_t visible_lines, std::uint64_t current_line);

    bool move_down();

    bool move_up();

    inline std::uint64_t first() const { return m_first_line; }

    inline std::uint64_t last() const { return m_last_line; }

    inline std::uint64_t total() const { return m_total_lines; }

    inline std::uint64_t visible() const { return m_visible_lines; }

    inline std::uint64_t active() const { return m_active_line; }

private:
    std::uint64_t m_first_line;
    std::uint64_t m_last_line;
    std::uint64_t m_total_lines;
    std::uint64_t m_visible_lines;
    std::uint64_t m_active_line;
};
} // namespace Hexit
#endif // SCROLLER_H
