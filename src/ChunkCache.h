#ifndef CHUNK_CACHE_H
#define CHUNK_CACHE_H

#include "IOHandler.h"
#include <cstdint>
#include <filesystem>

namespace Hexit
{
namespace fs = std::filesystem;

class ChunkCache
{
public:
    static constexpr std::uint64_t capacity = 1024u;

    struct DataChunk
    {
        std::uint64_t m_id             = { UINT64_MAX };
        std::uint64_t m_count          = { 0 };
        std::uint8_t  m_data[capacity] = { 0 };
    };

    ChunkCache(IOHandler& handler);

    ChunkCache(const ChunkCache&) = delete;

    ChunkCache& operator=(const ChunkCache&) = delete;

    bool load_chunk(std::uint64_t chunk_id);

    bool save_chunk(const DataChunk& chunk);

    inline std::uint64_t total_chunks() const { return m_total_chunks; }

    inline DataChunk& recent() { return *m_recent; }

    inline DataChunk& fallback() { return *m_fallback; }

    inline bool is_read_only() const { return m_handler.read_only(); }

private:
    IOHandler&    m_handler;
    std::uint64_t m_total_chunks;
    DataChunk     m_chunks[2];
    DataChunk*    m_recent;
    DataChunk*    m_fallback;
};
} // namespace Hexit
#endif // CHUNK_CACHE_H
