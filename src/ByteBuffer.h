#ifndef BYTE_BUFFER_H
#define BYTE_BUFFER_H

#include "ChunkCache.h"
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace Hexit
{
class ByteBuffer
{
public:
    explicit ByteBuffer(IOHandler& handler);

    ByteBuffer(const ByteBuffer&) = delete;

    ByteBuffer& operator=(const ByteBuffer&) = delete;

    std::uint8_t operator[](std::uintmax_t byte_id);

    void set_byte(std::uintmax_t byte_id, std::uint8_t byte_value);

    void save();

    inline bool is_dirty(std::uintmax_t byte_id) const { return m_dirty_bytes.contains(byte_id); }

    inline bool has_dirty() const { return m_dirty_bytes.size() != 0; }

    inline bool is_read_only() const { return m_cache.is_read_only(); }

    inline bool is_ok() const { return m_error_msg.empty(); };

    const std::string& error_msg() const { return m_error_msg; }

    const std::uintmax_t size;

private:
    inline void log_error(const std::string& err)
    {
        m_error_msg.reserve(err.size());
        m_error_msg = err;
    }

    typedef std::unordered_map<std::uintmax_t, std::uint8_t>      DirtyByteMap;
    typedef std::map<std::uintmax_t, std::vector<std::uintmax_t>> DirtyChunkMap;

    DirtyByteMap  m_dirty_bytes;
    DirtyChunkMap m_dirty_chunks;
    ChunkCache    m_cache;
    std::string   m_error_msg;
};
} // mamespace Hexit
#endif // BYTE_BUFFER_H
