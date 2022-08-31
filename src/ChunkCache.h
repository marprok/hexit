#ifndef CHUNK_CACHE_H
#define CHUNK_CACHE_H

#include <cstdint>
#include <filesystem>

namespace fs = std::filesystem;

class IOHandler;

class ChunkCache
{
public:
    static constexpr std::uint32_t capacity = 1024;

    struct DataChunk
    {
        std::uint32_t m_id             = { UINT32_MAX };
        std::uint32_t m_count          = { 0 };
        std::uint8_t  m_data[capacity] = { 0 };
    };

    ChunkCache(IOHandler& handler);

    const fs::path& name() const;

    std::uint32_t size() const;

    std::uint32_t total_chunks() const;

    bool open(const fs::path& name);

    bool load_chunk(std::uint32_t chunk_id);

    void save_chunk(const DataChunk& chunk);

    DataChunk& recent_chunk();

    DataChunk& fallback_chunk();

private:
    IOHandler&    m_handler;
    std::uint32_t m_total_chunks;
    DataChunk     m_chunks[2];
    std::uint8_t  m_recent_id;
    std::uint8_t  m_fallback_id;
};
#endif // CHUNK_CACHE_H
