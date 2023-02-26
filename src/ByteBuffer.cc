#include "ByteBuffer.h"
#include "IOHandler.h"
#include <string>

ByteBuffer::ByteBuffer(ChunkCache& cache)
    : m_cache(cache)
{
}

// No bounds checking is performed by the operator
std::uint8_t ByteBuffer::operator[](std::uint32_t byte_id)
{
    std::uint32_t chunk_id    = byte_id / ChunkCache::capacity;
    std::uint32_t relative_id = byte_id - ChunkCache::capacity * chunk_id;

    if (is_dirty(byte_id))
        return m_dirty_bytes[byte_id];

    if (m_cache.recent().m_id == chunk_id)
        return m_cache.recent().m_data[relative_id];
    else if (m_cache.fallback().m_id == chunk_id)
        return m_cache.fallback().m_data[relative_id];

    // chunk miss, load from disk...
    if (!m_cache.load_chunk(chunk_id))
        throw std::runtime_error(std::string("ByteBuffer::operator[] Could not load chunk ")
                                 + std::to_string(chunk_id));

    return m_cache.recent().m_data[relative_id];
}

void ByteBuffer::set_byte(std::uint32_t byte_id, std::uint8_t byte_value)
{
    const std::uint32_t chunk_id    = byte_id / ChunkCache::capacity;
    const std::uint32_t relative_id = byte_id - ChunkCache::capacity * chunk_id;
    if (!m_dirty_bytes.contains(byte_id))
    {
        auto& chunk_changes = m_dirty_chunks[chunk_id];
        chunk_changes.emplace_back(relative_id);
    }

    m_dirty_bytes.insert_or_assign(byte_id, byte_value);
}

bool ByteBuffer::is_dirty(std::uint32_t byte_id) const
{
    return m_dirty_bytes.contains(byte_id);
}

bool ByteBuffer::has_dirty() const
{
    return m_dirty_bytes.size() != 0;
}

void ByteBuffer::save()
{
    if (m_dirty_bytes.size() == 0 || m_cache.is_read_only())
        return;

    for (const auto& [chunk_id, changes] : m_dirty_chunks)
    {
        if (!m_cache.load_chunk(chunk_id))
            throw std::runtime_error(std::string("ByteBuffer::save Could not load chunk ")
                                     + std::to_string(chunk_id));
        auto& chunk = m_cache.recent();
        for (auto& change : changes)
            chunk.m_data[change] = m_dirty_bytes[chunk_id * ChunkCache::capacity + change];

        if (!m_cache.save_chunk(chunk))
            throw std::runtime_error(std::string("ByteBuffer::save Could not save chunk ")
                                     + std::to_string(chunk_id));
    }

    m_dirty_bytes.clear();
    m_dirty_chunks.clear();
}
