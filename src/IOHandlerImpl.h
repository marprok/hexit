#ifndef IO_HANDLER_IMPL_H
#define IO_HANDLER_IMPL_H

#include "IOHandler.h"
#include <fstream>

class IOHandlerImpl : public IOHandler
{
public:
    IOHandlerImpl() = default;

    ~IOHandlerImpl() = default;

    bool open(const fs::path& path) override;

    void close() override;

    bool read(std::uint8_t* o_buffer, std::size_t buffer_size) override;

    void write(const std::uint8_t* i_buffer, std::size_t buffer_size) override;

    void seek(std::uint32_t offset) override;

    const fs::path& name() const override;

    std::uint32_t size() const override;

private:
    std::fstream  m_stream;
    fs::path      m_name;
    std::uint32_t m_size;
};

#endif
