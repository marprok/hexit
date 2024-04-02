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
    static constexpr std::uintmax_t capacity = 1024u;

    struct DataChunk
    {
        std::uintmax_t m_id             = { UINTMAX_MAX };
        std::uintmax_t m_count          = { 0 };
        std::uint8_t   m_data[capacity] = { 0 };
    };

    explicit ChunkCache(IOHandler& handler);

    ChunkCache(const ChunkCache&) = delete;

    ChunkCache& operator=(const ChunkCache&) = delete;

    bool load_chunk(std::uintmax_t chunk_id);

    bool save_chunk(const DataChunk& chunk);

    inline std::uintmax_t total_chunks() const { return m_total_chunks; }

    inline DataChunk& recent() { return m_chunks[m_id]; }

    inline DataChunk& fallback() { return m_chunks[1 - m_id]; }

    inline bool is_read_only() const { return m_handler.read_only(); }

private:
    IOHandler&    m_handler;
    std::uintmax_t m_total_chunks;
    DataChunk     m_chunks[2];
    std::uint8_t  m_id; // id of the most recently used chunk.
};
} // namespace Hexit
#endif // CHUNK_CACHE_H
