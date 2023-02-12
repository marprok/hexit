#include "IOHandlerMock.h"

IOHandlerMock::IOHandlerMock()
    : m_id(0u)
    , m_load_count(0u)
{
    randomize();
}

bool IOHandlerMock::open(const fs::path& path)
{
    m_name = path;
    return true;
}

void IOHandlerMock::close()
{
}

bool IOHandlerMock::read(std::uint8_t* o_buffer, std::size_t buffer_size)
{
    if (!o_buffer
        || buffer_size == 0
        || buffer_size > ChunkCache::capacity
        || m_id >= chunk_count)
        return false;

    std::memcpy(o_buffer, m_data[m_id], buffer_size);
    m_load_count++;
    m_id++;
    return true;
}

bool IOHandlerMock::write(const std::uint8_t* i_buffer, std::size_t buffer_size)
{
    if (!i_buffer
        || buffer_size == 0
        || buffer_size > ChunkCache::capacity
        || m_id >= chunk_count)
        return false;

    std::memcpy(m_data[m_id], i_buffer, buffer_size);
    return true;
}

bool IOHandlerMock::seek(std::uint32_t offset)
{
    m_id = offset / ChunkCache::capacity;
    return true;
}

const fs::path& IOHandlerMock::name() const
{
    return m_name;
}

std::uint32_t IOHandlerMock::size() const
{
    return chunk_count * ChunkCache::capacity;
}

std::uint8_t* IOHandlerMock::data() { return reinterpret_cast<std::uint8_t*>(m_data); }

std::uint32_t IOHandlerMock::load_count() const { return m_load_count; }
