#include "FileHandler.h"

namespace Hexit
{
bool FileHandler::open(const fs::path& path)
{
    if (!fs::exists(path))
        return false;

    m_stream.open(path.string(), std::ios::in | std::ios::out | std::ios::binary);
    if (!m_stream)
        return false;

    m_name = fs::canonical(path);
    m_size = fs::file_size(m_name);

    return true;
}

void FileHandler::close()

{
    if (m_stream.is_open())
    {
        m_name = "";
        m_size = 0;
        m_stream.close();
    }
}

bool FileHandler::read(std::uint8_t* o_buffer, std::size_t buffer_size)
{
    if (!o_buffer || !m_stream.is_open())
        return false;

    if (buffer_size == 0)
        return true;

    m_stream.read(reinterpret_cast<char*>(o_buffer), buffer_size);

    return static_cast<std::size_t>(m_stream.gcount()) == buffer_size;
}

bool FileHandler::write(const std::uint8_t* i_buffer, std::size_t buffer_size)
{
    if (!m_stream.is_open())
        return false;

    // operator bool is explicit...
    return static_cast<bool>(
        m_stream.write(reinterpret_cast<const char*>(i_buffer), buffer_size));
}

bool FileHandler::seek(std::uint32_t offset)
{
    if (!m_stream.is_open())
        return false;

    return static_cast<bool>(m_stream.seekg(offset));
}

const fs::path& FileHandler::name() const
{
    return m_name;
}

std::uint32_t FileHandler::size() const
{
    return m_size;
}
} // namepsace Hexit
