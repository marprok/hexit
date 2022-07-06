#ifndef DATA_CACHE_H
#define DATA_CACHE_H

#include <cstdint>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class DataCache
{
public:
    static constexpr std::uint32_t capacity = 1024;

    struct DataChunk
    {
        std::uint32_t m_id             = { 0 };
        std::uint32_t m_count          = { 0 };
        std::uint8_t  m_data[capacity] = { 0 };
    };

    DataCache();

    const fs::path& name() const;

    std::uint32_t size() const;

    std::uint32_t total_chunks() const;

    bool open_file(const fs::path& file_name);

    bool load_chunk(std::uint32_t chunk_id);

    void save_chunk(const DataChunk& chunk);

    DataChunk& recent_chunk();

    DataChunk& fallback_chunk();

private:
    fs::path      m_name;
    std::fstream  m_stream;
    std::uint32_t m_size;
    std::uint32_t m_total_chunks;
    DataChunk     m_chunks[2];
    std::uint8_t  m_recent_id;
    std::uint8_t  m_fallback_id;
};
#endif
