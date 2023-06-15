#include "ByteBuffer.h"
#include "IOHandler.h"
#include <string>

namespace Hexit
{
ByteBuffer::ByteBuffer(IOHandler& handler)
    : m_cache(handler)
{
}

// No bounds checking is performed by the operator at all.
// is_ok() method should get called to check if an I/O error has occured.
std::uint8_t ByteBuffer::operator[](std::uint64_t byte_id)
{
    std::uint64_t chunk_id    = byte_id / ChunkCache::capacity;
    std::uint64_t relative_id = byte_id - ChunkCache::capacity * chunk_id;

    if (is_dirty(byte_id))
        return m_dirty_bytes[byte_id];

    if (m_cache.recent().m_id == chunk_id)
        return m_cache.recent().m_data[relative_id];
    else if (m_cache.fallback().m_id == chunk_id)
        return m_cache.fallback().m_data[relative_id];

    // chunk miss, load from disk...
    if (!m_cache.load_chunk(chunk_id))
        log_error("Error at ByteBuffer::operator[]: Could not load chunk with id " + std::to_string(chunk_id));

    return m_cache.recent().m_data[relative_id];
}

void ByteBuffer::set_byte(std::uint64_t byte_id, std::uint8_t byte_value)
{
    const std::uint64_t chunk_id    = byte_id / ChunkCache::capacity;
    const std::uint64_t relative_id = byte_id - ChunkCache::capacity * chunk_id;

    if (!m_dirty_bytes.contains(byte_id))
        m_dirty_chunks[chunk_id].emplace_back(relative_id);

    m_dirty_bytes.insert_or_assign(byte_id, byte_value);
}

void ByteBuffer::save()
{
    if (!has_dirty() || m_cache.is_read_only())
        return;

    for (const auto& [chunk_id, changes] : m_dirty_chunks)
    {
        if (!m_cache.load_chunk(chunk_id))
        {
            log_error("Error at ByteBuffer::save(): Could not load chunk with id " + std::to_string(chunk_id));
            break;
        }

        auto& chunk = m_cache.recent();
        for (auto& change : changes)
            chunk.m_data[change] = m_dirty_bytes[chunk_id * ChunkCache::capacity + change];

        if (!m_cache.save_chunk(chunk))
        {
            log_error("Error at ByteBuffer::save(): Could not save chunk with id " + std::to_string(chunk_id));
            break;
        }
    }

    m_dirty_bytes.clear();
    m_dirty_chunks.clear();
}
} // namespace Hexit
