#include "data_cache.h"

ChunkCache::ChunkCache()
    : m_size(0)
    , m_total_chunks(0)
    , m_recent_id(1)
    , m_fallback_id(0)
{
}

const fs::path& ChunkCache::name() const { return m_name; }

std::uint32_t ChunkCache::size() const { return m_size; }

std::uint32_t ChunkCache::total_chunks() const { return m_total_chunks; }

bool ChunkCache::open_file(const fs::path& file_name)
{
    if (!fs::exists(file_name))
        return false;

    m_name = fs::canonical(file_name);
    m_stream.open(m_name.string(), std::ios::in | std::ios::out | std::ios::binary);
    if (!m_stream)
        return false;

    m_size         = fs::file_size(m_name);
    m_total_chunks = m_size / capacity;
    if (m_size % capacity)
        m_total_chunks++;

    return true;
}

bool ChunkCache::load_chunk(std::uint32_t chunk_id)
{
    m_stream.seekg(chunk_id * capacity);
    if (!m_stream)
        return false;

    std::uint32_t bytes_to_read = capacity;
    if (chunk_id == m_total_chunks - 1)
        bytes_to_read = m_size % capacity;

    auto& target_cache = m_chunks[m_fallback_id];
    m_stream.read(reinterpret_cast<char*>(target_cache.m_data), bytes_to_read);

    target_cache.m_id    = chunk_id;
    target_cache.m_count = bytes_to_read;
    std::swap(m_fallback_id, m_recent_id);

    return true;
}

void ChunkCache::save_chunk(const DataChunk& chunk)
{
    m_stream.seekg(chunk.m_id * ChunkCache::capacity);
    m_stream.write(reinterpret_cast<const char*>(chunk.m_data), chunk.m_count);
}

ChunkCache::DataChunk& ChunkCache::recent_chunk()
{
    return m_chunks[m_recent_id];
}

ChunkCache::DataChunk& ChunkCache::fallback_chunk()
{
    return m_chunks[m_fallback_id];
}
