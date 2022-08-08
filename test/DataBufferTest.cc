#include "DataBuffer.h"
#include "IOHandler.h"
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <memory>
#include <string>

namespace fs = std::filesystem;

class IOHandlerMock : public IOHandler
{
public:
    IOHandlerMock()
        : m_id(0)
    {
    }

    ~IOHandlerMock() = default;

    bool open(const fs::path& path) override
    {
        m_name = path;
        return true;
    }

    void close() override
    {
    }

    bool read(std::uint8_t* o_buffer, std::size_t buffer_size) override
    {
        if (!o_buffer || buffer_size == 0
            || buffer_size > ChunkCache::capacity)
            return false;

        std::memcpy(o_buffer, m_data[m_id], buffer_size);
        m_id++;
        return true;
    }

    void write(const std::uint8_t* i_buffer, std::size_t buffer_size) override
    {
        if (!i_buffer || buffer_size == 0
            || buffer_size > ChunkCache::capacity)
            return;

        std::memcpy(m_data[m_id], i_buffer, buffer_size);
    }

    void seek(std::uint32_t offset) override
    {
        m_id = offset / ChunkCache::capacity;
    }

    const fs::path& name() const
    {
        return m_name;
    }

    std::uint32_t size() const
    {
        return m_size * ChunkCache::capacity;
    }

    std::uint8_t* Data() { return reinterpret_cast<std::uint8_t*>(m_data); }

private:
    fs::path      m_name;
    std::uint32_t m_size = 255;
    // Let the data uninitialized to simulate "real" data
    std::uint8_t  m_data[255][ChunkCache::capacity];
    std::uint32_t m_id;
};

namespace
{
const std::string file_name("test_path");
}

TEST(DataBufferTest, IOData)
{
    IOHandlerMock handler;
    DataBuffer    buffer(handler);
    EXPECT_TRUE(buffer.open_file(file_name));
    EXPECT_EQ(buffer.name(), handler.name());
    EXPECT_EQ(buffer.size(), handler.size());
    std::uint32_t expected_chunks = handler.size() / DataBuffer::ChunkCache::capacity;
    if (handler.size() % DataBuffer::ChunkCache::capacity)
        expected_chunks++;
    EXPECT_EQ(expected_chunks, buffer.total_chunks());
}

TEST(DataBufferTest, LoadChunk)
{
    IOHandlerMock handler;
    DataBuffer    buffer(handler);
    EXPECT_TRUE(buffer.open_file(file_name));
    EXPECT_TRUE(buffer.total_chunks() > 0);

    EXPECT_TRUE(buffer.load_chunk(0));
    auto previous_chunk_id = buffer.recent_chunk().m_id;
    for (std::uint32_t i = 1; i < buffer.total_chunks(); ++i)
    {
        EXPECT_TRUE(buffer.load_chunk(i));
        EXPECT_TRUE(previous_chunk_id == buffer.fallback_chunk().m_id);
        previous_chunk_id = buffer.recent_chunk().m_id;
    }
}

TEST(DataBufferTest, LoadChunkReverse)
{
    IOHandlerMock handler;
    DataBuffer    buffer(handler);
    EXPECT_TRUE(buffer.open_file(file_name));
    EXPECT_TRUE(buffer.total_chunks() > 2);

    EXPECT_TRUE(buffer.load_chunk(buffer.total_chunks() - 1));
    auto previous_chunk_id = buffer.recent_chunk().m_id;
    for (std::uint32_t i = buffer.total_chunks() - 2; i > 0; --i)
    {
        EXPECT_TRUE(buffer.load_chunk(i));
        EXPECT_TRUE(previous_chunk_id == buffer.fallback_chunk().m_id);
        previous_chunk_id = buffer.recent_chunk().m_id;
    }
}

TEST(DataBufferTest, DataModification)
{
    IOHandlerMock handler;
    auto          expectation = handler.Data();
    DataBuffer    buffer(handler);
    EXPECT_TRUE(buffer.open_file(file_name));
    auto byte_id         = buffer.size() - 1;
    expectation[byte_id] = 0x0F;
    EXPECT_EQ(buffer[byte_id], 0x0F);
    EXPECT_FALSE(buffer.is_dirty(byte_id));
    EXPECT_FALSE(buffer.has_dirty());
    buffer.set_byte(byte_id, 0xFF);
    EXPECT_TRUE(buffer.is_dirty(byte_id));
    EXPECT_TRUE(buffer.has_dirty());
    EXPECT_EQ(buffer[byte_id], 0xFF);
}

TEST(DataBufferTest, DataRead)
{
    IOHandlerMock handler;
    auto          expectation = handler.Data();
    DataBuffer    buffer(handler);
    EXPECT_TRUE(buffer.open_file(file_name));
    // start from the first byte
    EXPECT_TRUE(buffer.load_chunk(0));
    bool match = true;
    for (std::uint32_t i = 0; i < buffer.size() && match; ++i)
        match = buffer[i] == expectation[i];

    EXPECT_TRUE(match);
}

TEST(DataBufferTest, DataReadReverse)
{
    IOHandlerMock handler;
    auto          expectation = handler.Data();
    DataBuffer    buffer(handler);
    EXPECT_TRUE(buffer.open_file(file_name));
    // start from the first byte
    EXPECT_TRUE(buffer.load_chunk(buffer.total_chunks() - 1));
    bool match = true;
    for (std::uint32_t i = buffer.size() - 1; i > 0 && match; --i)
        match = buffer[i] == expectation[i];

    EXPECT_TRUE(match);
}
