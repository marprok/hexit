#ifndef STDIN_HANDLER_H
#define STDIN_HANDLER_H

#include "IOHandler.h"
#include <vector>

namespace Hexit
{
class StdInHandler : public IOHandler
{
public:
    explicit StdInHandler(bool read_only = false)
        : IOHandler(read_only)
        , m_offset(0u)
    {
    }

    ~StdInHandler() = default;

    bool open(const fs::path& path) override;

    void close() override;

    bool read(std::uint8_t* o_buffer, std::uintmax_t buffer_size) override;

    bool write(const std::uint8_t* i_buffer, std::uintmax_t buffer_size) override;

    bool seek(std::uintmax_t offset) override;

private:
    std::vector<std::uint8_t> m_data;
    std::uintmax_t            m_offset;
};
} // namespace Hexit
#endif // STDIN_HANDLER_H
