#ifndef BYTE_BUFFER_H
#define BYTE_BUFFER_H

#include "ChunkCache.h"
#include <map>
#include <unordered_map>
#include <vector>

class ByteBuffer
{
public:
    ByteBuffer(ChunkCache& cache);

    std::uint8_t operator[](std::uint32_t byte_id);

    void set_byte(std::uint32_t byte_id, std::uint8_t byte_value);

    bool is_dirty(std::uint32_t byte_id) const;

    bool has_dirty() const;

    void save();

    inline const fs::path& name() const { return m_cache.name(); }

    inline std::uint32_t size() const { return m_cache.size(); }

    inline bool is_read_only() const { return m_cache.is_read_only(); }

private:
    typedef std::unordered_map<std::uint32_t, std::uint8_t>     DirtyByteMap;
    typedef std::map<std::uint32_t, std::vector<std::uint32_t>> DirtyChunkMap;

    DirtyByteMap  m_dirty_bytes;
    DirtyChunkMap m_dirty_chunks;
    ChunkCache&   m_cache;
};

#endif // BYTE_BUFFER_H
