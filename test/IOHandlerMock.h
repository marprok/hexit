#ifndef IOHANDLER_MOCK_H
#define IOHANDLER_MOCK_H

#include "ChunkCache.h"
#include "IOHandler.h"
#include <cstdlib>
#include <cstring>

class IOHandlerMock : public IOHandler
{
public:
    static constexpr std::uint32_t chunk_count = 255;

    IOHandlerMock();

    ~IOHandlerMock() = default;

    bool open(const fs::path& path) override;

    void close() override;

    bool read(std::uint8_t* o_buffer, std::size_t buffer_size) override;

    bool write(const std::uint8_t* i_buffer, std::size_t buffer_size) override;

    bool seek(std::uint32_t offset) override;

    const fs::path& name() const override;

    std::uint32_t size() const override;

    std::uint8_t* data();

    std::uint32_t load_count() const;

private:
    inline void randomize()
    {
        std::srand(std::time(nullptr));
        std::uint8_t* bytes = data();
        for (std::size_t i = 0; i < chunk_count * ChunkCache::capacity; ++i)
            bytes[i] = rand() % 256;
    }

    fs::path      m_name;
    std::uint8_t  m_data[chunk_count][ChunkCache::capacity];
    std::uint32_t m_id;
    std::uint32_t m_load_count;
};
#endif // IOHANDLER_MOCK_H