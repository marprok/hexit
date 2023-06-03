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

    bool open(const fs::path& name, bool read_only = false);

    bool load_chunk(std::uint64_t chunk_id);

    bool save_chunk(const DataChunk& chunk);

    inline const fs::path& name() const { return m_handler.name(); }

    inline std::uint64_t size() const { return m_handler.size(); }

    inline std::uint64_t total_chunks() const { return m_total_chunks; }

    inline DataChunk& recent() { return *m_recent; }

    inline DataChunk& fallback() { return *m_fallback; }

    inline bool is_read_only() const { return m_read_only; }

private:
    IOHandler&    m_handler;
    std::uint64_t m_total_chunks;
    DataChunk     m_chunks[2];
    DataChunk*    m_recent;
    DataChunk*    m_fallback;
    bool          m_read_only;
};
} // namespace Hexit
#endif // CHUNK_CACHE_H
