#ifndef BYTE_BUFFER_H
#define BYTE_BUFFER_H

#include "ChunkCache.h"
#include <map>
#include <optional>
#include <unordered_map>
#include <vector>

namespace Hexit
{
class ByteBuffer
{
public:
    explicit ByteBuffer(ChunkCache& cache);

    ByteBuffer(const ByteBuffer&) = delete;

    ByteBuffer& operator=(const ByteBuffer&) = delete;

    std::optional<std::uint8_t> operator[](std::uint64_t byte_id);

    void set_byte(std::uint64_t byte_id, std::uint8_t byte_value);

    bool save();

    inline bool is_dirty(std::uint64_t byte_id) const { return m_dirty_bytes.contains(byte_id); }

    inline bool has_dirty() const { return m_dirty_bytes.size() != 0; }

    inline const fs::path& name() const { return m_cache.name(); }

    inline std::uint64_t size() const { return m_cache.size(); }

    inline bool is_read_only() const { return m_cache.is_read_only(); }

private:
    typedef std::unordered_map<std::uint64_t, std::uint8_t>     DirtyByteMap;
    typedef std::map<std::uint64_t, std::vector<std::uint64_t>> DirtyChunkMap;

    DirtyByteMap  m_dirty_bytes;
    DirtyChunkMap m_dirty_chunks;
    ChunkCache&   m_cache;
};
} // mamespace Hexit
#endif // BYTE_BUFFER_H
