#include "DataBuffer.h"
#include "IOHandler.h"
#include <string>

DataBuffer::DataBuffer(IOHandler& handler)
    : ChunkCache(handler)
{
}

// No bounds checking is performed by the operator
std::uint8_t DataBuffer::operator[](std::uint32_t byte_id)
{
    std::uint32_t chunk_id    = byte_id / ChunkCache::capacity;
    std::uint32_t relative_id = byte_id - ChunkCache::capacity * chunk_id;

    if (is_dirty(byte_id))
        return m_dirty_bytes[byte_id];

    if (recent().m_id == chunk_id)
        return recent().m_data[relative_id];
    else if (fallback().m_id == chunk_id)
        return fallback().m_data[relative_id];

    // chunk miss, load from disk...
    if (!load_chunk(chunk_id))
        throw std::runtime_error(std::string("DataBuffer::operator[] Could not load chunk ")
                                 + std::to_string(chunk_id));

    return recent().m_data[relative_id];
}

void DataBuffer::set_byte(std::uint32_t byte_id, std::uint8_t byte_value)
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

bool DataBuffer::is_dirty(std::uint32_t byte_id) const
{
    return m_dirty_bytes.contains(byte_id);
}

bool DataBuffer::has_dirty() const
{
    return m_dirty_bytes.size() != 0;
}

void DataBuffer::save()
{
    if (m_dirty_bytes.size() == 0 || is_read_only())
        return;

    for (const auto& [chunk_id, changes] : m_dirty_chunks)
    {
        if (!load_chunk(chunk_id))
            throw std::runtime_error(std::string("DataBuffer::save Could not load chunk ")
                                     + std::to_string(chunk_id));
        DataChunk& chunk = recent();
        for (auto& change : changes)
            chunk.m_data[change] = m_dirty_bytes[chunk_id * ChunkCache::capacity + change];

        if (!save_chunk(chunk))
            throw std::runtime_error(std::string("DataBuffer::save Could not save chunk ")
                                     + std::to_string(chunk_id));
    }

    m_dirty_bytes.clear();
    m_dirty_chunks.clear();
}
