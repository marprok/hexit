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
    explicit IOHandler(bool read_only = false)
        : m_read_only(read_only)
        , m_size(0u)
    {
    }

    virtual ~IOHandler() = default;

    IOHandler(const IOHandler&) = delete;

    IOHandler& operator=(const IOHandler&) = delete;

    virtual bool open(const fs::path& path) = 0;

    virtual void close() = 0;

    virtual bool read(std::uint8_t* o_buffer, std::uintmax_t buffer_size) = 0;

    virtual bool write(const std::uint8_t* i_buffer, std::uintmax_t buffer_size) = 0;

    virtual bool seek(std::uintmax_t offset) = 0;

    inline const fs::path& name() const { return m_name; };

    inline std::uintmax_t size() const { return m_size; };

    inline bool read_only() const { return m_read_only; }

protected:
    bool          m_read_only;
    fs::path      m_name;
    std::uintmax_t m_size;
};
} // namespace Hexit
#endif // IO_HANDLER_H
