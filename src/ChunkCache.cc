#include "ChunkCache.h"
#include "IOHandler.h"

namespace Hexit
{
ChunkCache::ChunkCache(IOHandler& handler)
    : m_handler(handler)
    , m_total_chunks(0u)
    , m_id(0u)
{
    m_total_chunks = m_handler.size() / capacity;
    if (m_handler.size() % capacity)
        m_total_chunks++;
}

bool ChunkCache::load_chunk(std::uint64_t chunk_id)
{
    if (!m_handler.seek(chunk_id * capacity))
        return false;

    std::uint64_t bytes_to_read = capacity;
    if (chunk_id == (m_total_chunks - 1) && m_handler.size() % capacity)
        bytes_to_read = m_handler.size() % capacity;

    auto& target_cache = m_chunks[1 - m_id];
    if (!m_handler.read(target_cache.m_data, bytes_to_read))
        return false;

    target_cache.m_id    = chunk_id;
    target_cache.m_count = bytes_to_read;
    m_id                 = 1 - m_id;

    return true;
}

bool ChunkCache::save_chunk(const DataChunk& chunk)
{
    if (m_handler.read_only())
        return false;

    if (!m_handler.seek(chunk.m_id * ChunkCache::capacity))
        return false;

    return m_handler.write(chunk.m_data, chunk.m_count);
}
} // namespace Hexit
