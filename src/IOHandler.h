#ifndef IO_HANDLER_H
#define IO_HANDLER_H

#include <cstdint>
#include <filesystem>

namespace Hexit
{
namespace fs = std::filesystem;

class IOHandler
{
public:
    virtual ~IOHandler() = default;

    IOHandler() = default;

    IOHandler(const IOHandler&) = delete;

    IOHandler& operator=(const IOHandler&) = delete;

    virtual bool open(const fs::path& path) = 0;

    virtual void close() = 0;

    virtual bool read(std::uint8_t* o_buffer, std::size_t buffer_size) = 0;

    virtual bool write(const std::uint8_t* i_buffer, std::size_t buffer_size) = 0;

    virtual bool seek(std::uint64_t offset) = 0;

    virtual const fs::path& name() const = 0;

    virtual std::uint64_t size() const = 0;
};
} // namespace Hexit
#endif // IO_HANDLER_H
