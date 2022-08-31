#include "StdInHandler.h"
#include <cstring>
#include <iostream>
#include <iterator>

bool StdInHandler::open(const fs::path& path)
{
    static_cast<void>(path);
    std::cin.unsetf(std::ios_base::skipws);
    m_data.insert(m_data.begin(),
                  std::istream_iterator<std::uint8_t>(std::cin),
                  std::istream_iterator<std::uint8_t>());

    if (m_data.empty())
        return false;

    m_size   = m_data.size();
    m_offset = 0;
    m_name   = path;
    // reopen the tty device to allow ncurses to read from stdin
    return freopen("/dev/tty", "rw", stdin) != nullptr;
}

void StdInHandler::close()
{
    m_offset = 0;
    m_name   = "";
    m_size   = 0;
    m_data.clear();
}

bool StdInHandler::read(std::uint8_t* o_buffer, std::size_t buffer_size)
{
    if (!o_buffer
        || m_data.empty()
        || (buffer_size + m_offset) > m_size)
        return false;

    if (buffer_size != 0)
        std::memcpy(o_buffer, m_data.data() + m_offset, buffer_size);

    return true;
}

void StdInHandler::write(const std::uint8_t* i_buffer, std::size_t buffer_size)
{
    static_cast<void>(i_buffer);
    static_cast<void>(buffer_size);
}

void StdInHandler::seek(std::uint32_t offset)
{
    if (m_data.empty() || offset >= m_size)
        return;

    m_offset = offset;
}

const fs::path& StdInHandler::name() const
{
    return m_name;
}

std::uint32_t StdInHandler::size() const
{
    return m_size;
}
