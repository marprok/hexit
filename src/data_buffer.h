#ifndef DATA_BUFFER_H
#define DATA_BUFFER_H

#include "data_cache.h"
#include <map>
#include <unordered_map>
#include <vector>

class DataBuffer : public ChunkCache
{
public:
    DataBuffer() = default;

    std::uint8_t operator[](std::uint32_t byte_id);

    void set_byte(std::uint32_t byte_id, std::uint8_t byte_value);

    bool is_dirty(std::uint32_t byte_id) const;

    bool has_dirty() const;

    void save();

private:
    typedef std::unordered_map<std::uint32_t, std::uint8_t>     DirtyByteMap;
    typedef std::map<std::uint32_t, std::vector<std::uint32_t>> DirtyChunkMap;

    DirtyByteMap  m_dirty_bytes;
    DirtyChunkMap m_dirty_chunks;
};

#endif // DATA_BUFFER_H
