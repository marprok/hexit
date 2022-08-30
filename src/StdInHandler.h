#ifndef STDIN_HANDLER_H
#define STDIN_HANDLER_H

#include "IOHandler.h"
#include <vector>

class StdInHandler : public IOHandler
{
public:
    StdInHandler() = default;

    ~StdInHandler() = default;

    bool open(const fs::path& path) override;

    void close() override;

    bool read(std::uint8_t* o_buffer, std::size_t buffer_size) override;

    void write(const std::uint8_t* i_buffer, std::size_t buffer_size) override;

    void seek(std::uint32_t offset) override;

    const fs::path& name() const override;

    std::uint32_t size() const override;

private:
    std::vector<std::uint8_t> m_data;
    fs::path                  m_name;
    std::uint32_t             m_size;
    std::uint32_t             m_offset;
};

#endif
