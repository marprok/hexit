#include "ChunkCache.h"
#include "IOHandler.h"
#include <stdexcept>

ChunkCache::ChunkCache(IOHandler& handler)
    : m_handler(handler)
    , m_total_chunks(0)
    , m_recent_id(1)
    , m_fallback_id(0)
    , m_immutable(false)
{
}

const fs::path& ChunkCache::name() const { return m_handler.name(); }

std::uint32_t ChunkCache::size() const { return m_handler.size(); }

std::uint32_t ChunkCache::total_chunks() const { return m_total_chunks; }

bool ChunkCache::open(const fs::path& path, bool immutable)
{
    if (!m_handler.open(path))
        return false;

    m_total_chunks = m_handler.size() / capacity;
    if (m_handler.size() % capacity)
        m_total_chunks++;

    m_immutable = immutable;
    return true;
}

bool ChunkCache::load_chunk(std::uint32_t chunk_id)
{
    m_handler.seek(chunk_id * capacity);

    std::uint32_t bytes_to_read = capacity;
    if (chunk_id == (m_total_chunks - 1) && m_handler.size() % capacity)
        bytes_to_read = m_handler.size() % capacity;

    auto& target_cache = m_chunks[m_fallback_id];
    if (!m_handler.read(target_cache.m_data, bytes_to_read))
        return false;

    target_cache.m_id    = chunk_id;
    target_cache.m_count = bytes_to_read;
    std::swap(m_fallback_id, m_recent_id);

    return true;
}

bool ChunkCache::save_chunk(const DataChunk& chunk)
{
    if (m_immutable)
        return false;

    m_handler.seek(chunk.m_id * ChunkCache::capacity);
    return m_handler.write(chunk.m_data, chunk.m_count);
}

ChunkCache::DataChunk& ChunkCache::recent_chunk()
{
    return m_chunks[m_recent_id];
}

ChunkCache::DataChunk& ChunkCache::fallback_chunk()
{
    return m_chunks[m_fallback_id];
}

bool ChunkCache::immutable() const
{
    return m_immutable;
}
