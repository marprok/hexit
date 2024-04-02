#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include "IOHandler.h"
#include <fstream>

namespace Hexit
{
class FileHandler : public IOHandler
{
public:
    FileHandler(bool read_only = false);

    ~FileHandler() = default;

    bool open(const fs::path& path) override;

    void close() override;

    bool read(std::uint8_t* o_buffer, std::uintmax_t buffer_size) override;

    bool write(const std::uint8_t* i_buffer, std::uintmax_t buffer_size) override;

    bool seek(std::uintmax_t offset) override;

private:
    std::fstream m_stream;
};
} // namespace Hexit
#endif
