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
    static constexpr std::uint64_t chunk_count = 255;

    IOHandlerMock(bool read_only = false);

    ~IOHandlerMock() = default;

    bool open(const fs::path& path) override;

    void close() override;

    bool read(std::uint8_t* o_buffer, std::size_t buffer_size) override;

    bool write(const std::uint8_t* i_buffer, std::size_t buffer_size) override;

    bool seek(std::uint64_t offset) override;

    const fs::path& name() const override;

    std::uint64_t size() const override;

    std::uint8_t* data();

    std::uint64_t load_count() const;

    void mock_io_fail(bool should_fail);

private:
    inline void randomize()
    {
        std::srand(std::time(nullptr));
        std::uint8_t* bytes = data();
        for (std::uint64_t i = 0; i < chunk_count * Hexit::ChunkCache::capacity; ++i)
            bytes[i] = rand() % 256;
    }

    fs::path      m_name;
    std::uint8_t  m_data[chunk_count][Hexit::ChunkCache::capacity];
    std::uint64_t m_id;
    std::uint64_t m_load_count;
    bool          m_io_fail;
};
#endif // IOHANDLER_MOCK_H
