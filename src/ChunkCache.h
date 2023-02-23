#ifndef CHUNK_CACHE_H
#define CHUNK_CACHE_H

#include "IOHandler.h"
#include <cstdint>
#include <filesystem>

namespace fs = std::filesystem;

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

    bool open(const fs::path& name, bool read_only = false);

    bool load_chunk(std::uint32_t chunk_id);

    bool save_chunk(const DataChunk& chunk);

    inline const fs::path& name() const { return m_handler.name(); }

    inline std::uint32_t size() const { return m_handler.size(); }

    inline std::uint32_t total_chunks() const { return m_total_chunks; }

    inline DataChunk& recent() { return *m_recent; }

    inline DataChunk& fallback() { return *m_fallback; }

    inline bool is_read_only() const { return m_read_only; }

private:
    IOHandler&    m_handler;
    std::uint32_t m_total_chunks;
    DataChunk     m_chunks[2];
    DataChunk*    m_recent;
    DataChunk*    m_fallback;
    bool          m_read_only;
};
#endif // CHUNK_CACHE_H
