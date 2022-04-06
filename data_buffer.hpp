#ifndef DATA_BUFFER_H
#define DATA_BUFFER_H

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

template <std::uint32_t capacity = 1024>
class DataBuffer
{
private:
    typedef std::unordered_map<std::uint32_t, std::uint8_t>     DirtyByteMap;
    typedef std::map<std::uint32_t, std::vector<std::uint32_t>> DirtyChunkMap;

    struct DataChunk
    {
        std::uint32_t m_id             = { 0 };
        std::uint32_t m_count          = { 0 };
        std::uint8_t  m_data[capacity] = { 0 };
    };

    fs::path      m_name;
    std::fstream  m_stream;
    std::uint32_t m_size;
    std::uint32_t m_total_chunks;
    DataChunk     m_chunks[2];
    std::uint8_t  m_front_id;
    std::uint8_t  m_back_id;
    DirtyByteMap  m_dirty_bytes;
    DirtyChunkMap m_dirty_chunks;

public:
    DataBuffer() : m_size(0),
                   m_total_chunks(0),
                   m_front_id(1),
                   m_back_id(0)
    {
    }

    const fs::path& name() const { return m_name; }
    std::uint32_t   size() const { return m_size; }

    bool open_file(const fs::path& file_name)
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

        // TODO<Marios>: maybe make this public so that we can
        // support the variable begining offset
        load_chunk(0);

        return true;
    }

    // No bounds checking is performed by the operator
    std::uint8_t operator[](std::uint32_t byte_id)
    {
        std::uint32_t chunk_id    = byte_id / capacity;
        std::uint32_t relative_id = byte_id - capacity * chunk_id;

        if (is_dirty(byte_id))
            return m_dirty_bytes[byte_id];

        if (m_chunks[m_front_id].m_id == chunk_id)
            return m_chunks[m_front_id].m_data[relative_id];
        else if (m_chunks[m_back_id].m_id == chunk_id)
            return m_chunks[m_back_id].m_data[relative_id];

        // chunk miss, load from disk...
        load_chunk(chunk_id);

        return m_chunks[m_front_id].m_data[relative_id];
    }

    bool load_chunk(std::uint32_t chunk_id)
    {
        m_stream.seekg(chunk_id * capacity);
        if (!m_stream)
            return false;

        std::uint32_t bytes_to_read = capacity;
        if (chunk_id == m_total_chunks - 1)
            bytes_to_read = m_size % capacity;

        auto& target_cache = m_chunks[m_back_id];
        m_stream.read(reinterpret_cast<char*>(target_cache.m_data), bytes_to_read);

        target_cache.m_id    = chunk_id;
        target_cache.m_count = bytes_to_read;
        std::swap(m_back_id, m_front_id);

        return true;
    }

    void set_byte(std::uint32_t byte_id, std::uint8_t byte_value)
    {
        std::uint32_t chunk_id    = byte_id / capacity;
        std::uint32_t relative_id = byte_id - capacity * chunk_id;
        if (!m_dirty_bytes.contains(byte_id))
        {
            auto& chunk_changes = m_dirty_chunks[chunk_id];
            chunk_changes.emplace_back(relative_id);
        }

        m_dirty_bytes.insert_or_assign(byte_id, byte_value);
    }

    bool is_dirty(std::uint32_t byte_id) const
    {
        return m_dirty_bytes.contains(byte_id);
    }

    bool has_dirty() const
    {
        return m_dirty_bytes.size() != 0;
    }

    void save()
    {
        if (m_dirty_bytes.size() == 0)
            return;

        for (const auto& [chunk_id, changes] : m_dirty_chunks)
        {
            load_chunk(chunk_id);
            DataChunk& chunk = m_chunks[m_front_id];
            for (auto& change : changes)
                chunk.m_data[change] = m_dirty_bytes[chunk_id * capacity + change];

            m_stream.seekg(chunk_id * capacity);
            m_stream.write(reinterpret_cast<char*>(chunk.m_data), chunk.m_count);
        }

        m_dirty_bytes.clear();
        m_dirty_chunks.clear();
    }
};

#endif // DATA_BUFFER_H
