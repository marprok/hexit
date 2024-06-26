#include "IOHandlerMock.h"

IOHandlerMock::IOHandlerMock(bool read_only)
    : IOHandler(read_only)
    , m_id(0u)
    , m_load_count(0u)
    , m_io_fail(false)
{
    m_size = chunk_count * Hexit::ChunkCache::capacity;
    randomize();
}

bool IOHandlerMock::open(const fs::path& path)
{
    m_name = path;
    return !m_io_fail;
}

void IOHandlerMock::close()
{
}

bool IOHandlerMock::read(std::uint8_t* o_buffer, std::uintmax_t buffer_size)
{
    if (!o_buffer
        || buffer_size == 0
        || buffer_size > Hexit::ChunkCache::capacity
        || m_id >= chunk_count)
        return false;

    std::memcpy(o_buffer, m_data[m_id], buffer_size);
    m_load_count++;
    m_id++;
    return !m_io_fail;
}

bool IOHandlerMock::write(const std::uint8_t* i_buffer, std::uintmax_t buffer_size)
{
    if (!i_buffer
        || buffer_size == 0
        || buffer_size > Hexit::ChunkCache::capacity
        || m_id >= chunk_count)
        return false;

    std::memcpy(m_data[m_id], i_buffer, buffer_size);
    return !m_io_fail;
}

bool IOHandlerMock::seek(std::uintmax_t offset)
{
    m_id = offset / Hexit::ChunkCache::capacity;
    return !m_io_fail;
}

std::uint8_t* IOHandlerMock::data() { return reinterpret_cast<std::uint8_t*>(m_data); }

std::uintmax_t IOHandlerMock::load_count() const { return m_load_count; }

void IOHandlerMock::mock_io_fail(bool should_fail)
{
    m_io_fail = should_fail;
}
