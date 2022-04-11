#ifndef DATA_BUFFER_H
#define DATA_BUFFER_H

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

class DataBuffer
{
private:
    static constexpr std::uint32_t capacity = 1024;

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
    DataBuffer();

    const fs::path& name() const;

    std::uint32_t size() const;

    bool open_file(const fs::path& file_name);

    std::uint8_t operator[](std::uint32_t byte_id);

    bool load_chunk(std::uint32_t chunk_id);

    void set_byte(std::uint32_t byte_id, std::uint8_t byte_value);

    bool is_dirty(std::uint32_t byte_id) const;

    bool has_dirty() const;

    void save();
};

#endif // DATA_BUFFER_H
