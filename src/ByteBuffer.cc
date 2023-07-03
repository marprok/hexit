#include "ByteBuffer.h"
#include "IOHandler.h"
#include <string>

namespace Hexit
{
ByteBuffer::ByteBuffer(IOHandler& handler)
    : size(handler.size())
    , m_cache(handler)
{
}

// No bounds checking is performed by the operator at all.
// is_ok() method should get called to check if an I/O error has occured.
std::uint8_t ByteBuffer::operator[](std::uint64_t byte_id)
{
    std::uint64_t chunk_id    = byte_id / ChunkCache::capacity;
    std::uint64_t relative_id = byte_id - ChunkCache::capacity * chunk_id;
    auto&         recent      = m_cache.recent();
    auto&         fallback    = m_cache.fallback();

    if (recent.m_id == chunk_id)
        return recent.m_data[relative_id];
    else if (fallback.m_id == chunk_id)
        return fallback.m_data[relative_id];

    // chunk miss, load from disk...
    if (!m_cache.load_chunk(chunk_id))
    {
        log_error("Error at ByteBuffer::operator[]: Could not load chunk with id " + std::to_string(chunk_id));
        // Return a garbage value since an error occured.
        return recent.m_data[relative_id];
    }

    auto& new_chunk = m_cache.recent();
    if (m_dirty_chunks.contains(chunk_id))
    {
        for (auto& change : m_dirty_chunks[chunk_id])
            new_chunk.m_data[change] = m_dirty_bytes[chunk_id * ChunkCache::capacity + change];
    }

    return new_chunk.m_data[relative_id];
}

void ByteBuffer::set_byte(std::uint64_t byte_id, std::uint8_t byte_value)
{
    const std::uint64_t chunk_id    = byte_id / ChunkCache::capacity;
    const std::uint64_t relative_id = byte_id - ChunkCache::capacity * chunk_id;
    auto&               recent      = m_cache.recent();
    auto&               fallback    = m_cache.fallback();

    if (recent.m_id == chunk_id)
        recent.m_data[relative_id] = byte_value;
    else
        fallback.m_data[relative_id] = byte_value;

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
        auto& recent   = m_cache.recent();
        auto& fallback = m_cache.fallback();

        if (recent.m_id == chunk_id)
        {
            if (!m_cache.save_chunk(recent))
            {
                log_error("Error at ByteBuffer::save(): Could not save chunk with id " + std::to_string(chunk_id));
                break;
            }
        }
        else if (fallback.m_id == chunk_id)
        {
            if (!m_cache.save_chunk(fallback))
            {
                log_error("Error at ByteBuffer::save(): Could not save chunk with id " + std::to_string(chunk_id));
                break;
            }
        }
        else
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
    }

    m_dirty_bytes.clear();
    m_dirty_chunks.clear();
}
} // namespace Hexit
