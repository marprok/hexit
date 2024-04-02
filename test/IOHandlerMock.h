#ifndef IOHANDLER_MOCK_H
#define IOHANDLER_MOCK_H

#include "ChunkCache.h"
#include "IOHandler.h"
#include <cstdlib>
#include <cstring>

namespace fs = std::filesystem;

class IOHandlerMock : public Hexit::IOHandler
{
public:
    static constexpr std::uintmax_t chunk_count = 255;

    IOHandlerMock(bool read_only = false);

    ~IOHandlerMock() = default;

    bool open(const fs::path& path) override;

    void close() override;

    bool read(std::uint8_t* o_buffer, std::uintmax_t buffer_size) override;

    bool write(const std::uint8_t* i_buffer, std::uintmax_t buffer_size) override;

    bool seek(std::uintmax_t offset) override;

    std::uint8_t* data();

    std::uintmax_t load_count() const;

    void mock_io_fail(bool should_fail);

private:
    inline void randomize()
    {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        std::uint8_t* bytes = data();
        for (std::uintmax_t i = 0; i < m_size; ++i)
            bytes[i] = static_cast<std::uint8_t>(rand() % 256);
    }

    std::uint8_t  m_data[chunk_count][Hexit::ChunkCache::capacity];
    std::uintmax_t m_id;
    std::uintmax_t m_load_count;
    bool          m_io_fail;
};
#endif // IOHANDLER_MOCK_H
